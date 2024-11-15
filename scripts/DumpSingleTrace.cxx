#include <string>
#include <fstream>
#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>

#include "RootDevStruct.hpp"

void DumpSingleTrace(unsigned int entrynum,int crateNum,int chanNum,int modNum,bool ispileup,std::string inputname,std::string outputname){
	TFile* file = new TFile(inputname.c_str(),"READ");
	TTree* tree = dynamic_cast<TTree*>(file->Get("RootDev"));
	TTreeReader reader(tree);
	TTreeReaderArray<ProcessorStruct::RootDev> rd(reader,"data_vec");
	
	unsigned int curr = 0;
	std::ofstream out(outputname);
	while(reader.Next()){
		for( auto iter = rd.begin(); iter != rd.end(); ++iter ){
			if( iter->crateNum == crateNum and iter->chanNum == chanNum and iter->modNum == modNum and (iter->pileup == ispileup) ){
				if( curr == entrynum ){
					auto trace = iter->trace;
					for( size_t ii = 0; ii < trace.size(); ++ii ){
						out << ii << ' ' << trace[ii] << std::endl;
					}
					out.close();
					return;
				}
				++curr;
			}
		}
	}
}
