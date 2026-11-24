
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace {

	// -----------------------------------------------------------------------------
	// Event representation
	// -----------------------------------------------------------------------------

	struct Event {
		std::uint64_t timestamp{};
		std::vector<char> bytes;
	};

	// -----------------------------------------------------------------------------
	// Input file information
	// -----------------------------------------------------------------------------

	struct InputFile {
		std::filesystem::path path;
		std::filesystem::path base_path;
		std::uint64_t rollover_number = 0;
		bool has_header = true;
	};

	// -----------------------------------------------------------------------------
	// Identify and sort input files
	// -----------------------------------------------------------------------------

	std::vector<InputFile> sort_input_files(
		const std::vector<std::string>& filenames) {
		std::vector<InputFile> files;
		files.reserve(filenames.size());

		std::set<std::filesystem::path> unique_paths;

		// Normalize paths and reject duplicate inputs.
		for (const auto& filename : filenames) {
			const auto path =
				std::filesystem::absolute(filename).lexically_normal();

			if (!unique_paths.insert(path).second) {
				throw std::runtime_error(
					"Duplicate input file: " + path.string());
			}

			files.push_back({path,
					 path,
					 0,
					 true});
		}

		// Map original filenames to their positions.
		std::map<std::filesystem::path, std::size_t> original_files;

		for (std::size_t i = 0; i < files.size(); ++i) {
			original_files.emplace(files[i].path, i);
		}

		// Identify rollover files.
		for (auto& file : files) {
			const std::string stem = file.path.stem().string();

			const auto separator = stem.rfind('_');

			if (separator == std::string::npos)
				continue;

			const std::string suffix = stem.substr(separator + 1);

			if (suffix.empty())
				continue;

			std::uint64_t rollover_number = 0;

			const auto [ptr, ec] = std::from_chars(
				suffix.data(),
				suffix.data() + suffix.size(),
				rollover_number);

			if (ec != std::errc{} ||
			    ptr != suffix.data() + suffix.size() ||
			    rollover_number == 0) {
				continue;
			}

			const auto base_path =
				file.path.parent_path() /
				(stem.substr(0, separator) +
				 file.path.extension().string());

			// Only classify as a rollover if its original file
			// is also included in the input list.
			if (original_files.find(base_path) != original_files.end()) {
				file.base_path = base_path;
				file.rollover_number = rollover_number;
				file.has_header = false;
			}
		}

		// Check for duplicate rollover numbers.
		std::set<std::pair<std::filesystem::path, std::uint64_t>>
			rollover_numbers;

		for (const auto& file : files) {
			if (!file.has_header) {
				const auto key =
					std::make_pair(file.base_path, file.rollover_number);

				if (!rollover_numbers.insert(key).second) {
					throw std::runtime_error(
						"Duplicate rollover number " +
						std::to_string(file.rollover_number) +
						" for acquisition " + file.base_path.string());
				}
			}
		}

		// Sort by acquisition name, then numeric rollover number.
		std::sort(
			files.begin(),
			files.end(),
			[](const InputFile& a, const InputFile& b) {
				if (a.base_path != b.base_path) {
					return a.base_path.string() <
					       b.base_path.string();
				}

				return a.rollover_number < b.rollover_number;
			});

		return files;
	}

	// -----------------------------------------------------------------------------
	// CoMPASS binary input reader
	// -----------------------------------------------------------------------------

	class CaenInput {
	public:
		CaenInput(
			const std::filesystem::path& path,
			bool has_header,
			std::uint16_t expected_header = 0)
			: path_(path)
			, input_(path, std::ios::binary) {
			if (!input_) {
				throw std::runtime_error(
					"Cannot open input file: " + path.string());
			}

			if (has_header) {
				input_.read(
					reinterpret_cast<char*>(&header_),
					sizeof(header_));

				if (input_.gcount() !=
				    static_cast<std::streamsize>(sizeof(header_))) {
					throw std::runtime_error(
						"File is too small to contain a CoMPASS header: " +
						path.string());
				}

			} else {
				// Rollover files do not contain a header.
				// Use the header from their original acquisition.
				header_ = expected_header;
			}

			bit0_ = get_bit(header_, 0); // Raw energy
			bit1_ = get_bit(header_, 1); // Calibrated energy
			bit2_ = get_bit(header_, 2); // Short energy
			bit3_ = get_bit(header_, 3); // Waveform
		}

		std::uint16_t header() const {
			return header_;
		}

		const std::filesystem::path& path() const {
			return path_;
		}

		std::uint64_t event_count() const {
			return event_count_;
		}

		// -------------------------------------------------------------------------
		// Read one complete event.
		//
		// Returns false only on clean end-of-file.
		// Throws if an incomplete event is encountered.
		// -------------------------------------------------------------------------

		bool read_event(Event& event) {
			event.bytes.clear();

			std::uint16_t board{};

			input_.read(
				reinterpret_cast<char*>(&board),
				sizeof(board));

			const auto board_bytes = input_.gcount();

			if (board_bytes == 0 && input_.eof()) {
				return false;
			}

			if (board_bytes !=
			    static_cast<std::streamsize>(sizeof(board))) {
				throw truncated_record("board");
			}

			append_bytes(event.bytes, &board, sizeof(board));

			std::uint16_t channel{};
			read_field(channel, event.bytes, "channel");

			read_field(event.timestamp, event.bytes, "timestamp");

			// Each input file must already be timestamp sorted.
			if (have_previous_timestamp_ &&
			    event.timestamp < previous_timestamp_) {
				throw std::runtime_error(
					"Input file is not time-sorted: " +
					path_.string() +
					" (event " +
					std::to_string(event_count_ + 1) +
					", timestamp " +
					std::to_string(event.timestamp) +
					" follows " +
					std::to_string(previous_timestamp_) +
					")");
			}

			previous_timestamp_ = event.timestamp;
			have_previous_timestamp_ = true;

			std::uint16_t energy{};
			std::uint64_t calibrated_energy{};
			std::uint16_t energy_short{};
			std::uint32_t flags{};

			if (bit0_) {
				read_field(energy, event.bytes, "energy");
			}

			if (bit1_) {
				read_field(
					calibrated_energy,
					event.bytes,
					"calibrated energy");
			}

			if (bit2_) {
				read_field(
					energy_short,
					event.bytes,
					"short energy");
			}

			read_field(flags, event.bytes, "flags");

			// These fields are decoded only to advance the input stream.
			(void)channel;
			(void)energy;
			(void)calibrated_energy;
			(void)energy_short;
			(void)flags;

			// ---------------------------------------------------------------------
			// Waveform information
			// ---------------------------------------------------------------------

			if (bit3_) {
				std::uint8_t waveform_code{};
				std::uint32_t num_samples{};

				read_field(
					waveform_code,
					event.bytes,
					"waveform code");

				read_field(
					num_samples,
					event.bytes,
					"waveform sample count");

				(void)waveform_code;

				constexpr std::uint32_t max_samples = 100'000'000;

				if (num_samples > max_samples) {
					throw std::runtime_error(
						"Unreasonable waveform sample count in " +
						path_.string() + ": " +
						std::to_string(num_samples));
				}

				const std::size_t waveform_bytes =
					static_cast<std::size_t>(num_samples) *
					sizeof(std::uint16_t);

				if (waveform_bytes >
				    event.bytes.max_size() - event.bytes.size()) {
					throw std::runtime_error(
						"Waveform size exceeds event buffer capacity in " +
						path_.string());
				}

				const auto old_size = event.bytes.size();

				event.bytes.resize(old_size + waveform_bytes);

				if (waveform_bytes != 0) {
					input_.read(
						event.bytes.data() + old_size,
						static_cast<std::streamsize>(waveform_bytes));

					if (input_.gcount() !=
					    static_cast<std::streamsize>(waveform_bytes)) {
						throw truncated_record("waveform samples");
					}
				}
			}

			++event_count_;

			return true;
		}

	private:
		static bool get_bit(std::uint16_t word, unsigned bit) {
			return ((word >> bit) & 1U) != 0;
		}

		static void append_bytes(
			std::vector<char>& destination,
			const void* data,
			std::size_t size) {
			const auto* first =
				static_cast<const char*>(data);

			destination.insert(
				destination.end(),
				first,
				first + size);
		}

		template<typename T>
		void read_field(
			T& value,
			std::vector<char>& record,
			const char* field) {
			input_.read(
				reinterpret_cast<char*>(&value),
				sizeof(T));

			if (input_.gcount() !=
			    static_cast<std::streamsize>(sizeof(T))) {
				throw truncated_record(field);
			}

			append_bytes(record, &value, sizeof(T));
		}

		std::runtime_error truncated_record(const char* field) const {
			return std::runtime_error(
				"Truncated event (" +
				std::string(field) +
				") in " +
				path_.string() +
				" near event " +
				std::to_string(event_count_ + 1));
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

	// -----------------------------------------------------------------------------
	// Write event without modifying its contents.
	// -----------------------------------------------------------------------------

	void write_event(
		std::ofstream& output,
		const Event& event) {
		output.write(
			event.bytes.data(),
			static_cast<std::streamsize>(event.bytes.size()));

		if (!output) {
			throw std::runtime_error(
				"Failed while writing output file");
		}
	}

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char* argv[]) {
	namespace po = boost::program_options;

	std::vector<std::string> input_files;
	std::string output_file;

	po::options_description options(
		"CoMPASS Binary Merger Options");

	// clang-format off
    options.add_options()
        ("help,h",
         "Display this help message")

        ("input,i",
         po::value<std::vector<std::string>>(&input_files)
             ->multitoken()
             ->required(),
         "Input binary files to merge")

        ("output,o",
         po::value<std::string>(&output_file)->required(),
         "Output binary file");
	// clang-format on

	po::positional_options_description positional;
	positional.add("input", -1);

	// -------------------------------------------------------------------------
	// Parse command-line arguments
	// -------------------------------------------------------------------------

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
			throw std::runtime_error(
				"No input files specified");
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

	// -------------------------------------------------------------------------
	// Merge input files
	// -------------------------------------------------------------------------

	try {
		const std::filesystem::path output_path =
			std::filesystem::absolute(output_file).lexically_normal();

		// ---------------------------------------------------------------------
		// Automatically sort acquisition and rollover files.
		// ---------------------------------------------------------------------

		const auto sorted_files =
			sort_input_files(input_files);

		const std::size_t num_inputs = sorted_files.size();

		spdlog::info("Input files after sorting:");

		for (const auto& file : sorted_files) {
			spdlog::info(
				"  {} (header: {})",
				file.path.string(),
				file.has_header ? "yes" : "no");
		}

		// ---------------------------------------------------------------------
		// Initialize input streams.
		// ---------------------------------------------------------------------

		std::vector<std::unique_ptr<CaenInput>> inputs;
		inputs.reserve(num_inputs);

		std::vector<Event> events(num_inputs);

		// Store the header associated with each original acquisition.
		std::map<std::filesystem::path, std::uint16_t> acquisition_headers;

		std::uint16_t header = 0;
		bool have_common_header = false;

		for (const auto& file : sorted_files) {
			// Prevent overwriting an input file.
			if (file.path == output_path) {
				throw std::runtime_error(
					"Output file must not be an input file: " +
					file.path.string());
			}

			std::uint16_t expected_header = 0;

			if (!file.has_header) {
				// Find the header belonging to this rollover's original file.
				const auto it =
					acquisition_headers.find(file.base_path);

				if (it == acquisition_headers.end()) {
					throw std::runtime_error(
						"Cannot find original acquisition header for rollover: " +
						file.path.string());
				}

				expected_header = it->second;
			}

			auto input = std::make_unique<CaenInput>(
				file.path,
				file.has_header,
				expected_header);

			// Original acquisition files establish their own headers.
			if (file.has_header) {
				const auto file_header = input->header();

				acquisition_headers.emplace(
					file.path,
					file_header);

				// All acquisition groups must have the same event format.
				if (!have_common_header) {
					header = file_header;
					have_common_header = true;

				} else if (file_header != header) {
					throw std::runtime_error(
						"Input files have different CoMPASS headers: " +
						file.path.string());
				}
			}

			spdlog::info(
				"Opened input: {}",
				file.path.string());

			inputs.push_back(std::move(input));
		}

		if (!have_common_header) {
			throw std::runtime_error(
				"No original CoMPASS file with a header was supplied");
		}

		// ---------------------------------------------------------------------
		// Initialize priority queue.
		// ---------------------------------------------------------------------

		struct QueueEntry {
			std::uint64_t timestamp;
			std::size_t input_index;
		};

		// Minimum timestamp has highest priority.
		struct CompareEvents {
			bool operator()(
				const QueueEntry& a,
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

		// ---------------------------------------------------------------------
		// Create output file.
		// ---------------------------------------------------------------------

		std::ofstream output(
			output_path,
			std::ios::binary | std::ios::trunc);

		if (!output) {
			throw std::runtime_error(
				"Cannot create output file: " +
				output_path.string());
		}

		// Write the common CoMPASS header exactly once.
		output.write(
			reinterpret_cast<const char*>(&header),
			sizeof(header));

		if (!output) {
			throw std::runtime_error(
				"Failed to write output header");
		}

		// ---------------------------------------------------------------------
		// Read the first event from each input file.
		// ---------------------------------------------------------------------

		for (std::size_t i = 0; i < num_inputs; ++i) {
			if (inputs[i]->read_event(events[i])) {
				queue.push({events[i].timestamp,
					    i});
			}
		}

		// ---------------------------------------------------------------------
		// Perform k-way timestamp merge.
		// ---------------------------------------------------------------------

		std::uint64_t written = 0;

		while (!queue.empty()) {
			const auto current = queue.top();
			queue.pop();

			const auto idx = current.input_index;

			// Write the earliest event without modifying its bytes.
			write_event(output, events[idx]);

			++written;

			// Read the next event from the same input file.
			if (inputs[idx]->read_event(events[idx])) {
				queue.push({events[idx].timestamp,
					    idx});
			}

			if (written % 1'000'000 == 0) {
				spdlog::info(
					"Merged {} events...",
					written);
			}
		}

		// ---------------------------------------------------------------------
		// Finalize output.
		// ---------------------------------------------------------------------

		output.close();

		if (!output) {
			throw std::runtime_error(
				"Failed to finalize output file");
		}

		// ---------------------------------------------------------------------
		// Print statistics.
		// ---------------------------------------------------------------------

		spdlog::info("Merge complete.");

		std::uint64_t total_events = 0;

		for (std::size_t i = 0; i < num_inputs; ++i) {
			const auto count = inputs[i]->event_count();

			total_events += count;

			spdlog::info(
				"Input {}: {} events",
				sorted_files[i].path.string(),
				count);
		}

		spdlog::info("Total events: {}", written);
		spdlog::info("Output file: {}", output_path.string());

		if (total_events != written) {
			throw std::runtime_error(
				"Event count mismatch after merging");
		}

		return EXIT_SUCCESS;

	} catch (const std::exception& error) {
		spdlog::error("{}", error.what());

		return EXIT_FAILURE;
	}
}
