#ifndef __DATA_PARSER_HPP__
#define __DATA_PARSER_HPP__

#include <memory>
#include <string>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class DataParser{
	public:
		enum DataFileType{
			Unknown,
			CAEN_ROOT,
			CAEN_BIN,
			LDF,
			PLD,
			EVT_PRESORT,
			EVT_BUILT
		};
		DataParser(DataFileType,const std::string&);
		~DataParser() = default;
		void SetInputFiles(std::vector<std::string>&);
		
		void Parse(boost::container::devector<PhysicsData>&);
	private:
		DataFileType DataType;
		std::shared_ptr<spdlog::logger> console;
		std::string LogName;
		std::string ParserName;

		std::unique_ptr<Translator> DataTranslator;
};

#endif
