#include "WaveformAnalyzer.hpp"
#include <stdexcept>

WaveformAnalyzer::WaveformAnalyzer(const std::string& log) : Analyzer(log,"WaveformAnalyzer",{}){
	this->h2dsettings = {
		{1000, {1024,0,1024.0,512,0,512.0}},
		{1001, {16384,0,16384.0,512,0,512.0}},
		{1002, {1024,0,1024.0,512,0,512.0}},
		{1010, {1024,0,1024.0,512,0,512.0}},
		{1011, {16384,0,16384.0,512,0,512.0}},
		{1012, {1024,0,1024.0,512,0,512.0}},
		{1020, {1024,0,1024.0,512,0,512.0}},
		{1021, {16384,0,16384.0,512,0,512.0}},
		{1022, {16384,0,16384.0,512,0,512.0}},
		{1030, {16384,0.0,16384.0,512,0,512.0}},
		{1031, {16384,0.0,16384.0,512,0,512.0}},
		{1032, {16384,0.0,16384.0,512,0,512.0}},
		{1033, {16384,-16.0,16.0,512,0,512.0}},
		{1034, {16384,-16.0,16.0,512,0,512.0}},
		{1035, {16384,-16.0,16.0,512,0,512.0}},
	};

	this->currsave = 0;
	this->NumTraceFits = 0;
	this->fittime = 0.0;
}

WaveformAnalyzer::~WaveformAnalyzer(){
	this->console->info("Number of traces fit : {}, total time spent fitting {:.3f}s",this->NumTraceFits,this->fittime/1000.0);
}

