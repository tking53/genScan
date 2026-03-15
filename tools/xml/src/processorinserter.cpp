#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <pugixml.hpp>

typedef std::tuple<std::string, std::string> isotopetag;
typedef std::tuple<std::string> cuttag;
typedef std::tuple<double, double> gatetag;
typedef std::tuple<double, double, double, double> boxtag;
typedef std::tuple<std::optional<int>, std::optional<double>, std::optional<double>> h1dtag;
typedef std::tuple<std::optional<int>, std::optional<double>, std::optional<double>, std::optional<int>, std::optional<double>, std::optional<double>> h2dtag;

boost::regex uinput("^(?:Y(?:ES)?|N(?:O)?|C(?:ONTINUE)?)$", boost::regex::icase);
boost::regex okinput("^(?:Y(?:ES)?)$", boost::regex::icase);
boost::regex skipinput("^(?:C(?:ONTINUE)?)$", boost::regex::icase);
boost::regex badinput("^(?:N(?:O)?)?$", boost::regex::icase);
boost::regex number("^(?!-0(\\.0+)?(e|$))-?(0|[1-9]\\d*)(\\.\\d+)?(e-?(0|[1-9]\\d*))?");
boost::regex integer("^(0|[1-9]\\d*)$");

struct processor_predicate {
	std::string id;

	processor_predicate(const std::string& val)
		: id(val) {
	}

	bool operator()(pugi::xml_attribute attr) const {
		return strcmp(attr.name(), "name") == 0;
	}

	bool operator()(pugi::xml_node node) const {
		return strcmp(node.attribute("name").as_string(""), id.c_str()) == 0;
	}
};

void AskUser(bool& InputOk, bool& IsValid, bool& Continue) {
	std::string input;
	std::cin >> input;
	if (boost::regex_match(input, uinput)) {
		InputOk = true;
		if (boost::regex_match(input, skipinput)) {
			Continue = true;
		} else {
			IsValid &= boost::regex_match(input, okinput);
		}
	} else {
		InputOk = false;
	}
}

void ParseIsotopes(const std::vector<std::string>& args, std::map<std::string, isotopetag>& itags) {
	for (const auto& s : args) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		if (strs.size() == 3) {
			itags[strs[0]] = {strs[1], strs[2]};
		} else {
			throw std::runtime_error("Invalid number of args to isotopes flag");
		}
	}
}

bool ValidateIsotopeTags(const std::map<std::string, isotopetag>& itags) {
	bool IsValid = true;
	bool InputOk = false;
	bool Continue = false;

	for (const auto& i : itags) {
		do {
			// spdlog::info("Is this tag valid '<Isotope name=\"{}\" cutid=\"{}\" filename=\"{}\" />' ? (y/Y,n/N,c/C)",i.first,std::get<0>(i.second),std::get<1>(i.second));
			AskUser(InputOk, IsValid, Continue);
		} while (not InputOk);

		if (Continue) {
			break;
		}
	}
	return IsValid;
}

void ParseCuts(const std::vector<std::string>& args, std::map<std::string, cuttag>& itags) {
	for (const auto& s : args) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		if (strs.size() == 2) {
			itags[strs[0]] = {strs[1]};
		} else {
			throw std::runtime_error("Invalid number of args to cuts flag");
		}
	}
}

bool ValidateCutTags(const std::map<std::string, cuttag>& ctags) {
	bool IsValid = true;
	bool InputOk = false;
	bool Continue = false;

	for (const auto& i : ctags) {
		do {
			// spdlog::info("Is this tag valid '<Cut name=\"{}\" filename=\"{}\" />' ? (y/Y,n/N,c/C)",i.first,std::get<0>(i.second));
			AskUser(InputOk, IsValid, Continue);
		} while (not InputOk);

		if (Continue) {
			break;
		}
	}
	return IsValid;
}

void ParseGates(const std::vector<std::string>& args, std::map<std::string, gatetag>& gtags) {
	for (const auto& s : args) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		// spdlog::info("{}",strs.size());
		if (strs.size() == 3) {
			double lb = 0.0;
			double ub = 0.0;
			if (boost::regex_match(strs[1], number)) {
				lb = std::stod(strs[1]);
			} else {
				throw std::runtime_error("Missing lowerbound argument for gate");
			}
			if (boost::regex_match(strs[2], number)) {
				ub = std::stod(strs[2]);
			} else {
				throw std::runtime_error("Missing upperbound argument for gate");
			}
			gtags[strs[0]] = {lb, ub};
		} else {
			throw std::runtime_error("Invalid number of args to gates flag");
		}
	}
}

