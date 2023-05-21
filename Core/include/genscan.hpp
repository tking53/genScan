#ifndef GENSCAN_HPP
#define GENSCAN_HPP

#include <iostream>
#include <thread>
#include <utility>
#include <map>
#include <string>

#include <sys/stat.h> //For directory manipulation

#include "ConfigXMLparser.hpp"
#include "Unpacker.hpp"

#ifdef USE_SPDLOG
	#include <spdlog/spdlog.h>
	#include <spdlog/cfg/env.h>
	#include <spdlog/fmt/ostr.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/sinks/stdout_color_sinks.h>
	#include <spdlog/async_logger.h>
#endif


class genscan{
	public:
		genscan();
		~genscan() = default;

		void SetCfgFile(std::string*);
		void SetInputFile(std::string*);
		void SetOutputFile(std::string*);
		void SetEvtBuild(bool);

		void DoScan();
	private:
		std::string* cfgFile;
		std::string* inputFile;
		std::string* outputFile;
		bool evtBuild;

		#ifdef USE_SPDLOG
			//std::shared_ptr<spdlog::logger> LogFile;
			std::shared_ptr<spdlog::sinks::basic_file_sink_mt> LogFileSink;
			std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogFileConsole;
		#endif

		Unpacker* unpacker;
		configXMLparser* configParser;
};
#endif
