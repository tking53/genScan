#include "MtasIsomerProcessor.hpp"
#include "Processor.hpp"
#include <memory>

MtasIsomerProcessor::MtasIsomerProcessor(const std::string& log,Processor* parent) : Processor(log,"MtasIsomerProcessor",{}){
	this->ParentProc = std::shared_ptr<Processor>(parent);

	this->h2dsettings = {
		{3500,{8192,0,8192,1000,0,10000}},
		{3501,{8192,0,8192,1000,0,10000}},
		{3502,{8192,0,8192,1000,0,10000}},
		{3503,{8192,0,8192,1000,0,10000}},
		{3504,{8192,0,8192,1000,0,10000}},
		{3505,{8192,0,8192,1000,0,10000}},

		{3600,{8192,0,8192,1000,0,10000}},
		{3601,{8192,0,8192,1000,0,10000}},
		{3602,{8192,0,8192,1000,0,10000}},
		{3603,{8192,0,8192,1000,0,10000}},
		{3604,{8192,0,8192,1000,0,10000}},
		{3605,{8192,0,8192,1000,0,10000}},

		{3700,{8192,0,8192,1000,0,10000}},
		{3701,{8192,0,8192,1000,0,10000}},
		{3702,{8192,0,8192,1000,0,10000}},
		{3703,{8192,0,8192,1000,0,10000}},
		{3704,{8192,0,8192,1000,0,10000}},
		{3705,{8192,0,8192,1000,0,10000}},

		{3800,{8192,0,8192,1000,0,10000}},
		{3801,{8192,0,8192,1000,0,10000}},
		{3802,{8192,0,8192,1000,0,10000}},
		{3803,{8192,0,8192,1000,0,10000}},
		{3804,{8192,0,8192,1000,0,10000}},
		{3805,{8192,0,8192,1000,0,10000}},

		{3900,{8192,0,8192,1000,0,1000}},

		{4500,{8192,0,8192,1000,0,10000}},
		{4501,{8192,0,8192,1000,0,10000}},
	
		{4600,{8192,0,8192,1000,0,10000}},
		{4601,{8192,0,8192,1000,0,10000}},

		{4700,{8192,0,8192,1000,0,10000}},
		{4701,{8192,0,8192,1000,0,10000}},

		{4800,{8192,0,8192,1000,0,10000}},
		{4801,{8192,0,8192,1000,0,10000}},

		{5500,{8192,0,8192,1000,0,10000}},
		{5501,{8192,0,8192,1000,0,10000}},
		{5502,{8192,0,8192,1000,0,10000}},
		{5503,{8192,0,8192,1000,0,10000}},
		{5504,{8192,0,8192,1000,0,10000}},
		{5505,{8192,0,8192,1000,0,10000}},
	
		{5700,{8192,0,8192,1000,0,10000}},
		{5701,{8192,0,8192,1000,0,10000}}
	};

	this->implant = "implant";
	this->hpge = "hpge";
	this->beta = "beta";
	this->gamma = "gamma";
	this->muon = "muon";
}

void MtasIsomerProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");

	for( pugi::xml_node boxgate = config.child("BoxGate"); boxgate; boxgate = boxgate.next_sibling("BoxGate") ){
		std::string label = boxgate.attribute("label").as_string("");
		if( label.compare("ISOMER_3701") == 0 ){
			auto xlow = boxgate.attribute("xlowerbound").as_double(0.0);
			auto xhigh = boxgate.attribute("xupperbound").as_double(16384.0);
			auto ylow = boxgate.attribute("ylowerbound").as_double(0.0);
			auto yhigh = boxgate.attribute("yupperbound").as_double(16384.0);
			this->ISOMER_3701_Gates.push_back(BoxGate<double>(xlow,xhigh,ylow,yhigh));
		}
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasIsomerProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasIsomerProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	
	//Prev    | Curr    | His 
	//beta    | no-beta | 370X   
	//no-beta | beta    | 380X 
	//beta    | beta    | 350X
	//no-beta | no-beta | 360X 
	hismanager->RegisterPlot<TH2F>("ISOMER_3500","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3500));
	hismanager->RegisterPlot<TH2F>("ISOMER_3501","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3501));
	hismanager->RegisterPlot<TH2F>("ISOMER_3502","Mtas prev-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3502));
	hismanager->RegisterPlot<TH2F>("ISOMER_3503","Mtas prev-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3503));
	hismanager->RegisterPlot<TH2F>("ISOMER_3504","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3504));
	hismanager->RegisterPlot<TH2F>("ISOMER_3505","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3505));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_3600","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3600));
	hismanager->RegisterPlot<TH2F>("ISOMER_3601","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3601));
	hismanager->RegisterPlot<TH2F>("ISOMER_3602","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3602));
	hismanager->RegisterPlot<TH2F>("ISOMER_3603","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3603));
	hismanager->RegisterPlot<TH2F>("ISOMER_3604","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3604));
	hismanager->RegisterPlot<TH2F>("ISOMER_3605","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3605));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_3700","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_3701","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3701));
	hismanager->RegisterPlot<TH2F>("ISOMER_3702","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3702));
	hismanager->RegisterPlot<TH2F>("ISOMER_3703","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3703));
	hismanager->RegisterPlot<TH2F>("ISOMER_3704","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3704));
	hismanager->RegisterPlot<TH2F>("ISOMER_3705","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3705));

	for( size_t ii = 0; ii < this->ISOMER_3701_Gates.size(); ++ii ){
		std::string label = "ISOMER_373"+std::to_string(ii);
		std::string title = "Mtas prev-#beta curr-no-#beta Measure Cycle Gated Curr Total Energy Gated [";
		title += std::to_string(this->ISOMER_3701_Gates.at(ii).GetLowerXBound())+","+std::to_string(this->ISOMER_3701_Gates.at(ii).GetUpperXBound());
		title += "] Time Gated [";
		title += std::to_string(this->ISOMER_3701_Gates.at(ii).GetLowerYBound())+","+std::to_string(this->ISOMER_3701_Gates.at(ii).GetUpperYBound());
		title += "]; Prev. Energy (keV);";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(3730));
	}

	hismanager->RegisterPlot<TH2F>("ISOMER_3800","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3800));
	hismanager->RegisterPlot<TH2F>("ISOMER_3801","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3801));
	hismanager->RegisterPlot<TH2F>("ISOMER_3802","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3802));
	hismanager->RegisterPlot<TH2F>("ISOMER_3803","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3803));
	hismanager->RegisterPlot<TH2F>("ISOMER_3804","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3804));
	hismanager->RegisterPlot<TH2F>("ISOMER_3805","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3805));

	hismanager->RegisterPlot<TH2F>("ISOMER_3900","Mtas curr-#beta TDiff (Last MTAS - First Si) Measure Cycle Gated; Energy (keV); Time (ns)",this->h2dsettings.at(3900));

	hismanager->RegisterPlot<TH2F>("ISOMER_4500","HPGe prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(4500));
	hismanager->RegisterPlot<TH2F>("ISOMER_4501","HPGe prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(4501));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_4600","HPGe prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(4600));
	hismanager->RegisterPlot<TH2F>("ISOMER_4601","HPGe prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(4601));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_4700","HPGe prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(4700));
	hismanager->RegisterPlot<TH2F>("ISOMER_4701","HPGe prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(4701));

	hismanager->RegisterPlot<TH2F>("ISOMER_4800","HPGe prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(4800));
	hismanager->RegisterPlot<TH2F>("ISOMER_4801","HPGe prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(4801));

	hismanager->RegisterPlot<TH2F>("ISOMER_5500","Silicon Max prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(5500));
	hismanager->RegisterPlot<TH2F>("ISOMER_5501","Silicon Max prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(5501));
	hismanager->RegisterPlot<TH2F>("ISOMER_5502","Silicon Max prev-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(5502));
	hismanager->RegisterPlot<TH2F>("ISOMER_5503","Silicon Max prev-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(5503));
	hismanager->RegisterPlot<TH2F>("ISOMER_5504","Silicon Max prev-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(5504));
	hismanager->RegisterPlot<TH2F>("ISOMER_5505","Silicon Max prev-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(5505));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_5700","Silicon Max prev-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(5700));
	hismanager->RegisterPlot<TH2F>("ISOMER_5701","Silicon Max prev-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(5701));

	this->console->info("Finished Declaring Plots");
}

		
void MtasIsomerProcessor::FillMtasPlots(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager,MtasProcessor* mtasproc,MtasSSDProcessor* siproc){
	//these are the traditional ISOMER_3XXX plots

	auto erg = mtasproc->GetTotalEnergy(0);

	hismanager->Fill("ISOMER_3900",erg,internaltdiff);
	auto numhist = eventhistory->GetMaxHistoryID();
	auto summary = eventhistory->GetCurrentEventSummary();
	bool hasbeta = summary->ContainsEventTag(this->beta);

	if( numhist > 1 ){
		//current event has gamma not-muon, not-beta
		if( not hasbeta ){
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto prevmuon = prevsummary->ContainsEventTag(this->muon);
				if( prevmuon ){
					continue;
				}
				auto prevbeta = prevsummary->ContainsEventTag(this->beta);
				auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
				//this looks for a beta decay into a delayed level
				//like 137Cs
				if( prevbeta ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					hismanager->Fill("ISOMER_3700",erg,isomer_tdiff);
					hismanager->Fill("ISOMER_3701",erg,isomer_tdiff*1.0e-3);

					auto oldsi = prevsummary->GetEventObservable("SiMax").value_or(0.0);
					hismanager->Fill("ISOMER_5700",oldsi,isomer_tdiff);
					hismanager->Fill("ISOMER_5701",oldsi,isomer_tdiff*1.0e-3);

					auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
					hismanager->Fill("ISOMER_3702",olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3703",olderg,isomer_tdiff*1.0e-3);
					//trying to find how we got into this level
					for( size_t ii = 0; ii < this->ISOMER_3701_Gates.size(); ++ii ){
						if( this->ISOMER_3701_Gates.at(ii).IsWithin(erg,isomer_tdiff*1.0e-3) ){
							std::string label = "ISOMER_373"+std::to_string(ii);
							hismanager->Fill(label,olderg);
						}
					}

					hismanager->Fill("ISOMER_3704",erg+olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3705",erg+olderg,isomer_tdiff*1.0e-3);
					break;
				}
			}
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto prevmuon = prevsummary->ContainsEventTag(this->muon);
				if( prevmuon ){
					continue;
				}
				auto prevbeta = prevsummary->ContainsEventTag(this->beta);
				auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
				//looking for stepping through short isomer after we start in daughter isomer
				if( not prevbeta ){
					//no beta in this event or the current one we're looking at, so we can ignore the 56XX series
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					hismanager->Fill("ISOMER_3600",erg,isomer_tdiff);
					hismanager->Fill("ISOMER_3601",erg,isomer_tdiff*1.0e-3);
					auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
					hismanager->Fill("ISOMER_3602",olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3603",olderg,isomer_tdiff*1.0e-3);

					hismanager->Fill("ISOMER_3604",erg+olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3605",erg+olderg,isomer_tdiff*1.0e-3);
					break;
				}
			}
		}
		//current event does not have beta 
		if( hasbeta ){
			auto sierg = summary->GetEventObservable("SiMax").value_or(0.0);
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto prevmuon = prevsummary->ContainsEventTag(this->muon);
				if( prevmuon ){
					continue;
				}
				auto prevbeta = prevsummary->ContainsEventTag(this->beta);
				auto prevgamma = prevsummary->ContainsEventTag(this->gamma);

				//this looks for a gamma decay into a delayed beta
				//i.e. beam isomer, but need mtas energy for this old event
				if( not prevbeta ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					hismanager->Fill("ISOMER_3800",erg,isomer_tdiff);
					hismanager->Fill("ISOMER_3801",erg,isomer_tdiff*1.0e-3);
					auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
					hismanager->Fill("ISOMER_3802",olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3803",olderg,isomer_tdiff*1.0e-3);

					hismanager->Fill("ISOMER_3804",erg+olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3805",erg+olderg,isomer_tdiff*1.0e-3);
					break;
				}
			}
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto prevmuon = prevsummary->ContainsEventTag(this->muon);
				if( prevmuon ){
					continue;
				}
				auto prevbeta = prevsummary->ContainsEventTag(this->beta);
				auto prevgamma = prevsummary->ContainsEventTag(this->gamma);

				//this looks for a gamma decay into a delayed beta
				//i.e. beam isomer, but need mtas energy for this old event
				if( prevbeta ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					hismanager->Fill("ISOMER_3500",erg,isomer_tdiff);
					hismanager->Fill("ISOMER_3501",erg,isomer_tdiff*1.0e-3);

					hismanager->Fill("ISOMER_5500",sierg,isomer_tdiff);
					hismanager->Fill("ISOMER_5501",sierg,isomer_tdiff*1.0e-3);
					auto oldsi = prevsummary->GetEventObservable("SiMax").value_or(0.0);
					hismanager->Fill("ISOMER_5502",oldsi,isomer_tdiff);
					hismanager->Fill("ISOMER_5503",oldsi,isomer_tdiff*1.0e-3);
					hismanager->Fill("ISOMER_5504",oldsi+sierg,isomer_tdiff);
					hismanager->Fill("ISOMER_5505",oldsi+sierg,isomer_tdiff*1.0e-3);

					auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
					hismanager->Fill("ISOMER_3502",olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3503",olderg,isomer_tdiff*1.0e-3);

					hismanager->Fill("ISOMER_3504",erg+olderg,isomer_tdiff);
					hismanager->Fill("ISOMER_3505",erg+olderg,isomer_tdiff*1.0e-3);
					break;
				}
			}
		}
	}
}

void MtasIsomerProcessor::FillImplantPlots(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager,SimpleHPGeProcessor* hpgeproc,PSPMTProcessor* implantproc){
	//these do not care about muons
	auto numhist = eventhistory->GetMaxHistoryID();
	auto summary = eventhistory->GetCurrentEventSummary();
	bool hasimplant = summary->ContainsEventTag(this->implant);

	if( numhist > 1 ){
		//current event has gamma not-muon, not-beta
		if( not hasimplant ){
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
				auto previmplant = prevsummary->ContainsEventTag(this->implant);
				auto prevhpge = prevsummary->ContainsEventTag(this->hpge);
				//this looks for a beta decay into a delayed level
				//like 137Cs
				if( previmplant ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					for( size_t jj = 0; jj < hpgeproc->GetNumCrystals(); ++jj ){
						auto hpge_erg = hpgeproc->GetEnergy(jj);
						hismanager->Fill("ISOMER_4700",hpge_erg,isomer_tdiff);
						hismanager->Fill("ISOMER_4701",hpge_erg,isomer_tdiff*1.0e-3);
					}
					break;
				}
			}
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
				auto previmplant = prevsummary->ContainsEventTag(this->implant);
				auto prevhpge = prevsummary->ContainsEventTag(this->hpge);
				//looking for stepping through short isomer after we start in daughter isomer
				if( not previmplant ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();

					for( size_t jj = 0; jj < hpgeproc->GetNumCrystals(); ++jj ){
						auto hpge_erg = hpgeproc->GetEnergy(jj);
						hismanager->Fill("ISOMER_4600",hpge_erg,isomer_tdiff);
						hismanager->Fill("ISOMER_4601",hpge_erg,isomer_tdiff*1.0e-3);
					}
					break;
				}
			}
		}
		//current event does not have beta 
		if( hasimplant ){
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto previmplant = prevsummary->ContainsEventTag(this->implant);
				auto prevhpge = prevsummary->ContainsEventTag(this->hpge);

				//this looks for a gamma decay into a delayed beta
				//i.e. beam isomer, but need mtas energy for this old event
				if( not previmplant ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					for( size_t jj = 0; jj < hpgeproc->GetNumCrystals(); ++jj ){
						auto hpge_erg = hpgeproc->GetEnergy(jj);
						hismanager->Fill("ISOMER_4800",hpge_erg,isomer_tdiff);
						hismanager->Fill("ISOMER_4801",hpge_erg,isomer_tdiff*1.0e-3);
					}
					break;
				}
			}
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto previmplant = prevsummary->ContainsEventTag(this->implant);
				auto prevhpge = prevsummary->ContainsEventTag(this->hpge);

				//this looks for a gamma decay into a delayed beta
				//i.e. beam isomer, but need mtas energy for this old event
				if( previmplant ){
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					for( size_t jj = 0; jj < hpgeproc->GetNumCrystals(); ++jj ){
						auto hpge_erg = hpgeproc->GetEnergy(jj);
						hismanager->Fill("ISOMER_4500",hpge_erg,isomer_tdiff);
						hismanager->Fill("ISOMER_4501",hpge_erg,isomer_tdiff*1.0e-3);
					}
					break;
				}
			}
		}
	}
}

void MtasIsomerProcessor::SetInternalTDiff(double val){
	this->internaltdiff = val;
}
