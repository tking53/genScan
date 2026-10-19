#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "yaml-cpp/emitter.h"
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

#include "StringManipFunctions.hpp"

template<class T>
struct Kernel {
	T weight;
	boost::circular_buffer<T> values;
	T sum;
	std::vector<T> datavalues;
	Kernel(const std::vector<T>& v, size_t b, size_t sz, const T& w = 1.0)
		: weight(w) {
		values = boost::circular_buffer<T>(v.begin() + b, v.begin() + b + sz);
		for (auto& d : values) {
			d *= weight;
		}
		sum = std::accumulate(values.begin(), values.end(), 0.0);
		datavalues.push_back(sum);

		for (size_t ii = b + sz; ii < v.size(); ++ii) {
			sum -= values.front();
			values.push_back(v.at(ii) * weight);
			sum += values.back();
			datavalues.push_back(sum);
		}
	}
	T& operator[](size_t idx) {
		return datavalues[idx];
	}

	const T& operator[](size_t idx) const {
		return datavalues.at(idx);
	}

	size_t size() const {
		return datavalues.size();
	}
};

template<class T>
struct SNIP {
	std::vector<T> curriter;
	std::vector<T> previter;

	SNIP(const std::vector<T>& data, size_t iter) {
		previter = std::vector<T>(data.begin(), data.end());
		for (auto& p : previter) {
			p = std::log(std::log(std::sqrt(p + 1) + 1) + 1);
		}
		curriter = std::vector<T>(previter.begin(), previter.end());
		for (size_t pp = 0; pp < iter; ++pp) {
			for (size_t ii = pp; ii < (data.size() - pp); ++ii) {
				curriter[ii] = std::min<T>(previter[ii], (previter[ii - pp] + previter[ii + pp]) / 2.0);
			}
			previter = curriter;
		}
		for (auto& d : curriter) {
			auto dd = std::exp(std::exp(d) - 1.0) - 1.0;
			d = dd * dd - 1.0;
		}
	}

	T& operator[](size_t idx) {
		return curriter[idx];
	}

	const T& operator[](size_t idx) const {
		return curriter.at(idx);
	}
};

std::vector<std::pair<double, double>> ParseGates(const std::vector<std::string>& values) {
	std::vector<std::pair<double, double>> retvals;
	for (const auto& s : values) {
		std::vector<std::string> strs;
		boost::split(strs, s, boost::is_any_of(":"));
		boost::regex number("^(?!-0(\\.0+)?(e|$))-?(0|[1-9]\\d*)(\\.\\d+)?(e-?(0|[1-9]\\d*))?");

		if (strs.size() == 2) {
			boost::smatch lmatch;
			boost::smatch umatch;
			if (boost::regex_match(strs[0], lmatch, number) and boost::regex_match(strs[1], lmatch, number)) {
				retvals.push_back({std::stod(strs[0]), std::stod(strs[1])});
			} else {
				throw std::runtime_error("Invalid bounded pair, not two numbers");
			}
		} else {
			throw std::runtime_error("Unable to parse input");
		}
	}
	return retvals;
}

