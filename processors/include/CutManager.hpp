#ifndef __CUT_MANAGER_HPP__
#define __CUT_MANAGER_HPP__

#include <unordered_map>
#include <string>
#include <regex>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/sort/spreadsort/string_sort.hpp>

#include "TCutG.h"

namespace CUTS{
	class CutRegistry{
		public:
			CutRegistry(const std::string&);
			~CutRegistry() = default;
			void AddCut(const std::string&,const std::string&);
			void AddCut(const std::string&,const std::vector<double>&,const std::vector<double>&);
			bool IsWithin(const std::string&,double,double);

		private:
			bool CutIDExists(const std::string&) const;
			std::vector<std::string> CutIDs;
			std::unordered_map<std::string,TCutG*> Cuts;
			std::string LogName;
			std::string RegistryName;
			std::shared_ptr<spdlog::logger> console;
			std::regex SetPointRegex;
			std::regex NumberPattern; 
	};

}

#endif
