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
	if (argc != 4 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
		std::cout
			<< "Usage: " << argv[0]
			<< " <input1.bin> <input2.bin> <output.bin>\n\n"
			<< "Merge two time-sorted CoMPASS binary files into one "
			   "time-sorted binary file.\n";
		return (argc == 4) ? 0 : 1;
	}

	try {
		const std::filesystem::path input1_path(argv[1]);
		const std::filesystem::path input2_path(argv[2]);
		const std::filesystem::path output_path(argv[3]);

		if (input1_path == output_path || input2_path == output_path) {
			throw std::runtime_error("Output file must not be either input file");
		}

		CaenInput input1(input1_path);
		CaenInput input2(input2_path);

		if (input1.header() != input2.header()) {
			throw std::runtime_error(
				"The input files have different CoMPASS headers (" + std::to_string(input1.header()) + " versus " + std::to_string(input2.header()) + "). Their event formats are not identical.");
		}

		std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
		if (!output) {
			throw std::runtime_error("Cannot create output file: " + output_path.string());
		}

		// A merged file has one format header, followed by all event records.
		const std::uint16_t header = input1.header();
		output.write(reinterpret_cast<const char*>(&header), sizeof(header));
		if (!output) {
			throw std::runtime_error("Failed to write output header");
		}

		Event event1;
		Event event2;
		bool has1 = input1.read_event(event1);
		bool has2 = input2.read_event(event2);

		std::uint64_t written = 0;

		while (has1 || has2) {
			// On equal timestamps, input 1 is written first.
			if (has1 && (!has2 || event1.timestamp <= event2.timestamp)) {
				write_event(output, event1);
				++written;
				has1 = input1.read_event(event1);
			} else {
				write_event(output, event2);
				++written;
				has2 = input2.read_event(event2);
			}

			if (written % 1'000'000 == 0) {
				std::cout << "Merged " << written << " events...\n";
			}
		}

		output.close();
		if (!output) {
			throw std::runtime_error("Failed to finalize output file");
		}

		std::cout << "\nMerge complete.\n"
			  << "Input 1 events: " << input1.event_count() << '\n'
			  << "Input 2 events: " << input2.event_count() << '\n'
			  << "Total events:   " << written << '\n'
			  << "Output file:    " << output_path << '\n';

		return 0;
	} catch (const std::exception& error) {
		std::cerr << "Error: " << error.what() << '\n';
		return 1;
	}
}
