#ifndef __ROOT_FILE_MANAGER_HPP__
#define __ROOT_FILE_MANAGER_HPP__

#include <string>
#include <unordered_map>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "TFile.h"
#include "TTree.h"

#include "Processor.hpp"

class RootFileManager{
	public:
		RootFileManager(const std::string& log,const std::string oup){
			this->LogName = log;
			this->outputprefix = oup;
			this->outputfilename = this->outputprefix+".root";
			this->OutputFile = new TFile(this->outputfilename.c_str(),"RECREATE");
		}

		void FinalizeTrees(){
			for( auto& name : this->KnownProcNames ){
				this->OutputTrees[name]->Write(0,2,0);
			}
			this->OutputFile->Close();
		}

		void RegisterProcessor(Processor* proc){
			auto procname = proc->GetProcessorName();
			auto proctree = proc->RegisterTree();
			if( proctree != nullptr ){
				this->KnownProcNames.push_back(procname);
				this->OutputTrees[this->KnownProcNames.back()] = proctree;
				spdlog::get(this->LogName)->info("Registering Processor [{}] to the root file [{}]",this->KnownProcNames.back(),this->outputfilename);
			}else{
				this->NullProcNames.push_back(procname);
				spdlog::get(this->LogName)->critical("Processor [{}] has null OutputTree",this->NullProcNames.back());
			}
		}

	private:
		std::string LogName;
		std::string outputprefix;
		std::string outputfilename;

		TFile* OutputFile;
		std::vector<std::string> KnownProcNames;
		std::vector<std::string> NullProcNames;
		std::unordered_map<std::string,TTree*> OutputTrees;
};

#endif
