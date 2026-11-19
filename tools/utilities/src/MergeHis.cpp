#include <filesystem>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <TFile.h>
#include <TH1.h>

#include <boost/program_options.hpp>

void MergeHistograms(const std::vector<std::string>& filenames,
		     const std::string& histogram_name,
		     const std::string& suffix_regex,
		     const std::string& output_filename,
		     const bool& full_path) {
	const std::regex pattern(suffix_regex);

	TFile output(output_filename.c_str(), "RECREATE");

	if (output.IsZombie()) {
		throw std::runtime_error("Could not create output file: " +
					 output_filename);
	}

	for (const auto& filename : filenames) {
		const std::string basename = full_path ? (filename) : std::filesystem::path(filename).filename().string();

		std::smatch match;

		if (!std::regex_search(basename, match, pattern) ||
		    match.size() < 2) {
			std::cerr << "Warning: could not extract suffix from "
				  << filename << '\n';
			continue;
		}

		const std::string suffix = match[1].str();
		const std::string new_name = histogram_name + "_" + suffix;

		TFile input(filename.c_str(), "READ");

		if (input.IsZombie()) {
			std::cerr << "Warning: could not open " << filename
				  << '\n';
			continue;
		}

		auto* hist = input.Get<TH1>(histogram_name.c_str());

		if (!hist) {
			std::cerr << "Warning: histogram '" << histogram_name
				  << "' not found in " << filename << '\n';
			input.Close();
			continue;
		}

		output.cd();

		auto* renamed =
			static_cast<TH1*>(hist->Clone(new_name.c_str()));
		renamed->SetDirectory(&output);
		renamed->Write();

		delete renamed;
		input.Close();

		std::cout << filename << " -> " << new_name << '\n';
	}

	output.Close();
}

int main(int argc, char* argv[]) {
	std::string histogram_name;
	std::string suffix_regex;
	std::string output_filename;
	std::vector<std::string> filenames;
	bool full_path = false;

	// clang-format off
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("output,o", boost::program_options::value<std::string>(&output_filename)->default_value("merged.root"), "output file name")
		("histogram,H", boost::program_options::value<std::string>(&histogram_name), "name of the histogram")
		("regex,r", boost::program_options::value<std::string>(&suffix_regex), "Regex used to extract the suffix")
		("input,i", boost::program_options::value<std::vector<std::string>>(&filenames), "list of files to perform the operation on")
		("full-path,f", "use full path in the regex"); 

	boost::program_options::positional_options_description p;
	p.add("input", -1);
	// clang-format on

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			std::cout << cmdline_options;
			exit(EXIT_SUCCESS);
		}

		full_path = vm.count("full-path");

	} catch (std::exception& e) {
		std::cerr << e.what();
		exit(EXIT_FAILURE);
	}

	if (histogram_name.empty() || suffix_regex.empty() || optind >= argc) {
		std::cout << cmdline_options;
		return 1;
	}

	try {
		MergeHistograms(filenames, histogram_name, suffix_regex,
				output_filename, full_path);
	} catch (const std::regex_error& e) {
		std::cerr << "Invalid regex: " << e.what() << '\n';
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << '\n';
		return 1;
	}

	return 0;
}

