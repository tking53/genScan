///@authors T. King
/********************************************************************/
/* module_csrb_test.cpp                                              */
/*     Decode Pixie Module CSRB bitmask                              */
/********************************************************************/

#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace {

	constexpr const char* kGreen = "\033[32m";
	constexpr const char* kRed = "\033[31m";
	constexpr const char* kCyan = "\033[36m";
	constexpr const char* kBold = "\033[1m";
	constexpr const char* kReset = "\033[0m";

	const std::vector<std::string> kModuleCsrbBits({"Enable pullups for backplane bus lines",
							"",
							"",
							"",
							"Send triggers to trigger card and backplane",
							"",
							"Chassis master module",
							"Swap global fast trigger and validation inputs",
							"Swap external fast trigger and validation inputs",
							"",
							"Enable run inhibit signal input",
							"Sync runs and distribute triggers across chassis",
							"Sort events by timestamp",
							"Connect fast triggers to backplane"});

	bool ParseUInt32(const std::string& input, std::uint32_t& output) {
		try {
			std::size_t parsed = 0;
			const unsigned long value = std::stoul(input, &parsed, 0);
			if (parsed != input.size()) {
				return false;
			}
			if (value > std::numeric_limits<std::uint32_t>::max()) {
				return false;
			}
			output = static_cast<std::uint32_t>(value);
			return true;
		} catch (...) {
			return false;
		}
	}

	inline bool IsBitSet(const std::uint32_t value, const unsigned int bit) {
		return (value & (1u << bit)) != 0u;
	}

	template<typename T>
	std::string HighlightIfAny(const T value, const int width, const bool enabled) {
		const std::string cell = fmt::format("{:>{}}", value, width);
		if (enabled) {
			return fmt::format("{}{}{}", kGreen, cell, kReset);
		}
		return cell;
	}

	void PrintExamples() {
		spdlog::info("{}{}========================{}", kBold, kCyan, kReset);
		spdlog::info("{}{}SINGLE-CRATE TEMPLATE{}", kBold, kGreen, kReset);
		spdlog::info("{}{}========================{}", kBold, kCyan, kReset);
		spdlog::info("");
		spdlog::info("  System/Crate Director module -> 65");
		spdlog::info("    Bits set: 0, 6");
		spdlog::info("    Meaning: pullups + chassis master");
		spdlog::info("");
		spdlog::info("  Regular module -> 0");
		spdlog::info("    Bits set: none");
		spdlog::info("    Meaning: no special ModCSRB routing enabled");
		spdlog::info("");
		spdlog::info("{}{}======================={}", kBold, kCyan, kReset);
		spdlog::info("{}{}MULTI-CRATE TEMPLATE{}", kBold, kGreen, kReset);
		spdlog::info("{}{}======================={}", kBold, kCyan, kReset);
		spdlog::info("  System Director module in master crate -> 2129");
		spdlog::info("    Bits set: 0, 4, 6, 11");
		spdlog::info("    Meaning: pullups + trigger card/backplane + chassis master + sync");
		spdlog::info("");
		spdlog::info("  Crate Master module in secondary crate -> 2113");
		spdlog::info("    Bits set: 0, 6, 11");
		spdlog::info("    Meaning: pullups + chassis master + sync");
		spdlog::info("");
		spdlog::info("  Regular module -> 2048");
		spdlog::info("    Bits set: 11");
		spdlog::info("    Meaning: sync enabled only");
		spdlog::info("");
	}

	void PrintUsage(const char* prog) {
		spdlog::info("SYNTAX:");
		spdlog::info("  {} <MODULE_CSRB>", prog);
		spdlog::info("MODULE_CSRB should be given as decimal (e.g. 65).");
	}

	void DecodeModuleCsrb(const std::uint32_t module_csrb) {
		spdlog::info("\nModule CSRB Decode");
		spdlog::info("Input: 0x{:X} ({})", module_csrb, module_csrb);
		spdlog::info("{:>3} {:>8} {:>8}  {}", "Bit", "Value", "Total", "Bit Function");

		std::uint32_t running_total = 0u;
		for (unsigned int bit = 0; bit < kModuleCsrbBits.size(); ++bit) {
			const bool enabled = IsBitSet(module_csrb, bit);
			const std::uint32_t value = (1u << bit);
			if (enabled) {
				running_total += value;
			}

			const std::uint32_t row_total = enabled ? running_total : 0u;

			const std::string bit_col = HighlightIfAny(bit, 3, enabled);
			const std::string value_col = HighlightIfAny(value, 8, enabled);
			const std::string total_col = HighlightIfAny(row_total, 8, enabled);
			const std::string desc_col = HighlightIfAny((kModuleCsrbBits[bit]), 8, enabled);

			spdlog::info("{} {} {}  {}", bit_col, value_col, total_col, desc_col);
		}
	}

} // namespace

int main(int argc, char* argv[]) {
	spdlog::set_pattern("%v");

	std::vector<std::string> values;
	bool show_examples = false;

	// clang-format off
    boost::program_options::options_description visible_options("Generic Options");
    visible_options.add_options()
        ("help,h", "Produce help message");

    boost::program_options::options_description hidden_options("Hidden Options");
    hidden_options.add_options()
        ("examples,e", boost::program_options::bool_switch(&show_examples),
            "Print MODULE_CSRB reference examples")
        ("values", boost::program_options::value<std::vector<std::string>>(&values)->multitoken(),
            "MODULE_CSRB value");

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

		if (show_examples) {
			PrintExamples();
			return 0;
		}

		if (vm.count("help") || values.empty()) {
			std::stringstream stream;
			stream << visible_options;
			spdlog::info("{}", stream.str());
			PrintUsage(argv[0]);
			return 0;
		}

		if (values.size() != 1) {
			throw std::runtime_error(fmt::format(
				"Invalid number of MODULE_CSRB values: expected 1, received {}",
				values.size()));
		}
	} catch (const std::exception& e) {
		spdlog::error(e.what());
		PrintUsage(argv[0]);
		return 1;
	}

	std::uint32_t module_csrb = 0u;
	if (!ParseUInt32(values[0], module_csrb)) {
		spdlog::error("Failed to parse MODULE_CSRB value '{}'", values[0]);
		return 1;
	}

	DecodeModuleCsrb(module_csrb);
	return 0;
}
