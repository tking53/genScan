#include <map>
#include <filesystem>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>
#include <ROOT/TTreeProcessorMT.hxx>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>
#include <ROOT/RDF/HistoModels.hxx>

#include "BSMStruct.hpp"
#include "MtasStruct.hpp"

//NOTE: YOU NEED TO MODIFY YOUR ~/.rootlogon.C to link against 
//the genscan libraries for this to work, similar to how 
//we have to modify things on a mac to link against opencl
//no clue how we should actually properly fix this but this hack works 
//for now. If you do not know how to do this, contact T. Ruland
//ALSO THIS SCRIPT MUST BE COMPILED, UNLIKE THE OTHERS SHIPPED BY GENSCAN!!!!
//at some point this will likely move into tools and be turned into its own exe
//but for now I am lazy af and its christmas tomorrow so woe to ye who stumbled upon this message
#include "CommonFitFunctions.hpp"
#include "PulseFitFunctions.hpp"
#include "PeakFitFunctions.hpp"
#include "PeakFitter.hpp"

void BSMPileupTraceFitter(const std::string& filename,const std::string& oup,const int& nthreads){
	ROOT::EnableImplicitMT(nthreads);
	unsigned int numThreads = ROOT::GetThreadPoolSize();
	std::chrono::time_point<std::chrono::high_resolution_clock> global_start_time = std::chrono::high_resolution_clock::now();

	auto file = TFile::Open(filename.c_str(),"READ");

	auto bsm = file->Get<TTree>("BSMTraceFit");
	auto mtas = file->Get<TTree>("Mtas");
	bsm->AddFriend(mtas);

	std::vector<TH1*> histos(numThreads,nullptr);
	std::vector<PeakFitter1D*> fitters(numThreads,nullptr);

	double ns_per_tick = 4.0;
	std::vector<double> cal_scale = { 1.0, 1.0 }; 
	double lb = 10.0;
	double ub = 200.0;
	bool chi2 = false;
	auto mode = FitTypes::OneDim::DoublePlasticTrace;
	std::map<std::string,double> fixed_values = {
	};
	std::map<std::string,std::pair<double,double>> bounded_values{
		{"Offset",{2600.0,18800.0}},
		{"Amp1",{0.0,65536.0}},
		{"Amp2",{0.0,65536.0}},
		{"Rise1",{1.0e-1,2.0}},
		{"Fall1",{3.0,5.0}},
		{"Rise2",{1.0e-1,2.0}},
		{"Fall2",{3.0,5.0}},
		{"T01",{110.0,120.0}}
	};

	auto df = ROOT::RDataFrame(*bsm);

	auto fit_pulse = [&](const unsigned int& thread_id,const ROOT::RVec<ProcessorStruct::BSMSingle>& f){
				if( histos[thread_id] == nullptr ){
					histos[thread_id] = new TH1F(("h"+std::to_string(thread_id)).c_str(),"",f[0].trace.size(),0,f[0].trace.size());
				}
				for( size_t ii = 0; ii < f[0].trace.size(); ++ii ){
					histos[thread_id]->SetBinContent(ii,f[0].trace[ii]);
					//roughly ok
					histos[thread_id]->SetBinError(ii,1.0);
				}
				fitters[thread_id] = new PeakFitter1D(lb,ub,chi2,mode,histos[thread_id],fixed_values,bounded_values);
				std::vector<double> ergs = { 
					(*(fitters[thread_id]))["Amp1"].first, 
					(*(fitters[thread_id]))["Amp2"].first 
				};

				std::vector<double> toff = {
					(*(fitters[thread_id]))["T01"].first, 
					(*(fitters[thread_id]))["T02"].first 
				};

				std::vector<double> rise = {
					(*(fitters[thread_id]))["Rise1"].first, 
					(*(fitters[thread_id]))["Rise2"].first 
				};

				std::vector<double> fall = {
					(*(fitters[thread_id]))["Fall1"].first, 
					(*(fitters[thread_id]))["Fall2"].first 
				};

				double offset = (*(fitters[thread_id]))["Offset"].first;
				
				delete fitters[thread_id];
				fitters[thread_id] = nullptr;
				
				if( toff[0] <= toff[1] ){ 
					ROOT::RVecD data{toff[0],toff[1],ergs[0],ergs[1],rise[0],rise[1],fall[0],fall[1],offset};
					return data;
				}else{
					ROOT::RVecD data{toff[1],toff[0],ergs[1],ergs[0],rise[1],rise[0],fall[1],fall[0],offset};
					return data;
				}
			};

	auto t01 = [&](const ROOT::RVecD& f){
		return f[0]*ns_per_tick;
	};

	auto t02 = [&](const ROOT::RVecD& f){
		return f[1]*ns_per_tick;
	};

	auto e01 = [&](const ROOT::RVecD& f){
		return f[2]*cal_scale[0];
	};

	auto e02 = [&](const ROOT::RVecD& f){
		return f[3]*cal_scale[1];
	};

	auto r01 = [&](const ROOT::RVecD& f){
		return f[4];
	};

	auto r02 = [&](const ROOT::RVecD& f){
		return f[5];
	};

	auto f01 = [&](const ROOT::RVecD& f){
		return f[6];
	};

	auto f02 = [&](const ROOT::RVecD& f){
		return f[7];
	};

	auto off = [&](const ROOT::RVecD& f){
		return f[8];
	};

	auto tdiff = [](const double& t0,const double& t1){
		return t1 - t0;
	};

	auto average = [](const double& x,const double& y){
		return (x+y)/2.0;
	};

	auto total = [](const double& f,const double& b){
		return std::sqrt(f*b);
	};

	auto sum = [](const double& x,const double& y){
		return x+y;
	};

	auto front_df = df
		.DefineSlot("front_fit_info",fit_pulse,{"front"})
		.Define("front_fit_t01",t01,{"front_fit_info"})
		.Define("front_fit_t02",t02,{"front_fit_info"})
		.Define("front_fit_e01",e01,{"front_fit_info"})
		.Define("front_fit_e02",e02,{"front_fit_info"})
		.Define("front_fit_r01",r01,{"front_fit_info"})
		.Define("front_fit_r02",r02,{"front_fit_info"})
		.Define("front_fit_f01",f01,{"front_fit_info"})
		.Define("front_fit_f02",f02,{"front_fit_info"})
		.Define("front_fit_off",off,{"front_fit_info"})
		.Define("front_tdiff",tdiff,{"front_fit_t01","front_fit_t02"})
		.Define("front_sum_e",sum,{"front_fit_e01","front_fit_e02"});

	auto back_df = front_df
		.DefineSlot("back_fit_info",fit_pulse,{"back"})
		.Define("back_fit_t01",t01,{"back_fit_info"})
		.Define("back_fit_t02",t02,{"back_fit_info"})
		.Define("back_fit_e01",e01,{"back_fit_info"})
		.Define("back_fit_e02",e02,{"back_fit_info"})
		.Define("back_fit_r01",r01,{"back_fit_info"})
		.Define("back_fit_r02",r02,{"back_fit_info"})
		.Define("back_fit_f01",f01,{"back_fit_info"})
		.Define("back_fit_f02",f02,{"back_fit_info"})
		.Define("back_fit_off",off,{"back_fit_info"})
		.Define("back_tdiff",tdiff,{"back_fit_t01","back_fit_t02"})
		.Define("back_sum_e",sum,{"back_fit_e01","back_fit_e02"});

	auto augmented_df = back_df
		.Define("e1_total",total,{"front_fit_e01","back_fit_e01"})
		.Define("e2_total",total,{"front_fit_e02","back_fit_e02"})
		.Define("e_total",sum,{"e1_total","e2_total"})
		.Define("avg_tdiff",average,{"front_tdiff","back_tdiff"});

	std::filesystem::path outputprefix(oup);
	std::filesystem::path opr = outputprefix.parent_path().string()+outputprefix.stem().string()+".root";

	augmented_df.Snapshot("FitTreeInfo",opr.c_str());

	//auto e1 = augmented_df.Histo1D(ROOT::RDF::TH1DModel("e1","e1",65536,0,65536),"e1_total");
	//auto e2 = augmented_df.Histo1D(ROOT::RDF::TH1DModel("e2","e2",65536,0,65536),"e2_total");
	//auto et = augmented_df.Histo1D(ROOT::RDF::TH1DModel("et","et",65536,0,65536),"e_total");
	//auto ft = augmented_df.Histo1D(ROOT::RDF::TH1DModel("ft","ft",65536,0,65536),"front_sum_e");
	//auto bt = augmented_df.Histo1D(ROOT::RDF::TH1DModel("bt","bt",65536,0,65536),"back_sum_e");
	//auto ftdiff = augmented_df.Histo1D(ROOT::RDF::TH1DModel("ftdiff","ftdiff",10000,0,1000),"front_tdiff");
	//auto btdiff = augmented_df.Histo1D(ROOT::RDF::TH1DModel("btdiff","btdiff",10000,0,1000),"back_tdiff");
	//auto atdiff = augmented_df.Histo1D(ROOT::RDF::TH1DModel("atdiff","atdiff",10000,0,1000),"avg_tdiff");
	//auto mtve1f = augmented_df.Histo1D(ROOT::RDF::TH2DModel("mtve1f","mtve1f",4096,0,4096,4096,0,4096),
	//		"Total.sumenergy","front_fit_e01");
	//auto mtve2f = augmented_df.Histo1D(ROOT::RDF::TH2DModel("mtve1f","mtve1f",4096,0,4096,4096,0,4096),
	//		"Total.sumenergy","front_fit_e02");
	//auto mtve1b = augmented_df.Histo1D(ROOT::RDF::TH2DModel("mtve1b","mtve1b",4096,0,4096,4096,0,4096),
	//		"Total.sumenergy","front_fit_e01");
	//auto mtve2b = augmented_df.Histo1D(ROOT::RDF::TH2DModel("mtve1b","mtve1b",4096,0,4096,4096,0,4096),
	//		"Total.sumenergy","front_fit_e02");

	//e1->SetDirectory(0);
	//e2->SetDirectory(0);
	//et->SetDirectory(0);
	//ft->SetDirectory(0);
	//bt->SetDirectory(0);
	//ftdiff->SetDirectory(0);
	//btdiff->SetDirectory(0);
	//atdiff->SetDirectory(0);

	//auto ofile = TFile::Open(opr.c_str(),"RECREATE");
	//
	//e1->Write(0,2,2);
	//e2->Write(0,2,2);
	//et->Write(0,2,2);
	//ft->Write(0,2,2);
	//bt->Write(0,2,2);
	//ftdiff->Write(0,2,2);
	//btdiff->Write(0,2,2);
	//atdiff->Write(0,2,2);

	//ofile->Close();

	//file->cd();
	//file->Close();

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
}
