///@file resetdelay_test.cpp
///@author T.T. King
/********************************************************************/
/*  resetdelay_test.cpp                                             */
/*                                                                  */
/*  Encode and decode the Pixie ResetDelay parameter.               */
/********************************************************************/

#include <cctype>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/program_options.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

constexpr uint32_t RESET_DELAY_MASK = 0xFFFF;
constexpr uint32_t RESET_DELAY_ENABLE = (1u << 16);
constexpr uint32_t RESET_DELAY_MAX = 0x1FFFF;

double GetResetDelayTickUs(unsigned int frequency_mhz, char revision) {
	switch (revision) {
	case 'F':
		switch (frequency_mhz) {
		case 250:
			return 0.008; // 8 ns
		case 500:
			return 0.010; // 10 ns
		case 100:
			return 0.010; // 10 ns
		}
		break;

	case 'H':
		switch (frequency_mhz) {
		case 250:
			return 0.008; // 8 ns
		case 500:
			return 0.008; // 8 ns
		case 125:
			return 0.008; // 8 ns
		}
		break;
	}

	throw std::invalid_argument(
		fmt::format(
			"Unsupported module configuration: {} MHz Rev {}",
			frequency_mhz,
			revision));
}

/**
 * Print command-line usage information.
 */
void PrintUsage(const char* prog) {
	//clang-format off
	spdlog::info("SYNTAX:");
	spdlog::info(
		"  {} --encode <delay_us> [--state <on|off>] "
		"[--frequency <MHz>] [--revision <F|H>]",
		prog);

	spdlog::info(
		"  {} --decode <ResetDelay> "
		"[--frequency <MHz>] [--revision <F|H>]",
		prog);

	spdlog::info("");

	spdlog::info("MODES:");
	spdlog::info("  -e, --encode <us>     Encode blanking time in microseconds");
	spdlog::info("  -d, --decode <value>  Decode a ResetDelay register value");

	spdlog::info("");

	spdlog::info("ENCODE OPTIONS:");
	spdlog::info("  -b, --blanking <on|off>  Set blanking state");
	spdlog::info("                        Default: on");

	spdlog::info("");

	spdlog::info("MODULE:");
	spdlog::info("  -f, --frequency <MHz> Pixie module sampling frequency");
	spdlog::info("                        Default: 250 MHz");
	spdlog::info("  -r, --revision <F|H>  Pixie module hardware revision");
	spdlog::info("                        Default: F");

	spdlog::info("");

	spdlog::info("RESETDELAY:");
	spdlog::info("  ResetDelay[15:0]      Blanking length in clock ticks");
	spdlog::info("  ResetDelay[16]        Blanking enable");

	spdlog::info("");

	spdlog::info("SUPPORTED CONFIGURATIONS (Requires underlying VANDLE Firmware Support):");
	spdlog::info("  100 MHz Rev F         10 ns Base Clock tick");
	spdlog::info("  250 MHz Rev F          8 ns Base Clock tick");
	spdlog::info("  500 MHz Rev F         10 ns Base Clock tick");
	spdlog::info("  125 MHz Rev H          8 ns Base Clock tick");
	spdlog::info("  250 MHz Rev H          8 ns Base Clock tick");
	spdlog::info("  500 MHz Rev H          8 ns Base Clock tick");

	spdlog::info("");

	spdlog::info("EXAMPLES:");
	spdlog::info("  {} --encode 5", prog);
	spdlog::info("  {} -e 5 -s off", prog);
	spdlog::info("  {} -e 1.5 -f 500 -r F", prog);
	spdlog::info("  {} --decode 66161", prog);
	spdlog::info("  {} -d 0x10271", prog);
	spdlog::info("  {} -d 66161 -f 250 -r H", prog);
	//clang-format on
}