[[maybe_unused]] bool WaveformAnalyzer::PreProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Analyzer::PreProcess();

	for( const auto& key : this->Types ){
		eventhistory->GetCurrentEventSummary()->GetDetectorSummary(this->AllDefaultRegex[key],this->SummaryData);
		//this->console->info("ROOTDEV Size for type {} : {}",key,this->SummaryData.size());
		for( auto& evt : this->SummaryData ){
			for( const auto& s : this->WaveSettings ){
				boost::smatch cmapmatch;
				if( boost::regex_match(evt->GetCMapID(),cmapmatch,s.first,boost::regex_constants::match_continuous) ){
					evt->AnalyzeWaveform(s.second.PreTriggerBounds,s.second.PostTriggerBounds,s.second.QDCBounds);
					if( s.second.CalcDerivative ){
						evt->CalculateTraceDerivatives();
					}
					if( s.second.HasPSD ){
						//auto fraction = std::get<2>(s.second.FractionalPSDBounds);
						//if( fraction <= 1.0 ){
						//	auto pre = std::get<0>(s.second.FractionalPSDBounds);
						//	auto post = std::get<1>(s.second.FractionalPSDBounds);
						//	evt->CalcTraceFractionalPSD(pre,post,fraction);
						//}

						auto mid = std::get<1>(s.second.FixedPSDBounds);
						if( mid > 0 ){
							auto begin = std::get<0>(s.second.FixedPSDBounds);
							auto end = std::get<2>(s.second.FixedPSDBounds);
							evt->CalcTraceFixedPSD(begin,mid,end);
							auto psd = evt->GetTraceFixedPSD();
							auto gcid = evt->GetGlobalChannelID();
							auto head = std::get<0>(psd);
							auto tail = std::get<1>(psd);
							auto total = std::get<2>(psd);

							hismanager->Fill("WAVE_1030",head,gcid);
							hismanager->Fill("WAVE_1031",tail,gcid);
							hismanager->Fill("WAVE_1032",total,gcid);
							
							hismanager->Fill("WAVE_1033",head/total,gcid);
							hismanager->Fill("WAVE_1034",tail/total,gcid);
							hismanager->Fill("WAVE_1035",head/tail,gcid);
						}
					}
					auto pre = evt->GetTracePreTriggerBaseline();
					auto post = evt->GetTracePostTriggerBaseline();
					auto maxval = evt->GetTraceMaxInfo();
					auto blmax = evt->GetBaselineSubtractedMaxValue();
					auto gcid = evt->GetGlobalChannelID();

					hismanager->Fill("WAVE_1000",s.second.PreTriggerBounds.first,gcid);
					hismanager->Fill("WAVE_1000",s.second.PreTriggerBounds.second,gcid);
					hismanager->Fill("WAVE_1001",pre.first,gcid);
					hismanager->Fill("WAVE_1002",pre.second,gcid);

					hismanager->Fill("WAVE_1010",s.second.PostTriggerBounds.first,gcid);
					hismanager->Fill("WAVE_1010",s.second.PostTriggerBounds.second,gcid);
					hismanager->Fill("WAVE_1011",post.first,gcid);
					hismanager->Fill("WAVE_1012",post.second,gcid);

					hismanager->Fill("WAVE_1020",maxval.first,gcid);
					hismanager->Fill("WAVE_1021",maxval.second,gcid);
					hismanager->Fill("WAVE_1022",blmax,gcid);

					break;
				}
			}
			//this->console->info("{} {}",evt->GetType(),this->SummaryData.size());
			for( auto& s : this->TraceFitSettings ){
				boost::smatch cmapmatch;
				if( boost::regex_match(evt->GetCMapID(),cmapmatch,s.first,boost::regex_constants::match_continuous) ){
					auto trace = evt->GetRawTrace();
					if( s.second.fithist == nullptr ){
						s.second.fithist = new TH1F((s.second.FitFuncName+evt->GetCMapID()).c_str(),s.second.FitFuncName.c_str(),trace.size(),0,trace.size());
					}
					for( size_t idx = 0; idx < trace.size(); ++idx ){
						s.second.fithist->SetBinContent(idx+1,trace.at(idx));
						//s.second.fithist->SetBinError(idx+1,std::sqrt(trace.at(idx)));
						s.second.fithist->SetBinError(idx+1,0.5);
					}
					for( const auto& parinfo : s.second.ParamInfo ){
						auto idx = std::get<0>(parinfo);
						auto isbounded = std::get<1>(parinfo);
						auto isfixed = std::get<2>(parinfo);
						auto value = std::get<4>(parinfo);
						auto lbound = std::get<5>(parinfo);
						auto ubound = std::get<6>(parinfo);

						s.second.fitfunc->SetParameter(idx,value);
						if( isfixed ){
							s.second.fitfunc->FixParameter(idx,value);
						}

						if( isbounded ){
							s.second.fitfunc->SetParLimits(idx,lbound,ubound);
						}

					}
					this->fit_start_time = std::chrono::high_resolution_clock::now();
					this->FitResult = s.second.fithist->Fit(s.second.fitfunc,"0SQ","",s.second.FitRange.first,s.second.FitRange.second);
					this->fit_stop_time = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double,std::milli> dur = this->fit_stop_time - this->fit_start_time;
					this->fittime += dur.count();
					++(this->NumTraceFits);
					//add params to evt
					for( const auto& parinfo : s.second.ParamInfo ){
						auto idx = std::get<0>(parinfo);
						auto parname = std::get<3>(parinfo);
						evt->AddTraceFitInfo(parname,this->FitResult->Parameter(idx),this->FitResult->ParError(idx));
						//this->console->info("{} : {}+-{}",parname,this->FitResult->Parameter(idx),this->FitResult->ParError(idx));
					}
					evt->AddTraceFitInfo("Chi2/NDF",this->FitResult->Chi2(),this->FitResult->Ndf());
					if( this->currsave < this->MaxSaveFits ){
						this->console->info("===== BEGIN TRACE FIT DUMP {}/{} ====",this->currsave,this->MaxSaveFits);
						std::string savename = "FitHist_"+std::to_string(this->currsave);
						this->console->info("saving fit for evt : {} as {}",*evt,savename);
						this->console->info("fit values : Chi2/NdF : {}/{} : {}",this->FitResult->Chi2(),this->FitResult->Ndf(),this->FitResult->Chi2()/static_cast<double>(this->FitResult->Ndf()));
						for( const auto& parinfo : s.second.ParamInfo ){
							auto idx = std::get<0>(parinfo);
							auto parname = std::get<3>(parinfo);
							this->console->info("{} : {} +- {}",parname,this->FitResult->Parameter(idx),this->FitResult->ParError(idx));
						}
						TH1* currhist = dynamic_cast<TH1*>(s.second.fithist->Clone(savename.c_str()));
						currhist->Write(0,2,0);
						this->console->info("===== END TRACE FIT DUMP {}/{} , {} ms====",this->currsave,this->MaxSaveFits,dur.count());
						++(this->currsave);
					}	
					s.second.fithist->GetListOfFunctions()->Clear();
					break;
				}
			}
		}
	}
	Analyzer::EndProcess();
	return true;
}

