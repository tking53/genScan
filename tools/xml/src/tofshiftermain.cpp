#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <spdlog/fmt/fmt.h>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <pugixml.hpp>

struct pidprocessor_predicate {
	bool operator()(pugi::xml_attribute attr) const {
		return strcmp(attr.name(), "name") == 0;
	}

	bool operator()(pugi::xml_node node) const {
		return strcmp(node.attribute("name").as_string(""), "PidProcessor") == 0;
	}
};

struct histogram_predicate {
	std::string id;

	histogram_predicate(const std::string& val)
		: id(val) {
	}

	bool operator()(pugi::xml_attribute attr) const {
		return strcmp(attr.name(), "id") == 0;
	}

	bool operator()(pugi::xml_node node) const {
		return strcmp(node.attribute("id").as_string(""), id.c_str()) == 0;
	}
};

struct ShiftData {
	// Shifts[fp1] = {{id_1,delta_1},...};
	// Shifts[fp2] = {{id_101,delta_101},...};
	std::map<std::string, std::map<int, double>> Shifts;
	std::map<int, std::vector<std::string>> His;

	// Config Copy
	pugi::xml_document Config;

	void InsertTofShift(const std::string& fp, const int& id, const double& delta, const std::vector<std::string>& h) {
		this->Shifts[fp][id] = -1.0 * delta;
		this->His[id] = h;
	}

	void LoadConfig(const std::string& f) {
		auto loadres = this->Config.load_file(f.c_str());
		if (not loadres) {
			throw std::runtime_error(loadres.description());
		}
	}

	void ModifyConfig() {
		pugi::xml_node Configuration = this->Config.child("Configuration");
		auto proc = Configuration.find_node(pidprocessor_predicate());
		if (!proc) {
			throw std::runtime_error("Missing PidProcessor node");
		} else {
			// need to determine do we have any TofShift already present as child nodes
			if (proc.child("TofShift")) {
				for (pugi::xml_node ts = proc.child("TofShift"); ts; ts = ts.next_sibling("TofShift")) {
					std::string label = ts.attribute("label").as_string();
					for (pugi::xml_node s = ts.child("Shift"); s; s = s.next_sibling("Shift")) {
						auto id = s.attribute("id").as_int();
						auto delta = s.attribute("delta");
						delta.set_value(this->Shifts[label][id]);
						auto h = this->His[id];
						for (const auto& name : h) {
							auto his = proc.find_node(histogram_predicate(name));
							if (his) {
								// we found one we need to shift
								auto xlow = his.attribute("xlow").as_double();
								auto xhigh = his.attribute("xhigh").as_double();
								auto dist = xhigh - xlow;
								auto xlowp = his.attribute("xlow");
								auto xhighp = his.attribute("xhigh");
								xlowp.set_value(-dist / 2.0);
								xhighp.set_value(dist / 2.0);
							}
						}
					}
				}
			} else {
				for (const auto& kv : this->Shifts) {
					auto ts = proc.append_child("TofShift");
					ts.append_attribute("label") = kv.first.c_str();
					for (const auto& kv2 : kv.second) {
						auto s = ts.append_child("Shift");
						s.append_attribute("id") = kv2.first;
						s.append_attribute("delta") = kv2.second;
						auto h = this->His[kv2.first];
						for (const auto& name : h) {
							auto his = proc.find_node(histogram_predicate(name));
							if (his) {
								// we found one we need to shift
								auto xlow = his.attribute("xlow").as_double();
								auto xhigh = his.attribute("xhigh").as_double();
								auto dist = xhigh - xlow;
								auto xlowp = his.attribute("xlow");
								auto xhighp = his.attribute("xhigh");
								xlowp.set_value(-dist / 2.0);
								xhighp.set_value(dist / 2.0);
							}
						}
					}
				}
			}
		}
	}

	void WriteConfig(const std::string& f) {
		this->Config.save_file(f.c_str());
	}
};

int main(int argc, char* argv[]) {
	std::string configfile;
	std::string outputdir;
	std::vector<std::string> yamlfiles;
	std::vector<std::string> fitfiles;

	// clang-format off
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("file,f", boost::program_options::value<std::vector<std::string>>(&yamlfiles), "[file1 file2 file3 ...] list of files used for input")
		("pidmap,p", boost::program_options::value<std::vector<std::string>>(&fitfiles)->multitoken(), "Add PID_X:id:focal_plane tuple (e.g. PID_7:6:fp1)")
		("configfile,c", boost::program_options::value<std::string>(&configfile), "configfile to read in and regenerate new configs from")
		("outputdir,o", boost::program_options::value<std::string>(&outputdir), "directory to output to, name will be based on the parse rootfile name in the input yaml");

	boost::program_options::positional_options_description p;
	// clang-format on

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		std::map<std::string, std::pair<int, std::string>> forwardmap;
		std::map<int, std::vector<std::string>> extrahis;
		for (const auto& kv : fitfiles) {
			std::vector<std::string> strs;
			boost::split(strs, kv, boost::is_any_of(":"));
			if (strs.size() < 3) {
				throw std::runtime_error("pidmap expects three or more values");
			} else {
				int val = stoi(strs[1]);
				forwardmap[strs[0]] = {val, strs[2]};
				for (size_t ii = 3; ii < strs.size(); ++ii) {
					extrahis[val].push_back(strs[ii]);
				}
			}
		}

		std::map<std::string, ShiftData> shiftvalues;
		// open a yaml file
		for (const auto& f : yamlfiles) {
			YAML::Node doc = YAML::LoadFile(f);
			std::string hisname = doc["HisName"].as<std::string>();
			auto results = doc["Results"];
			for (size_t ii = 0; ii < results.size(); ++ii) {
				auto filename = results[ii]["FileName"].as<std::string>();
				auto mean = results[ii]["Mean"].as<double>();
				if (shiftvalues.find(filename) == shiftvalues.end()) {
					shiftvalues[filename] = ShiftData();
				}
				shiftvalues[filename].InsertTofShift(forwardmap[hisname].second, forwardmap[hisname].first, mean, extrahis[forwardmap[hisname].first]);
			}
		}
		auto gennewname = [](const std::string& c, const std::string& f) {
			// new filename generation
			std::filesystem::path o(f.c_str());
			std::string ext("_" + o.stem().string() + ".xml");
			std::string i = c.substr(0, c.size() - 4);
			return i + ext;
		};

		for (auto& kv : shiftvalues) {
			kv.second.LoadConfig(configfile);
			kv.second.ModifyConfig();
			auto oname = gennewname(configfile, kv.first);
			kv.second.WriteConfig(oname);
		}

	} catch (std::exception& e) {
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}
}
