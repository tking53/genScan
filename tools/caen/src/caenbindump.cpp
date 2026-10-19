#include <TNamed.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <map>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/circular_buffer.hpp>

struct CaenHelper {
	std::string digitizer;
	std::vector<std::string> files;
	int curr_file_idx;
	boost::circular_buffer<uint64_t> ts;
	uint64_t earliest_ts;
	uint64_t latest_ts;
	unsigned long long evt_idx;
	std::ifstream input;
	uint16_t header;
	bool bit0; // raw energy present
	bool bit1; // calibrated energy present
	bool bit2; // energy short present
	bool bit3; // trace present

	int decode_header() {
		if (!this->input.read(reinterpret_cast<char*>(&(this->header)), sizeof(uint16_t))) {
			return -1;
		}
		this->bit0 = this->get_bit(this->header, 0);
		this->bit1 = this->get_bit(this->header, 1);
		this->bit2 = this->get_bit(this->header, 2);
		this->bit3 = this->get_bit(this->header, 3);
		spdlog::info("File: {} header: {} -> {} {} {} {}",
			     this->files[this->curr_file_idx],
			     this->header, this->bit0, this->bit1, this->bit2, this->bit3);
		return 0;
	}

	bool get_bit(uint16_t word, uint16_t bit) {
		return (word >> bit) & 1;
	}

	int cycle_file() {
		if (++this->curr_file_idx == 0) {
			this->input.open(this->files[this->curr_file_idx]);
			return 0;
		} else {
			if (this->curr_file_idx < this->files.size()) {
				spdlog::info("swapping file");
				this->input.close();
				this->input.open(this->files[this->curr_file_idx]);
				return 0;
			} else {
				return -1;
			}
		}
	}

	int decode_helper() {
		uint16_t board;
		uint16_t channel;
		uint64_t timestamp;
		uint16_t energy;
		uint64_t cal_energy;
		uint16_t energy_short;
		uint32_t flags;
		uint8_t waveform_code;
		uint32_t num_samples;
		std::vector<uint16_t> waveform;

		if (!input.read(reinterpret_cast<char*>(&board), sizeof(uint16_t))) {
			if (!this->cycle_file()) {
				return this->decode_helper();
			} else {
				return -1;
			}
		}
		this->input.read(reinterpret_cast<char*>(&channel), sizeof(uint16_t));
		this->input.read(reinterpret_cast<char*>(&timestamp), sizeof(uint64_t));
		if (timestamp < this->ts.back()) {
			spdlog::critical("{} : {} : Timestamp out of order Board: {} Channel: Timestamp: {} Flag: {} < {}",
					 this->files[this->curr_file_idx],
					 this->evt_idx, board, channel, timestamp, flags, this->ts.back());
		}
		this->ts.push_back(timestamp);
		if (timestamp < this->earliest_ts) {
			this->earliest_ts = timestamp;
		}
		if (timestamp > this->latest_ts) {
			this->latest_ts = timestamp;
		}
		if (bit0) {
			input.read(reinterpret_cast<char*>(&energy), sizeof(uint16_t));
		}
		if (bit1) {
			input.read(reinterpret_cast<char*>(&cal_energy), sizeof(uint64_t));
		}
		if (bit2) {
			input.read(reinterpret_cast<char*>(&energy_short), sizeof(uint16_t));
		}
		input.read(reinterpret_cast<char*>(&flags), sizeof(uint32_t));
		if (bit3) {
			input.read(reinterpret_cast<char*>(&waveform_code), sizeof(uint8_t));
			input.read(reinterpret_cast<char*>(&num_samples), sizeof(uint32_t));
			// spdlog::info("{} : {}",waveform_code,num_samples);
			waveform = std::vector<uint16_t>(num_samples, 0);
			input.read(reinterpret_cast<char*>(&(waveform.data()[0])), num_samples * sizeof(uint16_t));
			// input.read(reinterpret_cast<char*>(&waveform_code), sizeof(uint8_t));
		}

		// flag decoding for caen
		//
		// we need to handle each of these cases when we insert them into
		// the PhysicsData class. Not entirely sure how to handle the time-stamp
		// fuckery, but we probably should throw a warning and bail completely until
		// someone understands how to process it
		//
		// To match pixie definition of saturation, we use (0x400|0x80)
		// The 0x80 is the saturation of the "filters" which pixie doesn't distinguish
		// against, but 0x400 seems to be the matching definition of pixie
		//
		// we need to remove the (0x8|0x80000|0x100000|0x200000) events
		// and not include them when decoding into a PhysicsData object
		//
		// the events with 0x4000 are just in the ps time regime, but otherwise no special
		// decoding/processing is required
		//
		// I believe we can also ignore the 0x1000 events since we're doing the correlation
		//
		//      0x1 dead time event occured before this event
		//      0x2 time-stamp roll over
		//      0x4 time-stamp reset from external
		//      0x8 fake event
		//     0x10 memory full occured before this event
		//     0x20 trigger lost occured before this event
		//     0x40 N triggers have been lost [found through certain bits of a register]
		//     0x80 event saturating in the gate
		//    0x100 1024 triggers have been counted
		//    0x400 input is saturating
		//    0x800 N triggers have been counted [found through certain bits of a register]
		//   0x1000 event not matched in time correlation
		//   0x4000 event with fine timestamp
		//   0x8000 pileup event
		//  0x80000 identifies fake event reporting a PLL lock loss
		// 0x100000 identifies fake event reporting over-temperature condition
		// 0x200000 identifies fake event reporting and ADC shutdown

		// if ((flags & 0x2) != 0) {
		spdlog::info("File: {} Board: {} Channel: {} Energy: {} EnergyShort: {} Timestamp: {} Flag: {}",
			     this->files[this->curr_file_idx],
			     board, channel, energy, energy_short, timestamp, flags);
		// }

		// let's print things
		// if( channel == 8 and (evt_idx >= 2607940 and evt_idx <= 2607948) ){
		// spdlog::info("Board: {} Channel: {} Energy: {} EnergyShort: {} Timestamp: {} Flag: {}",
		// 	     board, channel, energy, energy_short, timestamp, flags);
		// }

		return 0;
	}

