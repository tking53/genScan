#include <TROOT.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include "Gates.hpp"
#include "StringManipFunctions.hpp"
#include "CutManager.hpp"
#include "HistogramManager.hpp"
#include "RootFileManager.hpp"

#include "MtasImplantStruct.hpp"
#include "MtasStruct.hpp"
#include "PidStruct.hpp"
#include "VetoStruct.hpp"

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

int main(int argc, char *argv[]) {
	std::chrono::time_point<std::chrono::high_resolution_clock> global_start_time = std::chrono::high_resolution_clock::now();

	int port;
	std::string configfile;
	std::string outputprefix;
	std::vector<std::string> inputfiles;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::vector<std::string>>(&inputfiles)->multitoken(),"input root file for merging")
		("configfile,c",boost::program_options::value<std::string>(&configfile),"yaml config file to read in settings")
		("outputprefix,o",boost::program_options::value<std::string>(&outputprefix),"output prefix to dump the histograms/trimmed root tree to")
		("port,p",boost::program_options::value<int>(&port)->default_value(9090),"[portid] port to listen/send on for the live histogramming, -1 disables for batch scanning")
		;


	boost::program_options::positional_options_description p;
	p.add("inputfile", -1);



	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}


		const std::string logname = "genmerger";
		const std::string logfilename = (outputprefix)+".log";
		const std::string errfilename = (outputprefix)+".err";
		const std::string dbgfilename = (outputprefix)+".dbg";

		spdlog::set_level(spdlog::level::debug);
		std::shared_ptr<spdlog::sinks::basic_file_sink_mt> LogFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilename,true);
		LogFileSink->set_level(spdlog::level::info);

		std::shared_ptr<spdlog::sinks::basic_file_sink_mt> ErrorFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(errfilename,true);
		ErrorFileSink->set_level(spdlog::level::err);

		std::shared_ptr<spdlog::sinks::basic_file_sink_mt> DebugFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(dbgfilename,true);
		DebugFileSink->set_level(spdlog::level::debug);

		std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogFileConsole = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		LogFileConsole->set_level(spdlog::level::info);

		std::vector<spdlog::sink_ptr> sinks {DebugFileSink,LogFileSink,ErrorFileSink,LogFileConsole};
		auto console = std::make_shared<spdlog::logger>(logname,sinks.begin(),sinks.end());
		spdlog::initialize_logger(console);
		console->flush_on(spdlog::level::info);

		console->info("Beginning running");

		YAML::Node doc = YAML::LoadFile(configfile);

		std::shared_ptr<CUTS::CutRegistry> CutManager(new CUTS::CutRegistry(logname));
		CutManager->AddCut("PID",doc["PID"].as<std::string>());

		//this is the good one for fp1 and fp2 usually
		int tofid = doc["TOFID"].as<int>(6);

		//this must be given in seconds
		double forward_corr_time = doc["CORRELATION"]["Forward"].as<double>(1.0);
		double backward_corr_time = doc["CORRELATION"]["Backward"].as<double>(-1.0);
		if( backward_corr_time > 0 ){
			backward_corr_time *= -1.0;
		}

		std::map<std::string,PLOTS::HisHelper1D> His1D = {
			{"TDiff_Beta_Ion_s",{1000,-10,10}},
			{"TDiff_Beta_Ion_ms",{10000,-500,500}},
			{"Radius_Beta_Ion",{1000,0,10}}
		};

		for( const auto& kv : doc["HISTOGRAM1D"] ){
			auto name = kv.first.as<std::string>();
			int nbinsx = kv.second["nbinsx"].as<int>(His1D.at(name).nbinsx);
			double xlow = kv.second["xlow"].as<double>(His1D.at(name).xlow);
			double xhigh = kv.second["xhigh"].as<double>(His1D.at(name).xhigh);
			His1D[name] = {nbinsx,xlow,xhigh};
		}

		std::map<std::string,PLOTS::HisHelper2D> His2D = {
			{"Implant_Radius_Beta_Ion",{16384,0,16384,1000,0,10}},
			{"AnodeSum_Radius_Beta_Ion",{4096,0,16384,1000,0,10}},
			{"TDiff_Radius_Beta_Ion_s",{1000,-10,10,1000,0,10}},
			{"Mtas_TDiff_Beta_Ion_Gamma_s",{16384,0,16384,1000,-10,10}}
		};

		for( const auto& kv : doc["HISTOGRAM2D"] ){
			auto name = kv.first.as<std::string>();
			int nbinsx = kv.second["nbinsx"].as<int>(His2D.at(name).nbinsx);
			double xlow = kv.second["xlow"].as<double>(His2D.at(name).xlow);
			double xhigh = kv.second["xhigh"].as<double>(His2D.at(name).xhigh);
			int nbinsy = kv.second["nbinsy"].as<int>(His2D.at(name).nbinsy);
			double ylow = kv.second["ylow"].as<double>(His2D.at(name).ylow);
			double yhigh = kv.second["yhigh"].as<double>(His2D.at(name).yhigh);
			His2D[name] = {nbinsx,xlow,xhigh,nbinsy,ylow,yhigh};
		}


		std::vector<Gate<double>> RitReject;
		for( const auto& g : doc["RIT"] ){
			auto lowerbound = g["Gate"]["lowerbound"].as<double>(-1.0);
			auto upperbound = g["Gate"]["upperbound"].as<double>(-1.0);
			RitReject.push_back(Gate<double>(lowerbound,upperbound));
			console->info("Found Rit Rejection Bounds : {} {}",lowerbound,upperbound);
		}

		Gate<double> ValidIon(2000.0,16384.0);
		if( auto g = doc["ION"] ){
			auto lowerbound = g["lowerbound"].as<double>(2000.0);
			auto upperbound = g["upperbound"].as<double>(16384.0);
			ValidIon = Gate<double>(lowerbound,upperbound);
		}
		console->info("Found Valid Ion Bounds : {} {}",ValidIon.GetLowerBound(),ValidIon.GetUpperBound());

		Gate<double> ValidBeta;
		if( auto g  = doc["BETA"] ){
			auto lowerbound = g["lowerbound"].as<double>(50.0);
			auto upperbound = g["upperbound"].as<double>(8192.0);
			ValidBeta = Gate<double>(lowerbound,upperbound);
		}
		console->info("Found Valid Beta Bounds : {} {}",ValidBeta.GetLowerBound(),ValidBeta.GetUpperBound());

		std::shared_ptr<PLOTS::PlotRegistry> HistogramManager(new PLOTS::PlotRegistry(logname,StringManip::StripFileExtension(outputprefix),port));

		HistogramManager->RegisterPlot<TH1F>("TDiff_Beta_Ion_s","TDiff [Beta - Ion]; Time Difference (s); Counts per s",His1D["TDiff_Beta_Ion_s"]);
		HistogramManager->RegisterPlot<TH1F>("TDiff_Beta_Ion_ms","TDiff [Beta - Ion]; Time Difference (ms); Counts per ms",His1D["TDiff_Beta_Ion_ms"]);
		
		HistogramManager->RegisterPlot<TH1F>("Radius_Beta_Ion","Radius [Beta - Ion]; Radius (arb.); ",His1D["Radius_Beta_Ion"]);

		HistogramManager->RegisterPlot<TH2F>("Implant_Radius_Beta_Ion","Radius vs Energy [Beta - Ion]; Energy (keV); Radius (arb.); ",His2D["Implant_Radius_Beta_Ion"]);
		HistogramManager->RegisterPlot<TH2F>("TDiff_Radius_Beta_Ion_s","Radius vs TDiff [Beta - Ion];  TDiff (s); Radius (arb.)",His2D["TDiff_Radius_Beta_Ion_s"]);
		HistogramManager->RegisterPlot<TH2F>("AnodeSum_Radius_Beta_Ion","Radius vs Energy [Beta - Ion]; Energy (keV); Radius (arb.); ",His2D["AnodeSum_Radius_Beta_Ion"]);
		HistogramManager->RegisterPlot<TH2F>("Mtas_TDiff_Beta_Ion_Gamma_s","TDiff vs Energy [Beta - Ion - Gamma]; Energy (keV); TDiff (s); ",His2D["Mtas_TDiff_Beta_Ion_Gamma_s"]);
		
		console->info("Generating {}.list file that contains all the declared histograms",StringManip::GetFileBaseName(outputprefix));
		HistogramManager->WriteInfo();

		ProcessorStruct::MtasImplant* lowgain = nullptr;
		ProcessorStruct::MtasImplant* highgain = nullptr;

		std::vector<ProcessorStruct::MtasTotal*> Total(5,nullptr);
		std::vector<ProcessorStruct::MtasSegment*> Segment(24,nullptr);

		double rf = 0.0;
		ProcessorStruct::DBOX* db3 = nullptr;
		ProcessorStruct::DBOX* db4 = nullptr;
		ProcessorStruct::DBOX* db5 = nullptr;
		ProcessorStruct::FP* fp1 = nullptr;
		ProcessorStruct::FP* fp2 = nullptr;
		std::vector<double> fp1Tofs(10,-999);
		std::vector<double> fp2Tofs(10,-999);

		ProcessorStruct::Veto* fit = nullptr;
		ProcessorStruct::Veto* rit = nullptr;


		//try this way if it doesn't work then we make a tchain of everything
		//this is currently specific to MTAS, need to make this dynamic
		auto implant = new TChain("MtasImplant");
		auto mtas = new TChain("Mtas");
		auto pid = new TChain("Pid");
		auto veto = new TChain("Veto");

		for( const auto& f : inputfiles ){
			implant->Add(f.c_str());
			mtas->Add(f.c_str());
			pid->Add(f.c_str());
			veto->Add(f.c_str());
		}

		implant->SetBranchAddress("lowgain",&lowgain);
		implant->SetBranchAddress("highgain",&highgain);

		mtas->SetBranchAddress("Total",&(Total[0]));
		mtas->SetBranchAddress("CenterRing",&(Total[1]));
		mtas->SetBranchAddress("InnerRing",&(Total[2]));
		mtas->SetBranchAddress("MiddleRing",&(Total[3]));
		mtas->SetBranchAddress("OuterRing",&(Total[4]));
		mtas->SetBranchAddress("C1",&(Segment[0]));
		mtas->SetBranchAddress("C2",&(Segment[1]));
		mtas->SetBranchAddress("C3",&(Segment[2]));
		mtas->SetBranchAddress("C4",&(Segment[3]));
		mtas->SetBranchAddress("C5",&(Segment[4]));
		mtas->SetBranchAddress("C6",&(Segment[5]));
		mtas->SetBranchAddress("I1",&(Segment[6]));
		mtas->SetBranchAddress("I2",&(Segment[7]));
		mtas->SetBranchAddress("I3",&(Segment[8]));
		mtas->SetBranchAddress("I4",&(Segment[9]));
		mtas->SetBranchAddress("I5",&(Segment[10]));
		mtas->SetBranchAddress("I6",&(Segment[11]));
		mtas->SetBranchAddress("M1",&(Segment[12]));
		mtas->SetBranchAddress("M2",&(Segment[13]));
		mtas->SetBranchAddress("M3",&(Segment[14]));
		mtas->SetBranchAddress("M4",&(Segment[15]));
		mtas->SetBranchAddress("M5",&(Segment[16]));
		mtas->SetBranchAddress("M6",&(Segment[17]));
		mtas->SetBranchAddress("O1",&(Segment[18]));
		mtas->SetBranchAddress("O2",&(Segment[19]));
		mtas->SetBranchAddress("O3",&(Segment[20]));
		mtas->SetBranchAddress("O4",&(Segment[21]));
		mtas->SetBranchAddress("O5",&(Segment[22]));
		mtas->SetBranchAddress("O6",&(Segment[23]));

		pid->SetBranchAddress("rf",&rf);
		pid->SetBranchAddress("db3",&db3);
		pid->SetBranchAddress("db4",&db4);
		pid->SetBranchAddress("db5",&db5);
		pid->SetBranchAddress("fp1",&fp1);
		pid->SetBranchAddress("fp2",&fp2);
		pid->SetBranchAddress("fp1Tof_0",&(fp1Tofs[0]));
		pid->SetBranchAddress("fp1Tof_1",&(fp1Tofs[1]));
		pid->SetBranchAddress("fp1Tof_2",&(fp1Tofs[2]));
		pid->SetBranchAddress("fp1Tof_3",&(fp1Tofs[3]));
		pid->SetBranchAddress("fp1Tof_4",&(fp1Tofs[4]));
		pid->SetBranchAddress("fp1Tof_5",&(fp1Tofs[5]));
		pid->SetBranchAddress("fp1Tof_6",&(fp1Tofs[6]));
		pid->SetBranchAddress("fp1Tof_7",&(fp1Tofs[7]));
		pid->SetBranchAddress("fp1Tof_8",&(fp1Tofs[8]));
		pid->SetBranchAddress("fp1Tof_9",&(fp1Tofs[9]));
		pid->SetBranchAddress("fp2Tof_0",&(fp2Tofs[0]));
		pid->SetBranchAddress("fp2Tof_1",&(fp2Tofs[1]));
		pid->SetBranchAddress("fp2Tof_2",&(fp2Tofs[2]));
		pid->SetBranchAddress("fp2Tof_3",&(fp2Tofs[3]));
		pid->SetBranchAddress("fp2Tof_4",&(fp2Tofs[4]));
		pid->SetBranchAddress("fp2Tof_5",&(fp2Tofs[5]));
		pid->SetBranchAddress("fp2Tof_6",&(fp2Tofs[6]));
		pid->SetBranchAddress("fp2Tof_7",&(fp2Tofs[7]));
		pid->SetBranchAddress("fp2Tof_8",&(fp2Tofs[8]));
		pid->SetBranchAddress("fp2Tof_9",&(fp2Tofs[9]));

		veto->SetBranchAddress("fit",&fit);
		veto->SetBranchAddress("rit",&rit);

		auto num_entries = pid->GetEntries();
		auto piter = num_entries/10;

		std::vector<ProcessorStruct::MtasImplant> ValidImplants;
		std::vector<ProcessorStruct::MtasImplant> RitRejectedImplants;
		std::vector<ProcessorStruct::MtasImplant> ValidBetas;
		std::vector<std::vector<ProcessorStruct::MtasTotal>> ValidTotals;
		std::vector<std::vector<ProcessorStruct::MtasSegment>> ValidSegments;
		for( Long64_t ii = 0; ii < num_entries; ++ii ){
			if( ii%piter == 0 ){
				console->info("Processed {}/{} Events",ii,num_entries);
			}

			pid->GetEntry(ii);
			implant->GetEntry(ii);
			veto->GetEntry(ii);
			mtas->GetEntry(ii);
			//this is the gate placed in EXP_11012, 
			bool LightIon = false;
			bool hasbeta = ValidBeta.IsWithin(highgain->dynodeerg);
			if( CutManager->IsWithin("PID",fp1Tofs[6],fp1->pin[0].energy) ){
				//let's only only load the rit/fit when we're inside a good tof
				for( const auto& g : RitReject ){
					if( g.IsWithin(rit->energy) ){
						LightIon = true;
						break;
					}
				}
				if( not ValidIon.IsWithin(lowgain->dynodeerg) ){
					LightIon = true;
				}
				//this is checking for either light ion or beta
				if( not LightIon ){
					ValidImplants.push_back(*lowgain);
				}else{
					RitRejectedImplants.push_back(*lowgain);
				}
			}
			if( hasbeta ){
				ValidBetas.push_back(*highgain);
				ValidTotals.push_back({*(Total[0]),*(Total[1]),*(Total[2]),*(Total[3]),*(Total[4])});
				ValidSegments.push_back({
						*(Segment[0]),*(Segment[1]),*(Segment[2]),*(Segment[3]),*(Segment[4]),*(Segment[5]),
						*(Segment[6]),*(Segment[7]),*(Segment[8]),*(Segment[9]),*(Segment[10]),*(Segment[11]),
						*(Segment[12]),*(Segment[13]),*(Segment[14]),*(Segment[15]),*(Segment[16]),*(Segment[17]),
						*(Segment[18]),*(Segment[19]),*(Segment[20]),*(Segment[21]),*(Segment[22]),*(Segment[23])
						});
			}
		}

		console->info("Found {} Valid Implants, {} RitRejectedImplants, {} Valid Betas",ValidImplants.size(),RitRejectedImplants.size(),ValidBetas.size());
		//correlate the betas and ions together
		//outer loop over ions, and for each ion keep a vector of betas within the forward and backward check

		//auto IsWithinCorrelationWindow = [](const double& forward_diff,const double& backward_diff,const double& ion_ts,const double& beta_ts){
		//	const auto tdiff = 1.0e-9*(beta_ts - ion_ts);	
		//	return tdiff > backward_diff and tdiff < forward_diff;
		//};

		//the actual correlation step
		console->info("Begin sorting");

		auto period = ValidImplants.size()/10;
		auto iiter = 0;
		for( const auto& ion : ValidImplants ){
			if( iiter%period == 0 ){
				console->info("Completed {}/{} Correllations",iiter,ValidImplants.size());
			}
			const auto b = ion.dynodets + backward_corr_time*1.0e9; 
			auto beta_begin = std::lower_bound(ValidBetas.begin(),ValidBetas.end(),b,
					[](const ProcessorStruct::MtasImplant& b,double t){ 
						return b.dynodets <= t; 
					});

			const auto f = ion.dynodets + forward_corr_time*1.0e9; 
			auto beta_end = std::upper_bound(ValidBetas.begin(),ValidBetas.end(),f,
					[](double t,const ProcessorStruct::MtasImplant& b){ 
						return b.dynodets >= t; 
					});

			//console->info("{}:{} {} {}:{}",beta_begin->first,beta_begin->second,ion.second,beta_end->first,beta_end->second);
			const auto ion_ts = ion.dynodets;
			const auto ion_x = ion.highresx;
			const auto ion_y = ion.highresy;
			auto start = std::distance(ValidBetas.begin(),beta_begin);
			auto stop = std::distance(ValidBetas.begin(),beta_end);
			for( auto iter = start; iter < stop; ++iter ){
				const auto beta_ts = ValidBetas[iter].dynodets;
				const auto beta_x = ValidBetas[iter].highresx;
				const auto beta_y = ValidBetas[iter].highresy;
				const auto tdiff = 1.0e-9*(beta_ts - ion_ts);
				const auto xdiff = ion_x - beta_x;
				const auto ydiff = ion_y - beta_y;
				const auto radius = std::sqrt(xdiff*xdiff + ydiff*ydiff);
				const auto beta_erg = ValidBetas[iter].dynodeerg;
				const auto beta_anode_sum = ValidBetas[iter].anodesum;

				HistogramManager->Fill("TDiff_Beta_Ion_s",tdiff);
				HistogramManager->Fill("TDiff_Beta_Ion_ms",1.0e3*tdiff);
				HistogramManager->Fill("Radius_Beta_Ion",radius);

				HistogramManager->Fill("Implant_Radius_Beta_Ion",beta_erg,radius);
				HistogramManager->Fill("AnodeSum_Radius_Beta_Ion",beta_anode_sum,radius);
				HistogramManager->Fill("TDiff_Radius_Beta_Ion_s",tdiff,radius);

				const auto T = ValidTotals[iter][0].sumenergy;
				HistogramManager->Fill("Mtas_TDiff_Beta_Ion_Gamma_s",T,tdiff);
			}
			++iiter;
		}
		console->info("Finished sorting");

		std::shared_ptr<RootFileManager> RootManager(new RootFileManager(logname,StringManip::StripFileExtension(outputprefix),false));

		HistogramManager->WriteAllPlots();


		std::chrono::time_point<std::chrono::high_resolution_clock> global_stop_time = std::chrono::high_resolution_clock::now();
		auto global_run_time = global_stop_time - global_start_time;
		const auto hrs = std::chrono::duration_cast<std::chrono::hours>(global_run_time);
		const auto mins = std::chrono::duration_cast<std::chrono::minutes>(global_run_time - hrs);
		const auto secs = std::chrono::duration_cast<std::chrono::seconds>(global_run_time - hrs - mins);
		const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(global_run_time - hrs - mins - secs);
		console->info("Finished running in {} hours {} minutes {} seconds {} milliseconds",hrs.count(),mins.count(),secs.count(),ms.count());
		console->critical("All data has been written to {}.root",outputprefix);
		spdlog::shutdown();
	}catch( std::exception& e){
		spdlog::error(e.what());
		spdlog::shutdown();
		exit(EXIT_FAILURE);
	}    
}
