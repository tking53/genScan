#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>

namespace {

	struct Event {
		std::uint64_t timestamp{};
		std::vector<char> bytes;
	};

	class CaenInput {
	public:
		explicit CaenInput(const std::filesystem::path& path)
			: path_(path)
			, input_(path, std::ios::binary) {
			if (!input_) {
				throw std::runtime_error("Cannot open input file: " + path.string());
			}

			input_.read(reinterpret_cast<char*>(&header_), sizeof(header_));
			if (input_.gcount() != static_cast<std::streamsize>(sizeof(header_))) {
				throw std::runtime_error("File is too small to contain a CoMPASS header: " + path.string());
			}

			bit0_ = get_bit(header_, 0); // raw energy
			bit1_ = get_bit(header_, 1); // calibrated energy
			bit2_ = get_bit(header_, 2); // short energy
			bit3_ = get_bit(header_, 3); // waveform
		}

		std::uint16_t header() const {
			return header_;
		}
		const std::filesystem::path& path() const {
			return path_;
		}

		// Returns false only on clean end-of-file. Throws on a truncated record.
		bool read_event(Event& event) {
			event.bytes.clear();

			std::uint16_t board{};
			input_.read(reinterpret_cast<char*>(&board), sizeof(board));
			const auto board_bytes = input_.gcount();

			if (board_bytes == 0 && input_.eof()) {
				return false;
			}
			if (board_bytes != static_cast<std::streamsize>(sizeof(board))) {
				throw truncated_record("board");
			}
			append_bytes(event.bytes, &board, sizeof(board));

			std::uint16_t channel{};
			read_field(channel, event.bytes, "channel");

			read_field(event.timestamp, event.bytes, "timestamp");

			// The event timestamp is expected to be monotonic within each input.
			if (have_previous_timestamp_ && event.timestamp < previous_timestamp_) {
				throw std::runtime_error(
					"Input file is not time-sorted: " + path_.string() + " (event " + std::to_string(event_count_ + 1) + ", timestamp " + std::to_string(event.timestamp) + " follows " + std::to_string(previous_timestamp_) + ")");
			}
			previous_timestamp_ = event.timestamp;
			have_previous_timestamp_ = true;

			std::uint16_t energy{};
			std::uint64_t calibrated_energy{};
			std::uint16_t energy_short{};
			std::uint32_t flags{};

			if (bit0_)
				read_field(energy, event.bytes, "energy");
			if (bit1_)
				read_field(calibrated_energy, event.bytes, "calibrated energy");
			if (bit2_)
				read_field(energy_short, event.bytes, "short energy");
			read_field(flags, event.bytes, "flags");

			if (bit3_) {
				std::uint8_t waveform_code{};
				std::uint32_t num_samples{};

				read_field(waveform_code, event.bytes, "waveform code");
				read_field(num_samples, event.bytes, "waveform sample count");

				// Protect against corrupt lengths and unreasonable allocations.
				constexpr std::uint32_t max_samples = 100'000'000;
				if (num_samples > max_samples) {
					throw std::runtime_error(
						"Unreasonable waveform sample count in " + path_.string() + ": " + std::to_string(num_samples));
				}

				const std::size_t waveform_bytes =
					static_cast<std::size_t>(num_samples) * sizeof(std::uint16_t);

				const auto old_size = event.bytes.size();
				event.bytes.resize(old_size + waveform_bytes);
				if (waveform_bytes != 0) {
					input_.read(event.bytes.data() + old_size,
						    static_cast<std::streamsize>(waveform_bytes));
					if (input_.gcount() != static_cast<std::streamsize>(waveform_bytes)) {
						throw truncated_record("waveform samples");
					}
				}
			}

			++event_count_;
			return true;
		}

		std::uint64_t event_count() const {
			return event_count_;
		}

	private:
		static bool get_bit(std::uint16_t word, unsigned bit) {
			return ((word >> bit) & 1U) != 0;
		}

		static void append_bytes(std::vector<char>& destination,
					 const void* data, std::size_t size) {
			const auto* first = static_cast<const char*>(data);
			destination.insert(destination.end(), first, first + size);
		}

		template<typename T>
		void read_field(T& value, std::vector<char>& record, const char* field) {
			input_.read(reinterpret_cast<char*>(&value), sizeof(T));
			if (input_.gcount() != static_cast<std::streamsize>(sizeof(T))) {
				throw truncated_record(field);
			}
			append_bytes(record, &value, sizeof(T));
		}

		std::runtime_error truncated_record(const char* field) const {
			return std::runtime_error(
				"Truncated event (" + std::string(field) + ") in " + path_.string() + " near event " + std::to_string(event_count_ + 1));
		}

		std::filesystem::path path_;
		std::ifstream input_;
		std::uint16_t header_{};
		bool bit0_{};
		bool bit1_{};
		bool bit2_{};
		bool bit3_{};
		bool have_previous_timestamp_{};
		std::uint64_t previous_timestamp_{};
		std::uint64_t event_count_{};
	};

	void write_event(std::ofstream& output, const Event& event) {
		output.write(event.bytes.data(),
			     static_cast<std::streamsize>(event.bytes.size()));
		if (!output) {
			throw std::runtime_error("Failed while writing output file");
		}
	}

} // namespace

