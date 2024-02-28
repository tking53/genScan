#include <fstream>
#include <stdexcept>
#include <string>

#include "CutManager.hpp"

namespace CUTS{
	CutRegistry::CutRegistry(const std::string& log){
		this->LogName = log;
		this->RegistryName = "CutRegistry";
		this->console = spdlog::get(this->LogName)->clone(this->RegistryName);
		this->console->info("Created CutRegistry [{}]",this->RegistryName);
		this->SetPointRegex = std::regex("SetPoint\\(([^)]+)\\)");
		this->NumberPattern = std::regex("-?\\d+(\\.\\d+)?");
	}

	void CutRegistry::AddCut(const std::string& cutid,const std::string& filename){
		if( this->CutIDExists(cutid) ){
			this->console->error("CutRegistry::AddCut(string,string) : Unable to add cut : {} as it already exists",cutid);
			throw std::runtime_error("CutRegistry::AddCut(string,string) : Unable to add cut "+cutid+" as it already exists");
		}
		std::ifstream input(filename);
		std::string line;
		std::vector<std::tuple<int,double,double>> vals;
		while( input.good() ){
			std::getline(input,line);
			std::sregex_iterator it(line.begin(), line.end(),this->SetPointRegex);
			std::sregex_iterator end;
			while (it != end) {
				std::smatch match = *it;
				std::string match_str = match[1].str(); // Extract the numbers within the parentheses

				// Use another regex to split the numbers

				// Iterator for numbers within parentheses
				std::sregex_iterator numbers_it(match_str.begin(), match_str.end(),this->NumberPattern);
				std::sregex_iterator numbers_end;

				int idx = std::stoi((*numbers_it++).str());
				double xval = std::stod((*numbers_it++).str());
				double yval = std::stod((*numbers_it++).str());

				// Create a tuple
				vals.push_back(std::make_tuple(idx,xval,yval));
				this->console->info("While Parsing : {}, found this cutpoint {} : {} , {}",filename,idx,xval,yval);

				++it;
			}
		}
		if( vals.size() > 0 ){
			this->CutIDs.push_back(cutid);
			this->Cuts[cutid] = new TCutG(cutid.c_str(),vals.size());
			for( const auto& v : vals ){
				auto idx = std::get<0>(v);
				auto xval = std::get<1>(v);
				auto yval = std::get<2>(v);
				this->Cuts[cutid]->SetPoint(idx,xval,yval);
				this->console->info("Registering point [{},{},{}] to cut [{}]",idx,xval,yval,cutid);
			}
		}else{
			this->console->error("CutRegistry::AddCut(string,string) : Unable to add cut listed in file : {}, because no points were parsed",filename);
			throw std::runtime_error("CutRegistry::AddCut(string,string) : Unable to add cut listed in file : "+filename+", because no points were parsed");
		}
	}

	void CutRegistry::AddCut(const std::string& cutid,const std::vector<double>& xvals,const std::vector<double>& yvals){
		if( this->CutIDExists(cutid) ){
			this->console->error("CutRegistry::AddCut(string,vector,vector) : Unable to add cut : {} as it already exists",cutid);
			throw std::runtime_error("CutRegistry::AddCut(string,vector,vector) : Unable to add cut "+cutid+" as it already exists");
		}
		if( xvals.size() != yvals.size() ){
			this->console->error("CutRegistry::AddCut(string,vector,vector) : Unable to add cut {}, because it has mismatched size between x and y of sizes {} and {} respectively",cutid,xvals.size(),yvals.size());
			throw std::runtime_error("CutRegistry::AddCut(string,vector,vector) : Unable to add cut "+cutid+", because it has mismatched size between x and y of sizes "+std::to_string(xvals.size())+" and "+std::to_string(yvals.size())+" respectively");
		}else{
			this->CutIDs.push_back(cutid);
			this->Cuts[cutid] = new TCutG(cutid.c_str(),xvals.size());
			for( size_t ii = 0; ii < xvals.size(); ++ii ){
				auto xval = xvals.at(ii);
				auto yval = yvals.at(ii);
				this->Cuts[cutid]->SetPoint(ii,xval,yval);
				this->console->info("Registering point [{},{},{}] to cut [{}]",ii,xval,yval,cutid);
			}
		}
	}

	bool CutRegistry::IsWithin(const std::string& cutid,double xtest,double ytest){
		if( this->CutIDExists(cutid) ){
			return this->Cuts[cutid]->IsInside(xtest,ytest);
		}else{
			this->console->error("CutRegistry::IsWithin() : Cut : {} does not exist",cutid);
			throw std::runtime_error("CutRegistry::IsWithin() : Cut "+cutid+" does not exist");
		}
	}

	bool CutRegistry::CutIDExists(const std::string& cutid) const{
		return (this->Cuts.find(cutid) != this->Cuts.end());
	}
}