	int decode_next_hit() {
		if (this->input.eof() or not this->input.good()) {
			if (not this->cycle_file()) {
				this->decode_helper();
				return 0;
			} else {
				return -1;
			}
		} else {
			return this->decode_helper();
		}
		++this->evt_idx;
		return 0;
	}
};

int main(int argc, char* argv[]) {
	std::string DirName;
	// std::string inputfile = "/Users/truland/Downloads/run_1/RAW/DataR_CH1@V1730_80_run_1.BIN";
	// std::ifstream input(inputfile);

	// clang-format off
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("directory,d", boost::program_options::value<std::string>(&DirName),
		 		"directory in which the files exist to build out the full translator")
		("single-file", "single file output")
		("time-sorted", "time sorted output of data")
		("help,h", "produce this message");
	// clang-format on

	boost::program_options::positional_options_description p;
	p.add("directory", -1);

	bool is_single_file = false;
	bool is_time_sorted = false;
	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}
		is_single_file = vm.count("single-file");
		is_time_sorted = vm.count("time-sorted");

		if (is_time_sorted) {
			is_single_file = true;
		}
	} catch (std::exception& e) {
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}

	std::filesystem::path data_dir(DirName);
	if (not std::filesystem::is_directory(data_dir)) {
		spdlog::error("directory: {} is not an actual directory", DirName);
		exit(EXIT_FAILURE);
	}

	if (not is_time_sorted and not is_single_file) {
		std::regex pattern("DataR_CH(\\d+)@(V|DT)(\\d+_?\\d+)(?:S)_(.*)");
		std::map<int, CaenHelper> info;
		for (const auto& entry : std::filesystem::directory_iterator(data_dir)) {
			if (std::filesystem::is_regular_file(entry)) {
				// pattern is DataR_CH(\d+).*
				spdlog::info("Found file: {}", entry);
				std::smatch match;
				auto f = entry.path().stem().string();
				if (std::regex_match(f, match, pattern)) {
					auto chan_num = std::stoi(match[1]);
					if (info.find(chan_num) == info.end()) {
						// match[4] is the run_1
						info[chan_num] = {
							.digitizer = std::string(match[2]) + std::string(match[3]),
							.files = {entry.path().string()},
							.curr_file_idx = -1,
							.ts = boost::circular_buffer<uint64_t>(10),
							.earliest_ts = std::numeric_limits<uint64_t>::max(),
							.latest_ts = std::numeric_limits<uint64_t>::min(),
							.evt_idx = 0,
						};
					} else {
						info[chan_num].files.push_back(entry.path().string());
					}
				}
			}
		}

		std::map<int, bool> finished_read;
		std::map<int, int> counts;
		for (auto& kv : info) {
			spdlog::info("{} : {}", kv.first, kv.second.digitizer);
			// need to make sure this occurs correctly
			std::sort(kv.second.files.begin(), kv.second.files.end());
			if (not kv.second.cycle_file()) {
				spdlog::info("opened file: {}", kv.second.files[0]);
				kv.second.decode_header();
				finished_read[kv.first] = false;
			} else {
				finished_read[kv.first] = true;
			}
			counts[kv.first] = 0;
			for (const auto& f : kv.second.files) {
				spdlog::info("---> {}", f);
			}
		}

		bool some_to_read = true;
		while (some_to_read) {
			for (auto& kv : info) {
				if (not finished_read[kv.first]) {
					auto retval = kv.second.decode_next_hit();
					if (retval != 0) {
						finished_read[kv.first] = true;
					} else {
						++counts[kv.first];
					}
				}
			}
			bool check = true;
			for (const auto& kv : finished_read) {
				check &= kv.second;
			}
			some_to_read = !check;
		}

		for (const auto& kv : info) {
			spdlog::info("{} : \n {} {} {} {} {} \n {} {} {} {} {} -> \n {} : {} {}",
				     kv.first,
				     kv.second.ts[0], kv.second.ts[1], kv.second.ts[2], kv.second.ts[3], kv.second.ts[4],
				     kv.second.ts[5], kv.second.ts[6], kv.second.ts[7], kv.second.ts[8], kv.second.ts[9],
				     counts[kv.first], kv.second.earliest_ts, kv.second.latest_ts);
		}
	} else {
		CaenHelper parser;
		std::regex pattern = is_time_sorted ? std::regex("SDataR_(.*)") : std::regex("DataR_(.*)");
		for (const auto& entry : std::filesystem::directory_iterator(data_dir)) {
			if (std::filesystem::is_regular_file(entry)) {
				// pattern is DataR_CH(\d+).*
				spdlog::info("Found file: {}", entry);
				std::smatch match;
				auto f = entry.path().stem().string();
				if (std::regex_match(f, match, pattern)) {
					parser.files.push_back(entry.path().string());
				}
			}
		}
		parser.curr_file_idx = -1;
		parser.ts = boost::circular_buffer<uint64_t>(10);
		parser.earliest_ts = std::numeric_limits<uint64_t>::max();
		parser.latest_ts = std::numeric_limits<uint64_t>::min();
		parser.evt_idx = 0;

		std::sort(parser.files.begin(), parser.files.end());
		bool finished_read = false;
		if (not parser.cycle_file()) {
			spdlog::info("opened file: {}", parser.files[0]);
			parser.decode_header();
			finished_read = false;
		} else {
			finished_read = true;
		}

		bool some_to_read = true;
		int counts = 0;
		while (some_to_read) {
			if (not finished_read) {
				auto retval = parser.decode_next_hit();
				if (retval != 0) {
					finished_read = true;
				} else {
					++counts;
				}
			}
			bool check = true & finished_read;
			some_to_read = !check;
		}

		spdlog::info("\n {} {} {} {} {} \n {} {} {} {} {} -> \n {} : {} {}",
			     parser.ts[0], parser.ts[1], parser.ts[2], parser.ts[3], parser.ts[4],
			     parser.ts[5], parser.ts[6], parser.ts[7], parser.ts[8], parser.ts[9],
			     counts, parser.earliest_ts, parser.latest_ts);
	}

	return 0;
}