bool ValidateGateTags(const std::map<std::string, gatetag>& gtags) {
	bool IsValid = true;
	bool InputOk = false;
	bool Continue = false;

	for (const auto& i : gtags) {
		do {
			// spdlog::info("Is this tag valid '<Gate name=\"{}\" lowerbound=\"{}\" upperbound=\"{}\" />' ? (y/Y,n/N,c/C)",i.first,std::get<0>(i.second),std::get<1>(i.second));
			AskUser(InputOk, IsValid, Continue);
		} while (not InputOk);

		if (Continue) {
			break;
		}
	}
	return IsValid;
}

void ParseBoxes(const std::vector<std::string>& args, std::map<std::string, boxtag>& btags) {
	for (const auto& s : args) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		// spdlog::info("{}",strs.size());
		if (strs.size() == 5) {
			double xlb = 0.0;
			double xub = 0.0;
			double ylb = 0.0;
			double yub = 0.0;
			if (boost::regex_match(strs[1], number)) {
				xlb = std::stod(strs[1]);
			} else {
				throw std::runtime_error("Missing xlowerbound argument for box gate");
			}
			if (boost::regex_match(strs[2], number)) {
				xub = std::stod(strs[2]);
			} else {
				throw std::runtime_error("Missing xupperbound argument for box gate");
			}
			if (boost::regex_match(strs[3], number)) {
				ylb = std::stod(strs[3]);
			} else {
				throw std::runtime_error("Missing ylowerbound argument for box gate");
			}
			if (boost::regex_match(strs[4], number)) {
				yub = std::stod(strs[4]);
			} else {
				throw std::runtime_error("Missing yupperbound argument for box gate");
			}
			btags[strs[0]] = {xlb, xub, ylb, yub};
		} else {
			throw std::runtime_error("Invalid number of args to boxes flag");
		}
	}
}

bool ValidateBoxTags(const std::map<std::string, boxtag>& btags) {
	bool IsValid = true;
	bool InputOk = false;
	bool Continue = false;

	for (const auto& i : btags) {
		do {
			// spdlog::info("Is this tag valid '<BoxGate name=\"{}\" xlowerbound=\"{}\" xupperbound=\"{}\" ylowerbound=\"{}\" yupperbound=\"{}\" />' ? (y/Y,n/N,c/C)",i.first,std::get<0>(i.second),std::get<1>(i.second),std::get<2>(i.second),std::get<3>(i.second));
			AskUser(InputOk, IsValid, Continue);
		} while (not InputOk);

		if (Continue) {
			break;
		}
	}
	return IsValid;
}

void ParseH1D(const std::vector<std::string>& args, std::map<int, h1dtag>& htags) {
	for (const auto& s : args) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		if (strs.size() == 4) {
			int id = -1;
			std::optional<int> nbinsx;
			std::optional<double> xlow;
			std::optional<double> xhigh;
			if (boost::regex_match(strs[0], integer)) {
				id = std::stoi(strs[0]);
			} else {
				throw std::runtime_error("Invalid ID for h1d tag, need positive integer");
			}
			if (boost::regex_match(strs[1], integer)) {
				nbinsx = std::stoi(strs[1]);
			} else {
				nbinsx = std::nullopt;
			}
			if (boost::regex_match(strs[2], number)) {
				xlow = std::stod(strs[2]);
			} else {
				xlow = std::nullopt;
			}
			if (boost::regex_match(strs[3], number)) {
				xhigh = std::stod(strs[3]);
			} else {
				xhigh = std::nullopt;
			}
			htags[id] = {nbinsx, xlow, xhigh};
		} else {
			throw std::runtime_error("Invalid number of args to h1d flag");
		}
	}
}

