#ifndef __ROOT_FILE_MANAGER_HPP__
#define __ROOT_FILE_MANAGER_HPP__

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <regex>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "TFile.h"
#include "TTree.h"
#include "TNamed.h"

#include "Processor.hpp"

class RootFileManager {
public:
	RootFileManager(const std::string& log, const std::string oup, bool enabletrees) {
		this->LogName = log;
		this->console = spdlog::get(this->LogName)->clone("RootFileManager");
		this->outputprefix = oup;
		this->outputfilename = this->outputprefix + ".root";
		this->OutputTreesToFile = enabletrees;
		this->filltime = 0;

		// we need to navigate the current directory of outputprefix, searching for
		// outputprefix_{1,}\d{1,}.root
		// and throw a fault unless we are ordered to delete them
		// otherwise root will complain when we swap every 2GB
		// and we are also only allowed up to 10 underscores

		std::filesystem::path input_prefix(oup);
		std::filesystem::path dir;
		std::string prefix;

		if (input_prefix.has_parent_path()) {
			dir = input_prefix.parent_path();
			prefix = input_prefix.filename().string();
		} else {
			dir = ".";
			prefix = oup;
		}

		auto regex_escape = [](const std::string& s) {
			static const std::regex re(R"([.^$|()\\[\]{}*+?])");
			return std::regex_replace(s, re, R"(\$&)");
		};

		if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
			this->console->error("the directory {} does not exist or is not a directory", dir.string());
			throw std::runtime_error("the directory " + dir.string() + " does not exist or is not a directory");
		}

		// vestigial remnants of trying to clean up root output,
		// not going to do this and just make the GB size huge and hope the problem never crops up again
		// these are the currently established limits on filesystems for a single file
		// since we almost always deal with ZFS/NTFS/ext4 we should be fine
		// ext4 with standard 4kB block size is the 16Tib limit
		// so a 500 GB file should always fit in anything we use now adays
		//
		// FAT32 - 4Gib
		// NTFS - 16Eib
		// ext2/3 - 16Gib - 2Tib (depends from block size)
		// ext4 - 16Gib - 16Tib
		// XFS - 9Eib
		// ZFS - 16Eib

		// std::vector<std::filesystem::path> matches;
		// std::regex pattern("^" + regex_escape(prefix) + "_+\\d+\\.root$");
		// for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		//	if (!entry.is_regular_file())
		//		continue;

		//	const std::string name = entry.path().filename().string();
		//	if (std::regex_match(name, pattern)) {
		//		matches.push_back(entry.path());
		//	}
		//}

		// if( matches.size() != 0 ){
		//	this->console->critical("Found files in {} matching root file rolling pattern",dir.c_str());
		//	//if( clean_outdir ){
		//		for( const auto& p : matches ){
		//			this->console->info("{}",p.c_str());
		//		}
		//	//}
		// }
		this->OutputFile = new TFile(this->outputfilename.c_str(), "RECREATE");
	}

	~RootFileManager() {
		this->console->info("Time spent filling trees : {:.3f}s", this->filltime / 1000.0);
	}

	void WriteTNamed(const std::string& name, const std::string& value) {
		TNamed curr(name.c_str(), value.c_str());
		curr.Write(0, 2, 0);
	}

	void FinalizeTrees() {
		if (this->OutputTreesToFile) {
			for (auto& name : this->KnownProcNames) {
				this->OutputFile = this->OutputTrees[name]->GetCurrentFile();
				this->OutputTrees[name]->Write(0, 2, 0);
			}
		}
		this->OutputFile->Close();
	}

	void RegisterProcessor(Processor* proc) {
		auto procname = proc->GetProcessorName();

		auto beforesize = this->OutputTrees.size();
		std::set<std::string> beforenames;
		for (const auto& kv : this->OutputTrees) {
			beforenames.insert(kv.first);
		}
		proc->RegisterTree(this->OutputTrees);
		std::set<std::string> newnames;
		for (const auto& kv : this->OutputTrees) {
			if (beforenames.find(kv.first) == beforenames.end()) {
				newnames.insert(kv.first);
			}
		}
		auto aftersize = this->OutputTrees.size();

		if (beforesize != aftersize) {
			for (const auto& name : newnames) {
				this->KnownProcNames.push_back(name);
				// set default tree rollover to 500 GB, if you need more than this something
				// is horribly wrong with what you're doing and you should reconsider your life
				// choices
				this->OutputTrees[name]->SetMaxTreeSize(500000000000LL);
				// this->OutputTrees[name]->SetMaxTreeSize(100000000LL);
			}
			if (newnames.size() == 1) {
				this->console->info("Registering Processor [{}] to the root file [{}]", this->KnownProcNames.back(), this->outputfilename);
			} else {
				for (const auto& name : newnames) {
					this->console->info("Registering Processor's [{}] Tree to the root file [{}] from [{}]", name, this->outputfilename, procname);
				}
			}
		} else {
			this->NullProcNames.push_back(procname);
			this->console->critical("Processor [{}] has no OutputTree(s)", this->NullProcNames.back());
		}
	}

	void Fill() {
		this->start_time = std::chrono::high_resolution_clock::now();
		if (this->OutputTreesToFile) {
			for (auto& tree : this->OutputTrees) {
				if (tree.second != nullptr) {
					tree.second->Fill();
				}
			}
		}
		this->stop_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> dur = this->stop_time - this->start_time;
		this->filltime += dur.count();
	}

private:
	std::string LogName;
	std::shared_ptr<spdlog::logger> console;
	std::string outputprefix;
	std::string outputfilename;

	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
	std::chrono::time_point<std::chrono::high_resolution_clock> stop_time;
	double filltime;

	bool OutputTreesToFile;

	TFile* OutputFile;
	std::vector<std::string> KnownProcNames;
	std::vector<std::string> NullProcNames;
	std::unordered_map<std::string, TTree*> OutputTrees;
};

#endif