int main(int argc, char* argv[]) {
	spdlog::set_pattern("%v");

	double encode_us = 0.0;
	std::string decode_value;

	std::string blanking = "on";
	unsigned int frequency_mhz = 250;
	std::string revision_string = "F";

	// clang-format off
    boost::program_options::options_description options("Generic Options");
    options.add_options()
        ("help,h","produce help message")
        ("encode,e",
            boost::program_options::value<double>(&encode_us),
            "encode blanking time in microseconds")
        ("decode,d",
            boost::program_options::value<std::string>(&decode_value),
            "decode ResetDelay register value")
        ("blanking,b",
            boost::program_options::value<std::string>(&blanking)->default_value("on"), 
            "blanking state when encoding (on/off)")
        ("frequency,f",
            boost::program_options::value<unsigned int>(&frequency_mhz)->default_value(250),
            "module sampling frequency in MHz")
        ("revision,r",
            boost::program_options::value<std::string>(&revision_string)->default_value("F"),
            "module hardware revision (F or H)");
	// clang-format on

	boost::program_options::variables_map vm;

	try {
		store(
			boost::program_options::parse_command_line(
				argc, argv, options),
			vm);

		notify(vm);

	} catch (const std::exception& e) {
		spdlog::error("{}", e.what());
		PrintUsage(argv[0]);
		return 1;
	}

	/*
	 * Help
	 */
	if (vm.count("help")) {
		std::stringstream stream;
		stream << options;

		spdlog::info("{}", stream.str());
		PrintUsage(argv[0]);

		return 0;
	}

	const bool encode = vm.count("encode");
	const bool decode = vm.count("decode");

	if (encode == decode) {
		spdlog::error("Specify exactly one of --encode or --decode.");
		PrintUsage(argv[0]);
		return 1;
	}

	/*
	 * --blanking is an encode-only option.
	 *
	 * Since --blanking has a default value, vm.count("blanking") is true
	 * even when the user did not explicitly specify it. Use
	 * defaulted() to determine whether it was supplied.
	 */
	const bool blanking_was_provided =
		vm.count("blanking") &&
		!vm["blanking"].defaulted();

	if (decode && blanking_was_provided) {
		spdlog::error("--blanking may only be used with --encode.");
		return 1;
	}

	for (auto& c : blanking) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}

	if (blanking != "on" && blanking != "off") {
		spdlog::error(
			"Blanking state must be 'on' or 'off'.");
		return 1;
	}

	const bool blanking_enabled =
		(blanking == "on");

	if (revision_string.size() != 1) {
		spdlog::error(
			"Module revision must be F or H.");
		return 1;
	}

	const char revision =
		static_cast<char>(
			std::toupper(
				static_cast<unsigned char>(
					revision_string[0])));

	if (revision != 'F' && revision != 'H') {
		spdlog::error(
			"Module revision must be F or H.");
		return 1;
	}
	double tick_us = 0.0;

	try {
		tick_us =
			GetResetDelayTickUs(frequency_mhz, revision);

	} catch (const std::exception& e) {
		spdlog::error("{}", e.what());
		return 1;
	}

	/*
	 * ENCODE
	 */
	if (encode) {
		if (!std::isfinite(encode_us) || encode_us < 0.0) {
			spdlog::error(
				"Delay must be greater than or equal to 0 us.");
			return 1;
		}

		const auto ticks = static_cast<uint32_t>(std::llround(encode_us / tick_us));

		if (ticks > RESET_DELAY_MASK) {
			spdlog::error(
				"Requested delay {:.3f} us exceeds maximum "
				"ResetDelay of {:.3f} us for a "
				"{} MHz Rev {} module.",
				encode_us,
				RESET_DELAY_MASK * tick_us,
				frequency_mhz,
				revision);

			return 1;
		}

		/*
		 * Bits [15:0] contain the interval.
		 * Bit 16 controls blanking enable.
		 */
		const uint32_t reset_delay = (blanking_enabled ? RESET_DELAY_ENABLE : 0u) | ticks;

		const double actual_us = static_cast<double>(ticks) * tick_us;
		// clang-format off
        spdlog::info("Module Frequency : {} MHz", frequency_mhz);
        spdlog::info("Module Revision  : {}", revision);
        spdlog::info("ResetDelay Tick  : {:.3f} us ({:.1f} ns)", tick_us, tick_us * 1000.0);
        spdlog::info("");
        spdlog::info("Requested Delay  : {:.3f} us", encode_us);
        spdlog::info("Actual Delay     : {:.3f} us", actual_us);
        spdlog::info("Delay Ticks      : {}", ticks);
        spdlog::info("Blanking         : {}", blanking_enabled ? "Enabled" : "Disabled");
        spdlog::info("");
        spdlog::info("ResetDelay       : {} (0x{:05X})", reset_delay, reset_delay);
		// clang-format on
		return 0;
	}

	/*
	 * DECODE
	 */
	try {
		std::size_t parsed = 0;

		const unsigned long value = std::stoul(decode_value, &parsed, 0);

		if (parsed != decode_value.size()) {
			throw std::invalid_argument("invalid characters");
		}

		if (value > RESET_DELAY_MAX) {
			spdlog::error("ResetDelay value {} exceeds the "
				      "17-bit maximum (0x1FFFF).",
				      decode_value);
			return 1;
		}

		const auto reset_delay = static_cast<uint32_t>(value);

		const bool enabled = (reset_delay & RESET_DELAY_ENABLE) != 0;

		const uint32_t ticks = reset_delay & RESET_DELAY_MASK;

		const double delay_us = static_cast<double>(ticks) * tick_us;
		// clang-format off
        spdlog::info("Module Frequency : {} MHz", frequency_mhz);
        spdlog::info("Module Revision  : {}", revision);
        spdlog::info("ResetDelay Tick  : {:.3f} us ({:.1f} ns)",tick_us,tick_us * 1000.0);
        spdlog::info("");
        spdlog::info("ResetDelay       : {} (0x{:05X})", reset_delay, reset_delay);
        spdlog::info("Blanking         : {}", enabled ? "Enabled" : "Disabled");
        spdlog::info("Delay Ticks      : {}", ticks);
        spdlog::info("Blanking Delay   : {:.3f} us", delay_us);
		// clang-format on
	} catch (const std::exception&) {
		spdlog::error("Unable to parse ResetDelay value '{}'.", decode_value);
		return 1;
	}

	return 0;
}