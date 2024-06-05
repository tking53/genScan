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
		RootFileManager(const std::string& log,const std::string oup,bool enabletrees){
			this->LogName = log;
			this->outputprefix = oup;
			this->outputfilename = this->outputprefix+".root";
			this->OutputFile = new TFile(this->outputfilename.c_str(),"RECREATE");
			this->OutputTreesToFile = enabletrees;
		}

		void FinalizeTrees(){
			if( this->OutputTreesToFile ){
				for( auto& name : this->KnownProcNames ){
					this->OutputTrees[name]->Write(0,2,0);
				}
			}
			this->OutputFile->Close();
		}

		void RegisterProcessor(Processor* proc){
			auto procname = proc->GetProcessorName();

			auto beforesize = this->OutputTrees.size();
			std::set<std::string> beforenames;
			for( const auto& kv : this->OutputTrees ){
				beforenames.insert(kv.first);
			}
			proc->RegisterTree(this->OutputTrees);
			std::set<std::string> newnames;
			for( const auto& kv : this->OutputTrees ){
				if( beforenames.find(kv.first) == beforenames.end() ){
					newnames.insert(kv.first);
				}
			}
			auto aftersize = this->OutputTrees.size();

			if( beforesize != aftersize ){
				for( const auto& name : newnames ){
					this->KnownProcNames.push_back(name);
				}
				if( newnames.size() == 1 ){
					spdlog::get(this->LogName)->info("Registering Processor [{}] to the root file [{}]",this->KnownProcNames.back(),this->outputfilename);
				}else{
					for( const auto& name : newnames ){
						spdlog::get(this->LogName)->info("Registering Processor's [{}] Tree to the root file [{}] from [{}]",name,this->outputfilename,procname);
					}
				}
			}else{
				this->NullProcNames.push_back(procname);
				spdlog::get(this->LogName)->critical("Processor [{}] has no OutputTree(s)",this->NullProcNames.back());
			}
		}

		void Fill(){
			if( this->OutputTreesToFile ){
				for( auto& tree : this->OutputTrees ){
					if( tree.second != nullptr ){
						tree.second->Fill();
					}
				}
			}
		}

	private:
		std::string LogName;
		std::string outputprefix;
		std::string outputfilename;

		bool OutputTreesToFile;

		TFile* OutputFile;
		std::vector<std::string> KnownProcNames;
		std::vector<std::string> NullProcNames;
		std::unordered_map<std::string,TTree*> OutputTrees;
};

#endif
