#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <iostream>
#include <chrono>
#include <mutex>

#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>
#include <ROOT/TTreeProcessorMT.hxx>

#include "BSMStruct.hpp"
#include "MtasStruct.hpp"

// This whole thing could be simplified to 
//
// auto df = ROOT::RDataFrame(bsm);
// auto df_filtered = df.Filter();
// //this takes a third param: vector of branch names to retain
// df_filtered.Snapshot("newtree","output.root");
//
// The below code is still equivalent though, and leaves room for manipulating the trace
// the dataframe option fails to allow for creating something of the type 
// ROOT::RVec<ROOT::RVec<unsigned int>> fails to compile 
// ROOT::RVec<std::vector<unsigned int>> compiles but gives nonsense for the size
// may need to ask in forum how the hell do you bind this leaf of a branch and access it appropriately

void BSMPileupTraceStripper(const std::string& filename,const std::string& oup,const int& nthreads){
	std::chrono::time_point<std::chrono::high_resolution_clock> global_start_time = std::chrono::high_resolution_clock::now();

	auto file = TFile::Open(filename.c_str(),"READ");

	auto bsm = file->Get<TTree>("BSMTraceFit");
	auto mtas = file->Get<TTree>("Mtas");
	bsm->AddFriend(mtas);

	TTreeReader reader(bsm);

	ROOT::TTreeProcessorMT Workers(*bsm,nthreads);

	std::atomic<unsigned int> entrycount = 0;
	std::atomic<size_t> trace_size = 0;
	std::vector<unsigned int> idlist;
	std::mutex idlist_mutex;

	auto work_item = [&](TTreeReader& currrd){
		TTreeReaderArray<ProcessorStruct::BSMSingle> bsm_f(currrd,"front");
		TTreeReaderArray<ProcessorStruct::BSMSingle> bsm_b(currrd,"back");
		TTreeReaderValue<ProcessorStruct::MtasTotal> mtas_t(currrd,"Total");

		unsigned int idx = 0;
		while( currrd.Next() ){
			if( !bsm_f.IsEmpty() and !bsm_b.IsEmpty() ){
				auto both_pileup = bsm_f[0].pileup && bsm_b[0].pileup;
				auto neither_saturate = (!bsm_f[0].saturation) && (!bsm_b[0].saturation);
				auto mtas_saturate = mtas_t->saturate;
				if( both_pileup && neither_saturate && not mtas_saturate ){
					++entrycount;
					auto f_trace = bsm_f[0].trace;
					auto b_trace = bsm_b[0].trace;
					if( f_trace.size() > trace_size ){
						trace_size = f_trace.size();
					}
					if( b_trace.size() > trace_size ){
						trace_size = b_trace.size();
					}
					auto currid = currrd.GetCurrentEntry();
					std::lock_guard<std::mutex> guard(idlist_mutex);
					idlist.push_back(currid);
				}
			}
		}
	};

	Workers.Process(work_item);

	std::filesystem::path outputprefix(oup);
	std::filesystem::path op = outputprefix.parent_path().string()+outputprefix.stem().string()+"_idlist.txt";
	std::filesystem::path opr = outputprefix.parent_path().string()+outputprefix.stem().string()+".root";

	std::sort(idlist.begin(),idlist.end());
	
	std::ofstream ouf(op);
	for( const auto& id : idlist ){
		ouf << id << std::endl;
	}
	ouf.close();

	auto ofile = TFile::Open(opr.c_str(),"RECREATE");
	TTree* cloned_bsm = bsm->CloneTree(0);
	TTree* cloned_mtas = mtas->CloneTree(0);

	for( const auto& id : idlist ){
		bsm->GetEntry(id);
		mtas->GetEntry(id);

		cloned_bsm->Fill();
		cloned_mtas->Fill();
	}

	cloned_bsm->AutoSave();
	cloned_mtas->AutoSave();
	ofile->Close();
	file->Close();

	std::chrono::time_point<std::chrono::high_resolution_clock> global_stop_time = std::chrono::high_resolution_clock::now();
	auto global_run_time = global_stop_time - global_start_time;
	const auto hrs = std::chrono::duration_cast<std::chrono::hours>(global_run_time);
	const auto mins = std::chrono::duration_cast<std::chrono::minutes>(global_run_time - hrs);
	const auto secs = std::chrono::duration_cast<std::chrono::seconds>(global_run_time - hrs - mins);
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(global_run_time - hrs - mins - secs);

	std::cout << "Finished processing in " 
		  << hrs.count() << " hours "
		  << mins.count() << " mins " 
		  << secs.count() << " secs " 
		  << ms.count() << " ms" << std::endl;
	std::cout << "There are " << entrycount << " entries that satisfy the cut requested" << std::endl;
	std::cout << "idlist file " << op << " has " << idlist.size() << " entries in it" << std::endl;
	std::cout << "Expected uncompressed size of files would be >= " << sizeof(unsigned int)*3*trace_size << " bytes" << std::endl;
}