[[maybe_unused]] bool WaveformAnalyzer::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool WaveformAnalyzer::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void WaveformAnalyzer::Init(const pugi::xml_node& config){
	console->info("Init called with pugi::xml_node");

	std::string additional_types = config.attribute("included_types").as_string(""); 
	if( additional_types.empty() ){
		this->console->error("Missing included_types tag in xml tag for WaveformAnalyzer");
		throw "Missing included_types tag in xml tag for WaveformAnalyzer";
	}else{
		this->InsertAdditionalTypes(additional_types);
	}
	this->MaxSaveFits = config.attribute("max_save").as_int(100);

	for(pugi::xml_node settings = config.child("settings"); settings; settings = settings.next_sibling("settings")){
		auto re = this->GenerateRegex(settings.attribute("Crate").as_string("[\\d]"),settings.attribute("Module").as_string("[\\d]"),settings.attribute("Channel").as_string("[\\d]"));
		this->console->info("Found settings node regex:{}",re.str());

		try{
			this->WaveSettings.push_back(std::make_pair(re,PSDCalculator(settings)));
		}catch(std::runtime_error& e){
			this->console->error("error parsing waveform settings for r:{} {}",re.str(),e.what());
			throw e;
		}
		for( pugi::xml_node fitsettings = settings.child("FitSettings"); fitsettings; fitsettings = fitsettings.next_sibling("FitSettings") ){
			this->console->info("Found trace settings for regex:{}",re.str());

			try{
				this->TraceFitSettings.push_back(std::make_pair(re,RootFitter(fitsettings)));
			}catch(std::runtime_error& e){
				this->console->error("error parsing trace fitting settings for r:{} {}",re.str(),e.what());
				throw e;
			}

		}

	}

	if( this->WaveSettings.empty() ){
		this->console->error("Missing settings node");
		throw "Missing settings node";
	}

	this->LoadHistogramSettings(config);
}

void WaveformAnalyzer::Finalize(){
	this->console->info("{} has been finalized",this->AnalyzerName);
}


void WaveformAnalyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	console->info("Finished Declaring Plots");
	hismanager->RegisterPlot<TH2F>("WAVE_1000","PreTriggerRegion Bounds; Trace position (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1000));
	hismanager->RegisterPlot<TH2F>("WAVE_1001","PreTriggerRegion Baseline; Trace baseline (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1001));
	hismanager->RegisterPlot<TH2F>("WAVE_1002","PreTriggerRegion Baseline Std. Dev.; Trace baseline std. dev. (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1002));

	hismanager->RegisterPlot<TH2F>("WAVE_1010","PostTriggerRegion Bounds; Trace position (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1010));
	hismanager->RegisterPlot<TH2F>("WAVE_1011","PostTriggerRegion Baseline; Trace baseline (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1011));
	hismanager->RegisterPlot<TH2F>("WAVE_1012","PostTriggerRegion Baseline Std. Dev.; Trace baseline std. dev. (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1012));

	hismanager->RegisterPlot<TH2F>("WAVE_1020","Max Trace Location; Trace position (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1020));
	hismanager->RegisterPlot<TH2F>("WAVE_1021","Max Trace Value; adc value (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1021));
	hismanager->RegisterPlot<TH2F>("WAVE_1022","Baseline Subtraced Max Trace Value; adc value (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1022));

	hismanager->RegisterPlot<TH2F>("WAVE_1030","Channel vs Early PSD Integral ; Integral (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1030));
	hismanager->RegisterPlot<TH2F>("WAVE_1031","Channel vs Late PSD Integral ; Integral (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1031));
	hismanager->RegisterPlot<TH2F>("WAVE_1032","Channel vs Total PSD Integral ; Integral (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1032));

	hismanager->RegisterPlot<TH2F>("WAVE_1033","Channel vs PSD (Tail/Total) ; PSD (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1033));
	hismanager->RegisterPlot<TH2F>("WAVE_1034","Channel vs PSD (Head/Total) ; PSD (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1034));
	hismanager->RegisterPlot<TH2F>("WAVE_1035","Channel vs PSD (Head/Tail) ; PSD (arb.); Linearized Channel Number (arb.)",this->h2dsettings.at(1035));
}

void WaveformAnalyzer::InsertAdditionalTypes(const std::string& typestring){
	std::set<std::string> typelist = {};
	std::regex word_regex("(\\w+)");
	auto words_begin =std::sregex_iterator(typestring.begin(),typestring.end(), word_regex);
	auto words_end = std::sregex_iterator();
	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		typelist.insert(match.str());
	}
	for( const auto& t : typelist ){
		this->AssociateType(t);
	}
}

boost::regex WaveformAnalyzer::GenerateRegex(const std::string& cratestr,const std::string& modstr,const std::string& channelstr){
	boost::regex re("^"+cratestr+":"+modstr+":"+channelstr+"$");
	auto result = this->KnownWaveSettings.insert(re);
	if( result.first == this->KnownWaveSettings.end() ){
		this->console->error("duplicate settings tag found for Crate=\"{}\" Module=\"{}\" Channel=\"{}\"",cratestr,modstr,channelstr);
		throw "duplicate settings found in WaveformAnalyzer settings tag";
	}
	return re;
}
