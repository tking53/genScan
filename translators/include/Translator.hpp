#ifndef __TRANSLATOR_HPP__
#define __TRANSLATOR_HPP__

#include <memory>
#include <string>
#include <iostream>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/container/devector.hpp>

#include "PhysicsData.hpp"


class Translator{
	public:
		Translator(const std::string&,const std::string&);
		virtual ~Translator() = default;
		virtual bool AddFile(const std::string&);
		[[noreturn]] virtual void Parse([[maybe_unused]] boost::container::devector<PhysicsData>&);
		virtual void FinalizeFiles();
		virtual bool OpenNextFile();
	protected:
		std::string LogName;
		std::string TranslatorName;

		std::vector<std::string> InputFiles;
		std::vector<int> FileSizes;
		std::ifstream CurrentFile;
		size_t NumTotalFiles;
		size_t NumFilesRemaining;
		size_t CurrentFileIndex;
		bool FinishedCurrentFile;

		std::shared_ptr<spdlog::logger> console;
};

#endif
