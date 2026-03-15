///@authors D. Miller, C. Thornsberry
/********************************************************************/
/*	csr_test.cpp						                            */
/*		last updated: April 17th, 2015 (CRT)                        */
/*		Updated: March13th, 2026 to be standalone like set2ascii (TTK) */
/*      Ported to GenScan tools by T.T. King */
/********************************************************************/

#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <limits>

#include <boost/program_options.hpp>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

const std::vector<std::string> csra_txt({"",
					 "",
					 "Good Channel",
					 "",
					 "",
					 "Trigger positive",
					 "",
					 "", "Enable trace capture",
					 "Enable QDC sums capture",
					 "Enable CFD trigger mode",
					 "Enable global trigger validation", "Enable raw energy sums capture", "Enable channel trigger validation",
					 "LO/HI gain", "Pileup Rejection",
					 "Inverse Pileup ",
					 "", "SHE single trace capture (grouptrigsel)",
					 "",
					 "",
					 "Enable External Timestamping",
					 "Enable External Fast Trigger",
					 "Enable Override Peak Sample Point"});

bool ParseUInt32(const std::string& input, unsigned int& output) {
	try {
		std::size_t parsed = 0;
		const unsigned long value = std::stoul(input, &parsed, 0);
		if (parsed != input.size()) {
			return false;
		}
		if (value > std::numeric_limits<unsigned int>::max()) {
			return false;
		}
		output = static_cast<unsigned int>(value);
		return true;
	} catch (...) {
		return false;
	}
}

void PrintUsage(const char* prog) {
	spdlog::info("SYNTAX:");
	spdlog::info("  {} <csra>", prog);
	spdlog::info("csra can be decimal or hex (e.g. 65 or 0x41).");
}

bool Test(unsigned int input_) {
	const unsigned int num_bits_ = csra_txt.size();
	if (num_bits_ > 32) {
		return false;
	} // Too many bits for unsigned int

	bool* active_bits = new bool[num_bits_];
	unsigned int* bit_values = new unsigned int[num_bits_];
	unsigned int* running_total = new unsigned int[num_bits_];

	unsigned int total = 0;
	unsigned int count = 1;

	for (unsigned int i = 0; i < num_bits_; i++) {
		bit_values[i] = count;
		if (input_ & (1 << i)) {
			active_bits[i] = true;

			total += count;
			running_total[i] = total;
		} else {
			active_bits[i] = false;
			running_total[i] = 0;
		}
		count *= 2;
	}

	std::stringstream input_stream;
	input_stream << " Input: 0x" << std::hex << input_ << " (" << std::dec
		     << input_ << ")";
	spdlog::info("{}", input_stream.str());
	if (!csra_txt.empty()) {
		spdlog::info("  Bit  On?  Value       Total\t  Bit Function");
	} else {
		spdlog::info("  Bit   On?\tValue\t   Total");
	}

	std::string bit_function;
	for (unsigned int i = 0; i < num_bits_; i++) {
		if (!csra_txt.empty()) {
			bit_function = csra_txt[i];
		} else {
			bit_function = "";
		}

		const auto line = fmt::format("  {:02}\t{}   {:<12}{:<10}{}",
					      i,
					      active_bits[i] ? 1 : 0,
					      bit_values[i],
					      running_total[i],
					      bit_function);
		if (active_bits[i])
			spdlog::info("\033[32m{}\033[0m", line);
		else
			spdlog::info("{}", line);
	}

	delete[] active_bits;
	delete[] bit_values;
	delete[] running_total;

	return true;
}

int main(int argc, char* argv[]) {
	spdlog::set_pattern("%v");

	std::vector<std::string> values;

	// clang-format off
    boost::program_options::options_description visible_options("Generic Options");
    visible_options.add_options()
        ("help,h", "produce help message");

    boost::program_options::options_description hidden_options("Hidden Options");
    hidden_options.add_options()
        ("values", boost::program_options::value<std::vector<std::string>>(&values)->multitoken(),
            "csra value");

    boost::program_options::options_description cmdline_options("Allowed Options");
    cmdline_options.add(visible_options).add(hidden_options);

    boost::program_options::positional_options_description pos;
    pos.add("values", -1);
	// clang-format on

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv)
			      .options(cmdline_options)
			      .positional(pos)
			      .run(),
		      vm);
		notify(vm);

		if (vm.count("help") || values.empty()) {
			std::stringstream stream;
			stream << visible_options;
			spdlog::info("{}", stream.str());
			PrintUsage(argv[0]);
			return 0;
		}

		if (values.size() != 1) {
			throw std::runtime_error(fmt::format(
				"Invalid number of csra values: expected 1, received {}",
				values.size()));
		}
	} catch (const std::exception& e) {
		spdlog::error(e.what());
		PrintUsage(argv[0]);
		return 1;
	}

	unsigned int csra_value = 0u;
	if (!ParseUInt32(values[0], csra_value)) {
		spdlog::error("Failed to parse csra value '{}'", values[0]);
		return 1;
	}

	Test(csra_value);

	return 0;
}
