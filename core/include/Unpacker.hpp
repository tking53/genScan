#ifndef UNPACKER_HPP
#define UNPACKER_HPP

#include <string>

#include "GeneralStats.hpp"

#ifdef USE_SPDLOG
	#include <spdlog/spdlog.h>
	#include <spdlog/cfg/env.h>
	#include <spdlog/fmt/ostr.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/sinks/stdout_color_sinks.h>
#endif


class Unpacker{
	public:
		Unpacker();
		virtual ~Unpacker();
		virtual void SetCurrFile(std::string*);
		virtual std::string* GetCurrFile();
	protected:
		std::string* currfile;
		std::string* unpackername;

		#ifdef USE_SPDLOG 
			std::shared_ptr<spdlog::logger> LogFile;
		#endif

		FileStats* f_stats;
		CoincidenceStats* c_stats;
};

#endif 