bool ValidateH1DTags(const std::map<int, h1dtag>& htags) {
	bool IsValid = true;
	bool InputOk = false;
	bool Continue = false;

	for (const auto& i : htags) {
		auto id = i.first;
		auto nbinsx = std::get<0>(i.second).value_or(-1);
		auto xlow = std::get<1>(i.second).value_or(-1.0);
		auto xhigh = std::get<2>(i.second).value_or(-1.0);
		do {
			int j = 0;
			j += ((std::get<0>(i.second).has_value()) ? 2 : 0);
			j += ((std::get<1>(i.second).has_value()) ? 4 : 0);
			j += ((std::get<2>(i.second).has_value()) ? 8 : 0);
			switch (j) {
			case 2:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" nbinsx=\"{}\" />' ? (y/Y,n/N,c/C)",id,nbinsx);
				break;
			case 4:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" xlow=\"{}\" />' ? (y/Y,n/N,c/C)",id,xlow);
				break;
			case 6:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" nbinsx=\"{}\" xlow=\"{}\" />' ? (y/Y,n/N,c/C)",id,nbinsx,xlow);
				break;
			case 8:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" xhigh=\"{}\" />' ? (y/Y,n/N,c/C)",id,xhigh);
				break;
			case 10:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" nbinsx=\"{}\" xhigh=\"{}\" />' ? (y/Y,n/N,c/C)",id,nbinsx,xhigh);
				break;
			case 12:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" xlow=\"{}\" xhigh=\"{}\" />' ? (y/Y,n/N,c/C)",id,xlow,xhigh);
				break;
			case 14:
				// spdlog::info("Is this tag valid '<Histogram id=\"{}\" nbinsx=\"{}\" xlow=\"{}\" xhigh=\"{}\" />' ? (y/Y,n/N,c/C)",id,nbinsx,xlow,xhigh);
				break;
			default:
				throw std::runtime_error("Invalid Histogram tag generation, only ID provided");
				break;
			}
			AskUser(InputOk, IsValid, Continue);
		} while (not InputOk);

		if (Continue) {
			break;
		}
	}
	return IsValid;
}

void ParseH2D(const std::vector<std::string>& args, std::map<int, h2dtag>& htags) {
	for (const auto& s : args) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		if (strs.size() == 7) {
			int id = -1;
			std::optional<int> nbinsx;
			std::optional<double> xlow;
			std::optional<double> xhigh;
			std::optional<int> nbinsy;
			std::optional<double> ylow;
			std::optional<double> yhigh;
			if (boost::regex_match(strs[0], integer)) {
				id = std::stoi(strs[0]);
			} else {
				throw std::runtime_error("Invalid ID for h2d tag, need positive integer");
			}

			if (boost::regex_match(strs[1], integer)) {
				nbinsx = std::stoi(strs[1]);
			} else {
				nbinsx = std::nullopt;
			}
			if (boost::regex_match(strs[2], number)) {
				xlow = std::stod(strs[2]);
			} else {
				xlow = std::nullopt;
			}
			if (boost::regex_match(strs[3], number)) {
				xhigh = std::stod(strs[3]);
			} else {
				xhigh = std::nullopt;
			}

			if (boost::regex_match(strs[4], integer)) {
				nbinsy = std::stoi(strs[4]);
			} else {
				nbinsy = std::nullopt;
			}
			if (boost::regex_match(strs[5], number)) {
				ylow = std::stod(strs[5]);
			} else {
				ylow = std::nullopt;
			}
			if (boost::regex_match(strs[6], number)) {
				yhigh = std::stod(strs[6]);
			} else {
				yhigh = std::nullopt;
			}
			htags[id] = {nbinsx, xlow, xhigh, nbinsy, ylow, yhigh};
		} else {
			throw std::runtime_error("Invalid number of args to h2d flag");
		}
	}
}