int main(int argc, char* argv[]) {
	namespace po = boost::program_options;

	std::vector<std::string> input_files;
	std::string output_file;

	po::options_description options("CoMPASS Binary Merger Options");

	// clang-format off
	options.add_options()("help,h","Display this help message")
			     ("input,i",po::value<std::vector<std::string>>(&input_files)->multitoken()->required(),"Input binary files to merge")
			     ("output,o",po::value<std::string>(&output_file)->required(),"Output binary file");
	// clang-format on

	po::positional_options_description positional;
	positional.add("input", -1);

	try {
		po::variables_map vm;

		po::store(
			po::command_line_parser(argc, argv)
				.options(options)
				.positional(positional)
				.run(),
			vm);

		if (vm.count("help")) {
			std::cout << options << '\n';
			return EXIT_SUCCESS;
		}

		po::notify(vm);

		if (input_files.empty()) {
			throw std::runtime_error("No input files specified");
		}

		if (input_files.size() < 2) {
			throw std::runtime_error(
				"At least two input files are required");
		}

	} catch (const po::error& e) {
		spdlog::error("Command line error: {}", e.what());
		std::cerr << options << '\n';
		return EXIT_FAILURE;

	} catch (const std::exception& e) {
		spdlog::error("{}", e.what());
		return EXIT_FAILURE;
	}

	try {
		const std::filesystem::path output_path(output_file);

		const std::size_t num_inputs = input_files.size();

		// Continue with the existing k-way merge implementation...

		// Each input file maintains its own event buffer.
		std::vector<std::unique_ptr<CaenInput>> inputs;
		std::vector<Event> events(num_inputs);

		inputs.reserve(num_inputs);

		std::uint16_t header = 0;

		// Priority queue entry.
		struct QueueEntry {
			std::uint64_t timestamp;
			std::size_t input_index;
		};

		// Minimum timestamp has highest priority.
		struct CompareEvents {
			bool operator()(const QueueEntry& a,
					const QueueEntry& b) const {
				if (a.timestamp != b.timestamp) {
					return a.timestamp > b.timestamp;
				}

				return a.input_index > b.input_index;
			}
		};

		std::priority_queue<
			QueueEntry,
			std::vector<QueueEntry>,
			CompareEvents>
			queue;

		// Open all input files and verify their headers.
		for (std::size_t i = 0; i < num_inputs; ++i) {
			const std::filesystem::path input_path(input_files[i]);

			if (std::filesystem::absolute(input_path).lexically_normal() == std::filesystem::absolute(output_path).lexically_normal()) {
				throw std::runtime_error(
					"Output file must not be an input file: " + input_path.string());
			}

			auto input = std::make_unique<CaenInput>(input_path);

			if (i == 0) {
				header = input->header();
			} else if (input->header() != header) {
				throw std::runtime_error(
					"Input files have different CoMPASS headers: " + input_path.string());
			}

			spdlog::info("Opened input: {}", input_path.string());

			inputs.push_back(std::move(input));
		}

		// Create output file.
		std::ofstream output(
			output_path,
			std::ios::binary | std::ios::trunc);

		if (!output) {
			throw std::runtime_error(
				"Cannot create output file: " + output_path.string());
		}

		// Write the common CoMPASS header once.
		output.write(
			reinterpret_cast<const char*>(&header),
			sizeof(header));

		if (!output) {
			throw std::runtime_error("Failed to write output header");
		}

		// Read the first event from each input file.
		for (std::size_t i = 0; i < num_inputs; ++i) {
			if (inputs[i]->read_event(events[i])) {
				queue.push({events[i].timestamp,
					    i});
			}
		}

		std::uint64_t written = 0;

		// Perform the k-way merge.
		while (!queue.empty()) {
			const auto current = queue.top();
			queue.pop();

			const auto idx = current.input_index;

			// Write the earliest event.
			// std::uint16_t raw_energy = 0;

			// if (events[idx].bytes.size() >= 14) {
			// 	std::memcpy(
			// 		&raw_energy,
			// 		events[idx].bytes.data() + 12,
			// 		sizeof(raw_energy));
			// }

			// spdlog::info(
			// 	"Event {}: timestamp={}, energy={}, event size={}",
			// 	written,
			// 	events[idx].timestamp,
			// 	raw_energy,
			// 	events[idx].bytes.size());
			write_event(output, events[idx]);
			++written;

			// Read the next event from the same input file.
			if (inputs[idx]->read_event(events[idx])) {
				queue.push({events[idx].timestamp,
					    idx});
			}

			if (written % 1'000'000 == 0) {
				spdlog::info("Merged {} events...", written);
			}
		}

		output.close();

		if (!output) {
			throw std::runtime_error(
				"Failed to finalize output file");
		}

		// Print statistics.
		spdlog::info("Merge complete.");

		for (std::size_t i = 0; i < num_inputs; ++i) {
			spdlog::info(
				"Input {}: {} events",
				i + 1,
				inputs[i]->event_count());
		}

		spdlog::info("Total events: {}", written);
		spdlog::info("Output file: {}", output_path.string());

		return 0;

	} catch (const std::exception& error) {
		spdlog::error("{}", error.what());
		return 1;
	}
}
