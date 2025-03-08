#include "WaveformAnalyzer.hpp"
#include "PulseFitFunctions.hpp"

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
		{1022, {16384,0,16384.0,512,0,512.0}}
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

void WaveformAnalyzer::Init(const YAML::Node& config){
	console->info("Init called with YAML::Node");
	this->LoadHistogramSettings(config);
}

void WaveformAnalyzer::Init(const Json::Value& config){
	console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
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

		this->WaveSettings.push_back(std::make_pair(re,WaveFormParams()));
		this->WaveSettings.back().second.CalcDerivative = settings.attribute("CalcDerivative").as_bool(false);
		this->ParsePreTrigger(settings,this->WaveSettings.back().second);
		this->ParsePostTrigger(settings,this->WaveSettings.back().second);
		this->ParseQDC(settings,this->WaveSettings.back().second);
		this->ParsePSD(settings,this->WaveSettings.back().second);

		for( pugi::xml_node fitsettings = settings.child("FitSettings"); fitsettings; fitsettings = fitsettings.next_sibling("FitSettings") ){
			this->console->info("Found trace settings for regex:{}",re.str());

			this->TraceFitSettings.push_back(std::make_pair(re,TraceFitParams()));
			std::string fitname = fitsettings.attribute("name").as_string("");
			this->TraceFitSettings.back().second.FitFuncName = fitname;

			double fitlowbound = fitsettings.attribute("min").as_double(0.0);
			double fithighbound = fitsettings.attribute("max").as_double(0.0);
			this->TraceFitSettings.back().second.FitRange = {fitlowbound,fithighbound};

			for( pugi::xml_node fitparam = fitsettings.child("FitParam"); fitparam; fitparam = fitparam.next_sibling("FitParam") ){
				int idx = fitparam.attribute("idx").as_int(-1);
				double value = fitparam.attribute("value").as_double(0.0);
				bool isbounded = fitparam.attribute("bounded").as_bool(false);
				bool isfixed = fitparam.attribute("fixed").as_bool(false);
				std::string parname = fitparam.attribute("name").as_string("");
				if( isbounded ){
					double lowbound = fitparam.attribute("min").as_double(0.0);
					double highbound = fitparam.attribute("max").as_double(0.0);
					if( lowbound >= highbound or value < lowbound or value > highbound ){
						this->console->error("bounded value not between lowbound and highbound {} : [{},{},{}]",parname,lowbound,value,highbound);
						throw "Bounded value not between lowbound and highbound";
					}
					this->TraceFitSettings.back().second.ParamInfo.push_back({idx,isbounded,isfixed,parname,value,lowbound,highbound});
				}
				if( isfixed ){
					this->TraceFitSettings.back().second.ParamInfo.push_back({idx,isbounded,isfixed,parname,value,value,value});
				}
				if( idx < 0 ){
					this->console->error("idx not assigned, need value greater than 0");
					throw "idx not assigned to fit parameter";
				}

				if( isfixed and isbounded ){
					this->console->error("value is set to both bounded and fixed {} : {}",parname,value);
					throw "Value is set to both bounded and fixed";
				}
			}
			std::sort(this->TraceFitSettings.back().second.ParamInfo.begin(),this->TraceFitSettings.back().second.ParamInfo.end(),[](const std::tuple<int,bool,bool,std::string,double,double,double>& a,const std::tuple<int,bool,bool,std::string,double,double,double>& b){ return std::get<0>(a) < std::get<0>(b); });

			this->TraceFitSettings.back().second.fithist = nullptr;
			this->TraceFitSettings.back().second.fitfunc = nullptr;
			if( fitname.compare("BSMSingleTracePulse") == 0 ){
				this->TraceFitSettings.back().second.fitfunc = new TF1(fitname.c_str(),PulseFit::BSMSingleTraceFit,fitlowbound,fithighbound,8);
				for( const auto& parinfo : this->TraceFitSettings.back().second.ParamInfo ){
					this->TraceFitSettings.back().second.fitfunc->SetParName(std::get<0>(parinfo),std::get<3>(parinfo).c_str());
				}
				this->TraceFitSettings.back().second.fithist = nullptr;
			}else{
				this->console->error("Unknown Trace Fitting Function : {}",fitname);
				throw "Unknown TraceFitting Function";
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
	if( this->ValidateSettingsString(cratestr) and this->ValidateSettingsString(modstr) and this->ValidateSettingsString(channelstr) ){
		boost::regex re("^"+cratestr+":"+modstr+":"+channelstr+"$");
		auto result = this->KnownWaveSettings.insert(re);
		if( result.first == this->KnownWaveSettings.end() ){
			this->console->error("duplicate settings tag found for Crate=\"{}\" Module=\"{}\" Channel=\"{}\"",cratestr,modstr,channelstr);
			throw "duplicate settings found in WaveformAnalyzer settings tag";
		}
		return re;
	}else{
		this->console->error("invalid string in either Crate, Module, or Channel attribute. Expect [first-last] or [a,b,c] where first, last, a, b, and c are all numbers");
		throw "invalid string in either Crate, Module, or Channel attribute. Expect [first-last] or [a,b,c] where first, last, a, b, and c are all numbers";
	}
}

bool WaveformAnalyzer::ValidateSettingsString(const std::string& teststr) const{
	return teststr.front() == '[' and teststr.back() == ']';
}

void WaveformAnalyzer::ParsePreTrigger(const pugi::xml_node& settings,WaveFormParams& wav){
	if( pugi::xml_node curr = settings.child("PreTrigger") ){
		wav.PreTriggerBounds = std::make_pair<size_t,size_t>(curr.attribute("min").as_int(0),curr.attribute("max").as_int(0));
		this->console->info("Found PreTrigger node : [min,max) -> [{},{})",wav.PreTriggerBounds.first,wav.PreTriggerBounds.second);
	}else{
		this->console->error("missing PreTrigger node in settings node {}",settings);
		throw "missing PreTrigger node";
	}
}

void WaveformAnalyzer::ParsePostTrigger(const pugi::xml_node& settings,WaveFormParams& wav){
	if( pugi::xml_node curr = settings.child("PostTrigger") ){
		wav.PostTriggerBounds = std::make_pair<size_t,size_t>(curr.attribute("min").as_int(0),curr.attribute("max").as_int(0));
		this->console->info("Found PostTrigger node : [min,max) -> [{},{})",wav.PostTriggerBounds.first,wav.PostTriggerBounds.second);
	}else{
		this->console->error("missing PostTrigger node in settings node {}",settings);
		throw "missing PostTrigger node";
	}
}

void WaveformAnalyzer::ParseQDC(const pugi::xml_node& settings,WaveFormParams& wav){
	if( pugi::xml_node curr = settings.child("QDC") ){
		std::string vals = curr.attribute("bounds").as_string("");
		if( vals.empty() ){
			this->console->error("missing bounds attribute in QDC node, should be a list of the bins/entries of the trace to do the trace qdc");
			this->console->error("e.g. bounds=\"0,10,15,20\" will integrate the trace from [0,10), [10,15), [15,20)");
			throw "missing bounds attribute in QDC node";
		}else{
			boost::regex digit("\\d{1,}");
			boost::sregex_iterator iter(vals.begin(),vals.end(),digit);
			boost::sregex_iterator end;
			for(; iter != end; ++iter){
				wav.QDCBounds.push_back(static_cast<size_t>(std::stoi(iter->str())));
			}
			if( wav.QDCBounds.size() < 2 ){
				this->console->error("need at least two value for the QDC bounds attribute");
				throw "need at least two value for the QDC bounds attribute";
			}
			this->console->info("Found QDC node : NumSums -> {}, BoundsString -> [{})",wav.QDCBounds.size()-1,vals);
		}
	}else{
		this->console->error("missing QDC node in settings node {}",settings);
		throw "missing QDC node";
	}
}

void WaveformAnalyzer::ParsePSD(const pugi::xml_node& settings,WaveFormParams& wav){
	if( pugi::xml_node curr = settings.child("PSD") ){
		wav.HasPSD = true;
		wav.FixedPSDBounds = std::make_tuple<size_t,size_t,size_t>(curr.attribute("begin").as_int(0),curr.attribute("middle").as_int(0),curr.attribute("end").as_int(0));
		wav.FractionalPSDBounds = std::make_tuple<size_t,size_t,float>(curr.attribute("pre").as_int(0),curr.attribute("post").as_int(0),curr.attribute("fraction").as_float(2.0));
		this->console->info("Found PSD node : [begin,middle,end)",std::get<0>(wav.FixedPSDBounds),std::get<1>(wav.FixedPSDBounds),std::get<2>(wav.FixedPSDBounds));
		this->console->info("Found PSD node : [pre,post) fraction",std::get<0>(wav.FractionalPSDBounds),std::get<1>(wav.FractionalPSDBounds),std::get<2>(wav.FractionalPSDBounds));

	}
}