bool ValidateH2DTags(const std::map<int, h2dtag>& htags) {
	bool IsValid = true;
	bool InputOk = false;
	bool Continue = false;

	for (const auto& i : htags) {
		auto id = i.first;
		auto nbinsx = std::get<0>(i.second).value_or(-1);
		auto xlow = std::get<1>(i.second).value_or(-1.0);
		auto xhigh = std::get<2>(i.second).value_or(-1.0);
		auto nbinsy = std::get<3>(i.second).value_or(-1);
		auto ylow = std::get<4>(i.second).value_or(-1.0);
		auto yhigh = std::get<5>(i.second).value_or(-1.0);
		do {
			std::string q = "Is this tag valid '<Histogram id=\"{}\" ";
			int j = 0;
			if (std::get<0>(i.second).has_value()) {
				j += 2;
				q += "nbinsx=\"{}\" ";
			}
			if (std::get<1>(i.second).has_value()) {
				j += 4;
				q += "xlow=\"{}\" ";
			}
			if (std::get<2>(i.second).has_value()) {
				j += 8;
				q += "xhigh=\"{}\" ";
			}
			if (std::get<3>(i.second).has_value()) {
				j += 16;
				q += "nbinsy=\"{}\" ";
			}
			if (std::get<4>(i.second).has_value()) {
				j += 32;
				q += "ylow=\"{}\" ";
			}
			if (std::get<5>(i.second).has_value()) {
				j += 64;
				q += "yhigh=\"{}\" ";
			}
			q += "/>' ? (y/Y,n/N,c/C)";
			switch (j) {
			case 2:
				// spdlog::info(q,id,nbinsx);
				break;
			case 4:
				// spdlog::info(q,id,xlow);
				break;
			case 6:
				// spdlog::info(q,id,nbinsx,xlow);
				break;
			case 8:
				// spdlog::info(q,id,xhigh);
				break;
			case 10:
				// spdlog::info(q,id,nbinsx,xhigh);
				break;
			case 12:
				// spdlog::info(q,id,xlow,xhigh);
				break;
			case 14:
				// spdlog::info(q,id,nbinsx,xlow,xhigh);
				break;
			case 16:
				// spdlog::info(q,id,nbinsy);
				break;
			case 18:
				// spdlog::info(q,id,nbinsx,nbinsy);
				break;
			case 20:
				// spdlog::info(q,id,xlow,nbinsy);
				break;
			case 22:
				// spdlog::info(q,id,nbinsx,xlow,nbinsy);
				break;
			case 24:
				// spdlog::info(q,id,xhigh,nbinsy);
				break;
			case 26:
				// spdlog::info(q,id,nbinsx,xhigh,nbinsy);
				break;
			case 28:
				// spdlog::info(q,id,xlow,xhigh,nbinsy);
				break;
			case 30:
				// spdlog::info(q,id,nbinsx,xlow,xhigh,nbinsy);
				break;
			case 32:
				// spdlog::info(q,id,ylow);
				break;
			case 34:
				// spdlog::info(q,id,nbinsx,ylow);
				break;
			case 36:
				// spdlog::info(q,id,xlow,ylow);
				break;
			case 38:
				// spdlog::info(q,id,nbinsx,xlow,ylow);
				break;
			case 40:
				// spdlog::info(q,id,xhigh,ylow);
				break;
			case 42:
				// spdlog::info(q,id,nbinsx,xhigh,ylow);
				break;
			case 44:
				// spdlog::info(q,id,xlow,xhigh,ylow);
				break;
			case 46:
				// spdlog::info(q,id,nbinsx,xlow,xhigh,ylow);
				break;
			case 48:
				// spdlog::info(q,id,nbinsy,ylow);
				break;
			case 50:
				// spdlog::info(q,id,nbinsx,nbinsy,ylow);
				break;
			case 52:
				// spdlog::info(q,id,xlow,nbinsy,ylow);
				break;
			case 54:
				// spdlog::info(q,id,nbinsx,xlow,nbinsy,ylow);
				break;
			case 56:
				// spdlog::info(q,id,xhigh,nbinsy,ylow);
				break;
			case 58:
				// spdlog::info(q,id,nbinsx,xhigh,nbinsy,ylow);
				break;
			case 62:
				// spdlog::info(q,id,xlow,xhigh,nbinsy,ylow);
				break;
			case 64:
				// spdlog::info(q,id,yhigh);
				break;
			case 66:
				// spdlog::info(q,id,nbinsx,yhigh);
				break;
			case 68:
				// spdlog::info(q,id,xlow,yhigh);
				break;
			case 70:
				// spdlog::info(q,id,nbinsx,xlow,yhigh);
				break;
			case 72:
				// spdlog::info(q,id,xhigh,yhigh);
				break;
			case 74:
				// spdlog::info(q,id,nbinsx,xhigh,yhigh);
				break;
			case 76:
				// spdlog::info(q,id,xlow,xhigh,yhigh);
				break;
			case 78:
				// spdlog::info(q,id,nbinsx,xlow,xhigh,yhigh);
				break;
			case 80:
				// spdlog::info(q,id,nbinsy,yhigh);
				break;
			case 82:
				// spdlog::info(q,id,nbinsx,nbinsy,yhigh);
				break;
			case 84:
				// spdlog::info(q,id,xlow,nbinsy,yhigh);
				break;
			case 86:
				// spdlog::info(q,id,nbinsx,xlow,nbinsy,yhigh);
				break;
			case 88:
				// spdlog::info(q,id,xhigh,nbinsy,yhigh);
				break;
			case 90:
				// spdlog::info(q,id,nbinsx,xhigh,nbinsy,yhigh);
				break;
			case 92:
				// spdlog::info(q,id,xlow,xhigh,nbinsy,yhigh);
				break;
			case 94:
				// spdlog::info(q,id,nbinsx,xlow,xhigh,nbinsy,yhigh);
				break;
			case 96:
				// spdlog::info(q,id,ylow,yhigh);
				break;
			case 98:
				// spdlog::info(q,id,nbinsx,ylow,yhigh);
				break;
			case 100:
				// spdlog::info(q,id,xlow,ylow,yhigh);
				break;
			case 102:
				// spdlog::info(q,id,nbinsx,xlow,ylow,yhigh);
				break;
			case 104:
				// spdlog::info(q,id,xhigh,ylow,yhigh);
				break;
			case 106:
				// spdlog::info(q,id,nbinsx,xhigh,ylow,yhigh);
				break;
			case 108:
				// spdlog::info(q,id,xlow,xhigh,ylow,yhigh);
				break;
			case 110:
				// spdlog::info(q,id,nbinsx,xlow,xhigh,ylow,yhigh);
				break;
			case 112:
				// spdlog::info(q,id,nbinsy,ylow,yhigh);
				break;
			case 114:
				// spdlog::info(q,id,nbinsx,nbinsy,ylow,yhigh);
				break;
			case 116:
				// spdlog::info(q,id,xlow,nbinsy,ylow,yhigh);
				break;
			case 118:
				// spdlog::info(q,id,nbinsx,xlow,nbinsy,ylow,yhigh);
				break;
			case 120:
				// spdlog::info(q,id,xhigh,nbinsy,ylow,yhigh);
				break;
			case 122:
				// spdlog::info(q,id,nbinsx,xhigh,nbinsy,ylow,yhigh);
				break;
			case 124:
				// spdlog::info(q,id,xlow,xhigh,nbinsy,ylow,yhigh);
				break;
			default:
				throw std::runtime_error("Invalid Histogram tag generation, only ID provided");
				break;
			}
			AskUser(InputOk, IsValid, Continue);
		} while (not InputOk);

		if (Continue) {
			break;
		}
	}
	return IsValid;
}