struct PeakLocator {
	PeakLocator(TH1* hist, int length, int sigma, float threshold)
		: hist(hist)
		, length(length)
		, sigma(sigma)
		, threshold(threshold) {
		for (int ii = 1; ii < hist->GetNbinsX() + 1; ++ii) {
			histoxvals.push_back(hist->GetBinCenter(ii));
			histoyvals.push_back(hist->GetBinContent(ii));
		}

		if (histoyvals.size() <= (3 * length)) {
			spdlog::error("Inappropriate length choice 3*{} exceeds total histo length: {}", length, histoyvals.size());
			exit(EXIT_FAILURE);
		}

		SNIP<float> filter(histoyvals, length);
		std::vector<float> clipped_spectrum(histoxvals.size(), 0.0);
		for (size_t ii = 0; ii < histoxvals.size(); ++ii) {
			clipped_spectrum[ii] = histoyvals[ii] - filter[ii];
		}

		Kernel<float> preregion(clipped_spectrum, 0, sigma, 1.0);
		Kernel<float> midregion(clipped_spectrum, sigma, sigma, 2.0);
		Kernel<float> postregion(clipped_spectrum, 2 * sigma, sigma, 1.0);

		std::vector<float> cfar(sigma + (sigma - 1) / 2, 0.0);
		for (size_t ii = 0; ii < std::min({preregion.datavalues.size(), midregion.datavalues.size(), postregion.datavalues.size()}); ++ii) {
			cfar.push_back(midregion[ii] - (preregion[ii] + postregion[ii]));
		}
		for (size_t ii = 0; ii < (sigma + sigma / 2); ++ii) {
			cfar.push_back(0.0);
		}

		std::vector<bool> mask;
		for (const auto& e : cfar) {
			mask.push_back(e > 0.0);
		}

		std::vector<size_t> current_peak;
		for (size_t ii = 0; ii < cfar.size(); ++ii) {
			if (mask[ii]) {
				current_peak.push_back(ii);
			} else {
				if (current_peak.size() > 0) {
					float pk = 0.0;
					float pk_height = 0.0;
					for (const auto& jj : current_peak) {
						pk_height += cfar[jj];
						pk += histoxvals[jj];
					}
					pk /= static_cast<float>(current_peak.size());
					pk_height /= static_cast<float>(current_peak.size());
					peak_locations.push_back({pk, pk_height});
					current_peak.clear();
				}
			}
		}
		if (current_peak.size() > 0) {
			float pk = 0.0;
			float pk_height = 0.0;
			for (const auto& jj : current_peak) {
				pk_height += cfar[jj];
				pk += histoxvals[jj];
			}
			pk /= static_cast<float>(current_peak.size());
			pk_height /= static_cast<float>(current_peak.size());
			peak_locations.push_back({pk, pk_height});
			current_peak.clear();
		}

		if (peak_locations.size() < 1) {
			throw std::runtime_error("No Peaks Found");
		}

		std::sort(peak_locations.begin(), peak_locations.end(),
			  [](const std::pair<float, float>& a, const std::pair<float, float>& b) {
				  return a.second > b.second;
			  });
	}

	TH1* hist;
	int length;
	int sigma;
	float threshold;
	std::vector<float> histoxvals;
	std::vector<float> histoyvals;
	std::vector<std::pair<float, float>> peak_locations;
};

