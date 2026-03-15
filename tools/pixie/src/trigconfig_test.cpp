///@authors T. King
/********************************************************************/
/* trigconfig_test.cpp                                               */
/*     Decode Pixie TrigConfig bitmasks                              */
/********************************************************************/

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace {

constexpr unsigned int kNumBits = 32;
constexpr const char *kGreen = "\033[32m";
constexpr const char *kReset = "\033[0m";

bool ParseUInt32(const std::string &input, std::uint32_t &output) {
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

std::string HighlightIfOne(const int value, const int width) {
    const std::string cell = fmt::format("{:>{}}", value, width);
    if (value == 1) {
        return fmt::format("{}{}{}", kGreen, cell, kReset);
    }
    return cell;
}

template<typename T>
std::string HighlightIfAny(const T value, const int width, const bool enabled) {
    const std::string cell = fmt::format("{:>{}}", value, width);
    if (enabled) {
        return fmt::format("{}{}{}", kGreen, cell, kReset);
    }
    return cell;
}

void PrintChannelView(const std::array<std::uint32_t, 4> &regs,
                      const unsigned int num_configs) {
    spdlog::info("");
    spdlog::info("Mask Summary");
    if (num_configs == 1) {
        spdlog::info("{:<4} {:>4} {:>12}", "Bit", "Trig", "BitValue");
    } else {
        spdlog::info("{:<4} {:>5} {:>5} {:>5} {:>5} {:>12}",
                     "Bit",
                     "Trig0",
                     "Trig1",
                     "Trig2",
                     "Trig3",
                     "BitValue");
    }

    for (unsigned int ch = 0; ch < kNumBits; ++ch) {
        const int b0 = IsBitSet(regs[0], ch) ? 1 : 0;
        const int b1 = IsBitSet(regs[1], ch) ? 1 : 0;
        const int b2 = IsBitSet(regs[2], ch) ? 1 : 0;
        const int b3 = IsBitSet(regs[3], ch) ? 1 : 0;

        const bool any_enabled =
            num_configs == 1 ? (b0 == 1) : ((b0 | b1 | b2 | b3) != 0);

        const std::string bit_col = HighlightIfAny(ch, 2, any_enabled);
        const std::string bit_val_col = HighlightIfAny((1u << ch), 12, any_enabled);
        if (num_configs == 1) {
            const std::string trig_col = HighlightIfOne(b0, 4);
            spdlog::info("{} {} {}", bit_col, trig_col, bit_val_col);
        } else {
            const std::string trig0_col = HighlightIfOne(b0, 5);
            const std::string trig1_col = HighlightIfOne(b1, 5);
            const std::string trig2_col = HighlightIfOne(b2, 5);
            const std::string trig3_col = HighlightIfOne(b3, 5);
            spdlog::info("{} {} {} {} {} {}",
                         bit_col,
                         trig0_col,
                         trig1_col,
                         trig2_col,
                         trig3_col,
                         bit_val_col);
        }
    }
}

void PrintUsage(const char *prog) {
    spdlog::info("SYNTAX:");
    spdlog::info("  {} <trigconfig>", prog);
    spdlog::info("  {} <trigconfig0> <trigconfig1> <trigconfig2> <trigconfig3>", prog);
    spdlog::info("Numbers can be decimal or hex (e.g. 65535 or 0xFFFF).");
}

void PrintTypeMasks() {
	spdlog::info("\nTrig Config Bit Masks for the UTK Firmware:");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "Type", "Trig0", "Trig1", "Trig2", "Trig3");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "-------", "-------", "-------", "-------", "-------");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "VANDLE", "0", "0", "0", "0");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "Valid", "0", "0", "0", "1");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "Neutron", "0", "0", "1", "0");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "Beta", "0", "0", "1", "1");
    spdlog::info("{:>7} {:^7} {:^7} {:^7} {:^7}", "Gamma", "0", "1", "0", "0");
}
} // namespace

int main(int argc, char *argv[]) {
    spdlog::set_pattern("%v");

    std::vector<std::string> trigconfig_values;

    // clang-format off
    boost::program_options::options_description visible_options("Generic Options");
    visible_options.add_options()
        ("help,h", "produce help message")
        ("types,t", "print TrigConfig type bit assignments");

    boost::program_options::options_description hidden_options("Hidden Options");
    hidden_options.add_options()
        ("values", boost::program_options::value<std::vector<std::string>>(&trigconfig_values)->multitoken(),
            "One TrigConfig value, or four values: TrigConfig0 TrigConfig1 TrigConfig2 TrigConfig3");

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

        const bool show_types = vm.count("types") > 0;

        if (vm.count("help") || trigconfig_values.empty()) {
            if (show_types) {
                PrintTypeMasks();
                return 0;
            }
            std::stringstream stream;
            stream << visible_options;
            spdlog::info("{}", stream.str());
            PrintUsage(argv[0]);
            return 0;
        }

        if (trigconfig_values.size() != 1 && trigconfig_values.size() != 4) {
            throw std::runtime_error(fmt::format(
                    "Invalid number of trigconfig values: expected 1 or 4, received {}",
                    trigconfig_values.size()));
        }

        if (show_types) {
            PrintTypeMasks();
        }
    } catch (const std::exception &e) {
        spdlog::error(e.what());
        PrintUsage(argv[0]);
        return 1;
    }

    std::array<std::uint32_t, 4> regs = {0u, 0u, 0u, 0u};
    if (trigconfig_values.size() == 1) {
        if (!ParseUInt32(trigconfig_values[0], regs[0])) {
            spdlog::error("Failed to parse TrigConfig value '{}'", trigconfig_values[0]);
            return 1;
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            if (!ParseUInt32(trigconfig_values[static_cast<std::size_t>(i)], regs[static_cast<std::size_t>(i)])) {
                spdlog::error("Failed to parse TrigConfig{} value '{}'", i,
                              trigconfig_values[static_cast<std::size_t>(i)]);
                return 1;
            }
        }
    }

    PrintChannelView(regs, trigconfig_values.size() == 1 ? 1u : 4u);

    return 0;
}