int main(int argc, char* argv[]) {
	std::string procname;
	std::vector<std::string> files;
	std::vector<std::string> isotopes;
	std::vector<std::string> gates;
	std::vector<std::string> boxes;
	std::vector<std::string> cuts;
	std::vector<std::string> h1d;
	std::vector<std::string> h2d;
	bool validate;

	// clang-format off
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("processor,p", boost::program_options::value<std::string>(&procname), "name of the processor to insert the info into, it does not matter if it is nested")
		("file,f", boost::program_options::value<std::vector<std::string>>(&files)->multitoken(), "config files which to edit")
		("isotope,i", boost::program_options::value<std::vector<std::string>>(&isotopes)->multitoken(), "isotope cut to add to the config files formatted as name:cutid:filename")
		("cut,c", boost::program_options::value<std::vector<std::string>>(&cuts)->multitoken(), "general cut to add to the config files formatted as name:filename")
		("gate,g", boost::program_options::value<std::vector<std::string>>(&gates)->multitoken(), "gates to add to the config files formatted as label:lowerbound:upperbound")
		("box,b", boost::program_options::value<std::vector<std::string>>(&boxes)->multitoken(), "boxes to add to the config files formatted as label:xlowerbound:xupperbound:ylowerbound:yupperbound")
		("h1d", boost::program_options::value<std::vector<std::string>>(&h1d)->multitoken(), "1d histogram to add to the config files formatted as id:nx:xlow:xhigh, to specify the default for either nx, xlow, or xhigh leave it empty like this 20::1.0:2.0, will set id=20 to be between 1.0 and 2.0, but use the default binsize")
		("h2d", boost::program_options::value<std::vector<std::string>>(&h2d)->multitoken(), "2d histogram to add to the config files formatted as id:nx:xlow:xhigh:ny:ylow:yhigh, to specify the default for either nx, xlow, xhigh, ny, ylow, or yhigh leave it empty like this 2000:100:::100::: will set id=2000 to have 100 bins in both x and y with default bounds")
		("validate,v", boost::program_options::value<bool>(&validate)->default_value(true), "validate the parameters to change before modifying the input files");

	boost::program_options::positional_options_description p;
	p.add("file", -1);
	// clang-format on

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			// spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		if (files.size() < 1) {
			throw std::runtime_error("No input files given, see usage.");
		} else {
			// spdlog::info("Provided {} config files to modify",files.size());
		}

		std::map<std::string, int> fcount;
		std::vector<std::pair<std::string, pugi::xml_document>> docs;
		for (const auto& f : files) {
			if (fcount.find(f) == fcount.end()) {
				fcount[f] = 1;
				pugi::xml_document inputconfig;
				auto loadres = inputconfig.load_file(f.c_str());
				if (not loadres) {
					throw std::runtime_error(loadres.description());
				} else {
					// pugi::xml_node Configuration = inputconfig.child("Configuration");
					docs.push_back({f, std::move(inputconfig)});
				}
			} else {
				fcount[f] += 1;
				// spdlog::warn("Found duplicate file : {}, has been input {} times so far. Not modifying it multiple times.",f,fcount[f]);
			}
			std::filesystem::path p(f);
			if (not std::filesystem::exists(p)) {
				std::string err = "Input file: " + f + " does not exist, not changing any config file";
				throw std::runtime_error(err);
			}
		}
		// deduplicate the input files?

		bool IsValid = true;

		// split apart the strings and then show to the user and ask if they're valid before overwriting the input file
		std::map<std::string, isotopetag> itags;
		if (isotopes.size() > 0) {
			// spdlog::info("Found {} isotope tags to generate for {} processor",isotopes.size(),procname);
			ParseIsotopes(isotopes, itags);
			if (validate) {
				IsValid &= ValidateIsotopeTags(itags);
			}
		}
		if (not IsValid) {
			throw std::runtime_error("Provided invalid isotope tag, not changing any config file");
		}

		std::map<std::string, cuttag> ctags;
		if (cuts.size() > 0) {
			// spdlog::info("Found {} cut tags to generate for {} processor",cuts.size(),procname);
			ParseCuts(cuts, ctags);
			if (validate) {
				IsValid &= ValidateCutTags(ctags);
			}
		}
		if (not IsValid) {
			throw std::runtime_error("Provided invalid cut tag, not changing any config file");
		}

		std::map<std::string, gatetag> gtags;
		if (gates.size() > 0) {
			// spdlog::info("Found {} gate tags to generate for {} processor",gates.size(),procname);
			ParseGates(gates, gtags);
			if (validate) {
				IsValid &= ValidateGateTags(gtags);
			}
		}
		if (not IsValid) {
			throw std::runtime_error("Provided invalid gate tag, not changing any config file");
		}

		std::map<std::string, boxtag> btags;
		if (boxes.size() > 0) {
			// spdlog::info("Found {} box tags to generate for {} processor",boxes.size(),procname);
			ParseBoxes(boxes, btags);
			if (validate) {
				IsValid &= ValidateBoxTags(btags);
			}
		}
		if (not IsValid) {
			throw std::runtime_error("Provided invalid gate tag, not changing any config file");
		}

		std::map<int, h1dtag> h1tags;
		if (h1d.size() > 0) {
			// spdlog::info("Found {} h1d tags to generate for {} processor",h1d.size(),procname);
			ParseH1D(h1d, h1tags);
			if (validate) {
				IsValid &= ValidateH1DTags(h1tags);
			}
		}
		if (not IsValid) {
			throw std::runtime_error("Provided invalid h1d tag, not changing any config file");
		}

		std::map<int, h2dtag> h2tags;
		if (h2d.size() > 0) {
			// spdlog::info("Found {} h2d tags to generate for {} processor",h2d.size(),procname);
			ParseH2D(h2d, h2tags);
			if (validate) {
				IsValid &= ValidateH2DTags(h2tags);
			}
		}
		if (not IsValid) {
			throw std::runtime_error("Provided invalid h2d tag, not changing any config file");
		}

		for (auto& [f, inputconfig] : docs) {
			pugi::xml_node Configuration = inputconfig.child("Configuration");
			auto proc = Configuration.find_node(processor_predicate(procname));
			if (proc) {
				// need to find Histogram subset
				for (const auto& kv : itags) {
					std::string query = "Isotope[@name='" + kv.first + "']";
					pugi::xpath_node_set nodes = proc.select_nodes(query.c_str());
					if (nodes.size() > 1) {
						std::string err = "Found duplicate isotope name of " + kv.first + " within processor " + procname + ", not modifying anything";
						throw std::runtime_error(err);
					} else if (nodes.size() == 1) {
						// one already exists, just update things
						auto isotope = nodes[0].node();
						auto cutid = isotope.attribute("cutid");
						cutid.set_value(std::get<0>(kv.second).c_str());
						auto filename = isotope.attribute("filename");
						filename.set_value(std::get<1>(kv.second).c_str());
					} else {
						// node does not exist
						auto isotope = proc.append_child("Isotope");
						isotope.append_attribute("name") = kv.first.c_str();
						isotope.append_attribute("cutid") = std::get<0>(kv.second).c_str();
						isotope.append_attribute("filename") = std::get<1>(kv.second).c_str();
					}
				}
				for (const auto& kv : ctags) {
					std::string query = "Cut[@name='" + kv.first + "']";
					pugi::xpath_node_set nodes = proc.select_nodes(query.c_str());
					if (nodes.size() > 1) {
						std::string err = "Found duplicate cut name of " + kv.first + " within processor " + procname + ", not modifying anything";
						throw std::runtime_error(err);
					} else if (nodes.size() == 1) {
						// one already exists, just update things
						auto cut = nodes[0].node();
						auto filename = cut.attribute("filename");
						filename.set_value(std::get<0>(kv.second).c_str());
					} else {
						// node does not exist
						auto cut = proc.append_child("Cut");
						cut.append_attribute("name") = kv.first.c_str();
						cut.append_attribute("filename") = std::get<0>(kv.second).c_str();
					}
				}
				for (const auto& kv : gtags) {
					std::string query = "Gate[@label='" + kv.first + "']";
					pugi::xpath_node_set nodes = proc.select_nodes(query.c_str());
					if (nodes.size() > 1) {
						std::string err = "Found duplicate gate name of " + kv.first + " within processor " + procname + ", not modifying anything";
						throw std::runtime_error(err);
					} else if (nodes.size() == 1) {
						// one already exists, just update things
						auto gate = nodes[0].node();
						auto lowerbound = gate.attribute("lowerbound");
						lowerbound.set_value(std::get<0>(kv.second));
						auto upperbound = gate.attribute("upperbound");
						upperbound.set_value(std::get<1>(kv.second));
					} else {
						// node does not exist
						auto gate = proc.append_child("Gate");
						gate.append_attribute("label") = kv.first.c_str();
						gate.append_attribute("lowerbound") = std::get<0>(kv.second);
						gate.append_attribute("upperbound") = std::get<1>(kv.second);
					}
				}
				for (const auto& kv : btags) {
					std::string query = "BoxGate[@label='" + kv.first + "']";
					pugi::xpath_node_set nodes = proc.select_nodes(query.c_str());
					if (nodes.size() > 1) {
						std::string err = "Found duplicate gate name of " + kv.first + " within processor " + procname + ", not modifying anything";
						throw std::runtime_error(err);
					} else if (nodes.size() == 1) {
						// one already exists, just update things
						auto gate = nodes[0].node();
						auto xlowerbound = gate.attribute("xlowerbound");
						xlowerbound.set_value(std::get<0>(kv.second));
						auto xupperbound = gate.attribute("xupperbound");
						xupperbound.set_value(std::get<1>(kv.second));
						auto ylowerbound = gate.attribute("ylowerbound");
						ylowerbound.set_value(std::get<2>(kv.second));
						auto yupperbound = gate.attribute("yupperbound");
						yupperbound.set_value(std::get<3>(kv.second));
					} else {
						// node does not exist
						auto gate = proc.append_child("BoxGate");
						gate.append_attribute("label") = kv.first.c_str();
						gate.append_attribute("xlowerbound") = std::get<0>(kv.second);
						gate.append_attribute("xupperbound") = std::get<1>(kv.second);
						gate.append_attribute("ylowerbound") = std::get<2>(kv.second);
						gate.append_attribute("yupperbound") = std::get<3>(kv.second);
					}
				}

				for (const auto& kv : h1tags) {
					std::string query = "Histogram[@id='" + std::to_string(kv.first) + "']";
					pugi::xpath_node_set nodes = proc.select_nodes(query.c_str());
					if (nodes.size() > 1) {
						std::string err = "Found duplicate histogram id of " + std::to_string(kv.first) + " within processor " + procname + ", not modifying anything";
						throw std::runtime_error(err);
					} else if (nodes.size() == 1) {
						// one already exists, just update things
						auto his = nodes[0].node();
						if (std::get<0>(kv.second).has_value()) {
							auto nbinsx = his.attribute("nbinsx");
							nbinsx.set_value(std::get<0>(kv.second).value());
						}
						if (std::get<1>(kv.second).has_value()) {
							auto xlow = his.attribute("xlow");
							xlow.set_value(std::get<1>(kv.second).value());
						}
						if (std::get<2>(kv.second).has_value()) {
							auto xhigh = his.attribute("xhigh");
							xhigh.set_value(std::get<2>(kv.second).value());
						}
					} else {
						// node does not exist
						auto his = proc.append_child("Histogram");
						his.append_attribute("id") = kv.first;
						if (std::get<0>(kv.second).has_value()) {
							his.append_attribute("nbinsx") = std::get<0>(kv.second).value();
						}
						if (std::get<1>(kv.second).has_value()) {
							his.append_attribute("xlow") = std::get<1>(kv.second).value();
						}
						if (std::get<2>(kv.second).has_value()) {
							his.append_attribute("xhigh") = std::get<2>(kv.second).value();
						}
					}
				}
				for (const auto& kv : h2tags) {
					std::string query = "Histogram[@id='" + std::to_string(kv.first) + "']";
					pugi::xpath_node_set nodes = proc.select_nodes(query.c_str());
					if (nodes.size() > 1) {
						std::string err = "Found duplicate histogram id of " + std::to_string(kv.first) + " within processor " + procname + ", not modifying anything";
						throw std::runtime_error(err);
					} else if (nodes.size() == 1) {
						// one already exists, just update things
						auto his = nodes[0].node();
						if (std::get<0>(kv.second).has_value()) {
							auto nbinsx = his.attribute("nbinsx");
							nbinsx.set_value(std::get<0>(kv.second).value());
						}
						if (std::get<1>(kv.second).has_value()) {
							auto xlow = his.attribute("xlow");
							xlow.set_value(std::get<1>(kv.second).value());
						}
						if (std::get<2>(kv.second).has_value()) {
							auto xhigh = his.attribute("xhigh");
							xhigh.set_value(std::get<2>(kv.second).value());
						}
						if (std::get<3>(kv.second).has_value()) {
							auto nbinsy = his.attribute("nbinsy");
							nbinsy.set_value(std::get<3>(kv.second).value());
						}
						if (std::get<4>(kv.second).has_value()) {
							auto ylow = his.attribute("ylow");
							ylow.set_value(std::get<4>(kv.second).value());
						}
						if (std::get<5>(kv.second).has_value()) {
							auto yhigh = his.attribute("yhigh");
							yhigh.set_value(std::get<5>(kv.second).value());
						}
					} else {
						// node does not exist
						auto his = proc.append_child("Histogram");
						his.append_attribute("id") = kv.first;
						if (std::get<0>(kv.second).has_value()) {
							his.append_attribute("nbinsx") = std::get<0>(kv.second).value();
						}
						if (std::get<1>(kv.second).has_value()) {
							his.append_attribute("xlow") = std::get<1>(kv.second).value();
						}
						if (std::get<2>(kv.second).has_value()) {
							his.append_attribute("xhigh") = std::get<2>(kv.second).value();
						}
						if (std::get<3>(kv.second).has_value()) {
							his.append_attribute("nbinsy") = std::get<3>(kv.second).value();
						}
						if (std::get<4>(kv.second).has_value()) {
							his.append_attribute("ylow") = std::get<4>(kv.second).value();
						}
						if (std::get<5>(kv.second).has_value()) {
							his.append_attribute("yhigh") = std::get<5>(kv.second).value();
						}
					}
				}
				// if( h1d.size() > 0 ){
				//	pugi::xpath_node_set nodes = proc.select_nodes("Histogram");
				//	for(pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it){
				//		it->node().print(std::cout);
				//	}
				// }
				// need to generate new file name
				std::filesystem::path currfilename(f);
				currfilename.replace_extension(".inserted.xml");
				inputconfig.save_file(currfilename.c_str());
			} else {
				// spdlog::warn("config {} does not have processor named {}, skipping over it",f,procname);
			}
		}

	} catch (std::exception& e) {
		// spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}
}