YAML::Emitter& operator<<(YAML::Emitter& out, const PeakLocator* pk) {
	out << YAML::BeginMap
	    << YAML::Key << "HisName" << YAML::Value << pk->hist->GetName()
	    << YAML::Key << "Length" << YAML::Value << pk->length
	    << YAML::Key << "Sigma" << YAML::Value << pk->sigma
	    << YAML::Key << "Threshold" << YAML::Value << pk->threshold;
	out << YAML::Key << "Locations" << YAML::Value;
	out << YAML::BeginSeq;
	for (size_t ii = 0; ii < pk->peak_locations.size(); ++ii) {
		if (pk->peak_locations[ii].second > pk->peak_locations[0].second * pk->threshold) {
			out << YAML::BeginMap;
			auto bin = pk->hist->FindBin(pk->peak_locations[ii].first);
			out << YAML::Key << "Energy" << pk->peak_locations[ii].first;
			out << YAML::Key << "Counts" << pk->hist->GetBinContent(bin);
			out << YAML::EndMap;
		}
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;
	return out;
}

int main(int argc, char* argv[]) {
	std::string axis;
	std::string hisname;
	std::string inputfile;
	std::string outputfile;
	int length;
	int sigma;
	float threshold;
	std::vector<int> indices;
	int dimensionality;
	std::vector<std::pair<double, double>> gatevalues;
	std::vector<std::string> gates;

	// clang-format off
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("axis,a", boost::program_options::value<std::string>(&axis)->default_value("x"), "axis to project onto (x,y,X,Y) if 2D")
		("data,d", boost::program_options::value<std::string>(&hisname), "histogram to manipulate")
		("gate,g", boost::program_options::value<std::vector<std::string>>(&gates)->multitoken(), "values to gate within in 2d histogram")
		("help,h", "produce help message")
		("inputfile,i", boost::program_options::value<std::string>(&inputfile), "root file to pull data from")
		("length,l", boost::program_options::value<int>(&length)->default_value(10), "length of filter in bins")
		("numdimension,n", boost::program_options::value<int>(&dimensionality)->default_value(1), "dimensionality of histogram (1,2)")
		("outputfile,o", boost::program_options::value<std::string>(&outputfile)->default_value("Peaks.yaml"), "yaml outputfile")
		("projectionindex,p", boost::program_options::value<std::vector<int>>(&indices)->multitoken(), "index to project on if 2d histogram")
		("sigma,s", boost::program_options::value<int>(&sigma)->default_value(10), "rough sigma of peak in bins")
		("threshold,t", boost::program_options::value<float>(&threshold)->default_value(0.05), "fraction [0,1] of max peak to include when dumping peaks");

	boost::program_options::positional_options_description pos;
	// clang-format on

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(pos).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		auto numproj = indices.size();
		auto numgates = gatevalues.size();

		gatevalues = ParseGates(gates);

		if (dimensionality == 2 and numproj < 1 and numgates < 1) {
			spdlog::error("dimensionality is 2, but no projections or gates given, and not fitting a 2D dataset");
			exit(EXIT_FAILURE);
		}

		axis = StringManip::tolower(axis);
		if (axis.compare("x") != 0 and axis.compare("y") != 0) {
			spdlog::error("unknown axis projection : {}", axis);
			exit(EXIT_FAILURE);
		}

		if (not vm.count("data")) {
			spdlog::error("Not provided histogram to fit");
			exit(EXIT_FAILURE);
		}

		if (not vm.count("inputfile")) {
			spdlog::error("Not provided inputfile containing histogram to fit");
			exit(EXIT_FAILURE);
		}

	} catch (std::exception& e) {
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}

	try {
		auto rfile = new TFile(inputfile.c_str(), "READ");
		auto mainhis = rfile->Get(hisname.c_str());
		std::vector<PeakLocator*> pks;
		if (mainhis != nullptr) {
			auto histype = std::string(mainhis->ClassName());
			boost::regex re2d("TH2");
			boost::regex re1d("TH1");
			TH1* hist;
			if (boost::regex_search(histype, re2d)) {
				for (const auto& idx : indices) {
					auto name = std::string(mainhis->GetName()) + "_proj_" + axis + std::to_string(idx);
					if (axis.compare("x") == 0) {
						hist = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(), idx, idx);
					} else {
						hist = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(), idx, idx);
					}
					hist->SetDirectory(0);
					pks.push_back(new PeakLocator(hist, length, sigma, threshold));
				}
				int idx = 0;
				for (const auto& g : gatevalues) {
					auto name = std::string(mainhis->GetName()) + "_gate_" + axis + std::to_string(idx);
					if (axis.compare("x") == 0) {
						auto minbin = dynamic_cast<TH2*>(mainhis)->GetYaxis()->FindBin(g.first);
						auto maxbin = dynamic_cast<TH2*>(mainhis)->GetYaxis()->FindBin(g.second);
						hist = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(), minbin, maxbin);
					} else {
						auto minbin = dynamic_cast<TH2*>(mainhis)->GetXaxis()->FindBin(g.first);
						auto maxbin = dynamic_cast<TH2*>(mainhis)->GetXaxis()->FindBin(g.second);
						hist = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(), minbin, maxbin);
					}
					hist->SetDirectory(0);
					pks.push_back(new PeakLocator(hist, length, sigma, threshold));
					++idx;
				}
			} else if (boost::regex_search(histype, re1d)) {
				hist = dynamic_cast<TH1*>(mainhis);
				hist->SetDirectory(0);
				pks.push_back(new PeakLocator(hist, length, sigma, threshold));
			} else {
				throw std::runtime_error("not passed a TH1 or TH2 histogram");
			}

			YAML::Emitter doc;
			doc << YAML::BeginMap;
			doc << YAML::Key << "InputFile" << YAML::Value << inputfile;
			doc << YAML::Key << "InputHistogram" << YAML::Value << hisname;
			doc << YAML::Key << "Peaks";
			doc << pks;
			doc << YAML::EndMap;

			std::ofstream out(outputfile);
			out << doc.c_str() << std::endl;
			out.close();

		} else {
			throw std::runtime_error("histogram does not exist");
		}

	} catch (std::exception& e) {
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}
	return 0;
}
