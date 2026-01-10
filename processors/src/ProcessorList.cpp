#include <random>
#include <set>
#include <stdexcept>
#include <sstream>

#include "ProcessorList.hpp"

#include "EventSummary.hpp"

#include "GenericProcessor.hpp"
#include "GenericAnalyzer.hpp"

#include "anl2021Processor.hpp"
#include "BSMExpProcessor.hpp"
#include "e21069b_fp2Processor.hpp"
#include "e21027Processor.hpp"
#include "KClComptonProcessor.hpp"
#include "ribf168Processor.hpp"
#include "YAPProcessor.hpp"

#include "BSMProcessor.hpp"
#include "HagridProcessor.hpp"
#include "IonizationChamberProcessor.hpp"
#include "MtasProcessor.hpp"
#include "MtasImplantProcessor.hpp"
#include "MtasSSDProcessor.hpp"
#include "MtasTapeProcessor.hpp"
#include "PidProcessor.hpp"
#include "PSPMTProcessor.hpp"
#include "PuckProcessor.hpp"
#include "RIKENIonizationChamberProcessor.hpp"
#include "RIKENPidProcessor.hpp"
#include "RootDevProcessor.hpp"
#include "SimpleHPGeProcessor.hpp"
#include "VetoProcessor.hpp"

#include "WaveformAnalyzer.hpp"

ProcessorList::ProcessorList(const std::string& log){
	this->LogName = log;
	this->console = spdlog::get(this->LogName)->clone("ProcessorList");
	std::random_device rd;
	this->randGen = std::mt19937_64(rd());
	this->randNum = std::uniform_real_distribution<double>(0.0,1.0);
	this->FirstTimeStamp = -1;
	this->EventStamp = 0;
	this->QDCHisNames = {"QDC_0","QDC_1","QDC_2","QDC_3","QDC_4","QDC_5","QDC_6","QDC_7"};
}

void ProcessorList::PreAnalyze(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager,CUTS::CutRegistry* CutManager){
	auto knowntypes = History->GetCurrentEventSummary()->GetKnownTypes();
	for( auto& anal : this->known_analyzers ){
		if( anal->ContainsAnyType(knowntypes) ){
			anal->PreProcess(History,HistogramManager,CutManager);
		}
	}
}

void ProcessorList::Analyze(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager,CUTS::CutRegistry* CutManager){
	auto knowntypes = History->GetCurrentEventSummary()->GetKnownTypes();
	for( auto& anal : this->known_analyzers ){
		if( anal->ContainsAnyType(knowntypes) ){
			anal->Process(History,HistogramManager,CutManager);
		}
	}
}

void ProcessorList::PostAnalyze(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager,CUTS::CutRegistry* CutManager){
	auto knowntypes = History->GetCurrentEventSummary()->GetKnownTypes();
	for( auto& anal : this->known_analyzers ){
		if( anal->ContainsAnyType(knowntypes) ){
			anal->PostProcess(History,HistogramManager,CutManager);
		}
	}
}

void ProcessorList::PreProcess(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager,CUTS::CutRegistry* CutManager){
	auto knowntypes = History->GetCurrentEventSummary()->GetKnownTypes();
	for( auto& proc : this->known_processors ){
		if( proc->ContainsAnyType(knowntypes) ){
			proc->PreProcess(History,HistogramManager,CutManager);
		}
	}
}

void ProcessorList::Process(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager,CUTS::CutRegistry* CutManager){
	auto knowntypes = History->GetCurrentEventSummary()->GetKnownTypes();
	for( auto& proc : this->known_processors ){
		if( proc->ContainsAnyType(knowntypes) ){
			proc->Process(History,HistogramManager,CutManager);
		}
	}
}

void ProcessorList::PostProcess(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager,CUTS::CutRegistry* CutManager){
	auto knowntypes = History->GetCurrentEventSummary()->GetKnownTypes();
	for( auto& proc : this->known_processors ){
		if( proc->ContainsAnyType(knowntypes) ){
			proc->PostProcess(History,HistogramManager,CutManager);
		}
	}
	++(this->EventStamp);
}

void ProcessorList::CreateProc(const std::string& name){
	if( name.compare("GenericProcessor") == 0 ){
		this->known_processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
	}else if( name.compare("anl2021Processor") == 0 ){
		known_processors.push_back(std::make_shared<anl2021Processor>(this->LogName));
	}else if( name.compare("BSMExpProcessor") == 0 ){
		known_processors.push_back(std::make_shared<BSMExpProcessor>(this->LogName));
	}else if( name.compare("e21027Processor") == 0 ){
		known_processors.push_back(std::make_shared<e21027Processor>(this->LogName));
	}else if( name.compare("e21069b_fp2Processor") == 0 ){
		known_processors.push_back(std::make_shared<e21069b_fp2Processor>(this->LogName));
	}else if( name.compare("KClComptonProcessor") == 0 ){
		known_processors.push_back(std::make_shared<KClComptonProcessor>(this->LogName));
	}else if( name.compare("ribf168Processor") == 0 ){
		known_processors.push_back(std::make_shared<ribf168Processor>(this->LogName));
	}else if( name.compare("BSMProcessor") == 0 ){
		known_processors.push_back(std::make_shared<BSMProcessor>(this->LogName));
	}else if( name.compare("HagridProcessor") == 0 ){
		known_processors.push_back(std::make_shared<HagridProcessor>(this->LogName));
	}else if( name.compare("IonizationChamberProcessor") == 0 ){
		known_processors.push_back(std::make_shared<IonizationChamberProcessor>(this->LogName));
	}else if( name.compare("MtasProcessor") == 0 ){
		known_processors.push_back(std::make_shared<MtasProcessor>(this->LogName));
	}else if( name.compare("MtasImplantProcessor") == 0 ){
		known_processors.push_back(std::make_shared<MtasImplantProcessor>(this->LogName));
	}else if( name.compare("MtasSSDProcessor") == 0 ){
		known_processors.push_back(std::make_shared<MtasSSDProcessor>(this->LogName));
	}else if( name.compare("MtasTapeProcessor") == 0 ){
		known_processors.push_back(std::make_shared<MtasTapeProcessor>(this->LogName));
	}else if( name.compare("PidProcessor") == 0 ){
		known_processors.push_back(std::make_shared<PidProcessor>(this->LogName));
	}else if( name.compare("PSPMTProcessor") == 0 ){
		known_processors.push_back(std::make_shared<PSPMTProcessor>(this->LogName));
	}else if( name.compare("PuckProcessor") == 0 ){
		known_processors.push_back(std::make_shared<PuckProcessor>(this->LogName));
	}else if( name.compare("RIKENIonizationChamberProcessor") == 0 ){
		known_processors.push_back(std::make_shared<RIKENIonizationChamberProcessor>(this->LogName));
	}else if( name.compare("RIKENPidProcessor") == 0 ){
		known_processors.push_back(std::make_shared<RIKENPidProcessor>(this->LogName));
	}else if( name.compare("RootDevProcessor") == 0 ){
		known_processors.push_back(std::make_shared<RootDevProcessor>(this->LogName));
	}else if( name.compare("SimpleHPGeProcessor") == 0 ){
		known_processors.push_back(std::make_shared<SimpleHPGeProcessor>(this->LogName));
	}else if( name.compare("VetoProcessor") == 0 ){
		known_processors.push_back(std::make_shared<VetoProcessor>(this->LogName));
	}else if( name.compare("YAPProcessor") == 0 ){
		known_processors.push_back(std::make_shared<YAPProcessor>(this->LogName));
	}else{
		std::stringstream ss;
		ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			<< name 
			<< "\"";
		throw std::runtime_error(ss.str());
	}

}

void ProcessorList::CreateAnal(const std::string& name){
	if( name.compare("GenericAnalyzer") == 0 ){
		known_analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
	}else if( name.compare("WaveformAnalyzer") == 0 ){
		known_analyzers.push_back(std::make_shared<WaveformAnalyzer>(this->LogName));
	}else{
		std::stringstream ss;
		ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			<< name 
			<< "\"";
		throw std::runtime_error(ss.str());
	}
}

void ProcessorList::InitializeProcessors(ConfigParser* cmap,bool enabletree){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( not enabletree and name.compare("RootDevProcessor") == 0 ){
			this->console->critical("tree output is disabled, but RootDevProcessor is declared, skipping it");
			continue;
		}
		this->CreateProc(name);
		known_processors.back()->Init(cmap->GetProcessorXMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(ConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		this->CreateAnal(name);
		known_analyzers.back()->Init(cmap->GetAnalyzerXMLInfo(name));
	}
}

void ProcessorList::RegisterCuts(CUTS::CutRegistry* CutManager){
	for( auto& proc : this->known_processors )
		proc->RegisterCuts(CutManager);
}

void ProcessorList::RegisterOutputTrees(RootFileManager* rootnamager){
	for( auto& proc : this->known_processors )
		rootnamager->RegisterProcessor(proc.get());
}

void ProcessorList::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	for( auto& proc : this->known_processors )
		proc->DeclarePlots(hismanager);
	for( auto& anal : this->known_analyzers )
		anal->DeclarePlots(hismanager);
}

void ProcessorList::ThreshAndCal(boost::container::devector<PhysicsData>& RawEvents,ChannelMap* cmap){
	if( this->FirstTimeStamp < 0 ){
		this->FirstTimeStamp = RawEvents.front().GetTimeStamp();
	}
	
	for( auto& evt : RawEvents ){
		auto alias = this->randNum(this->randGen);
		evt.SetAliasValue(alias);
		auto raw = evt.GetRawEnergy();
		auto erg = raw+alias;
		try{
			const auto [cal,iraw,ical,iiraw,iical] = cmap->GetCalibratedEnergy(evt.GetCrate(),evt.GetModule(),evt.GetChannel(),erg,alias,evt.GetRawTrace());
			evt.SetFilterEnergy(erg,cal);
			evt.SetInternalFilterEnergy(iraw,ical);
			evt.SetInternalIntegralEnergy(iiraw,iical);
		}catch(const boost::container::out_of_range& e){
			this->console->error("Invalid channel map for event {}. Next message is description from boost",evt);
			throw std::runtime_error(e.what());
		}

		try{
			cmap->SetChanConfigInfo(evt);
		}catch(const boost::container::out_of_range& e){
			this->console->error("Invalid channel map for event {}. Next message is description from boost",evt);
			throw std::runtime_error(e.what());
		}
		#ifdef PROCESSOR_DEBUG
		#ifndef NDEBUG
		this->console->info("raw : {}, rand : {}, cal : {}, cr : {}, mod : {} chan : {}, gchan : {}",raw,erg,cal,evt.GetCrate(),evt.GetModule(),evt.GetChannel(),evt.GetGlobalChannelID());
		#endif
		#endif
	}
}

void ProcessorList::ProcessRaw(EventHistoryManager* History,PLOTS::PlotRegistry* HistogramManager){
	auto RawEvents = History->GetCurrentEventSummary()->GetRawEvents();
	auto evtsize = RawEvents.size();
	double deltats = 0.0;
	double historyts = 0.0;
	double historydelta = 0.0;
	double historyspacing = 0.0;
	auto scalarsize = HistogramManager->GetScalarBins();

	if( evtsize > 1 ){
		deltats = RawEvents.back().GetTimeStamp()-RawEvents.front().GetTimeStamp();
	}
	auto evtcnt = History->GetEventCount();
	//got the first event ever in the scan, use it for all offsets
	if( evtcnt == 1 ){
		History->SetVeryFirstTime(RawEvents.front().GetTimeStamp());
	}

	if(evtcnt > 1 ){
		auto OldEvents = History->GetOldestEventSummary()->GetRawEvents();
		historyts = RawEvents.front().GetTimeStamp() - OldEvents.front().GetTimeStamp();

		auto PrevEvent = History->GetPreviousEventSummary(1)->GetRawEvents();
		historydelta = RawEvents.front().GetTimeStamp() - PrevEvent.front().GetTimeStamp();
		historyspacing = RawEvents.front().GetTimeStamp() - PrevEvent.back().GetTimeStamp();
	}	
	
	HistogramManager->Fill("Event_Size",evtsize);
	HistogramManager->Fill("Event_Width",deltats);
	HistogramManager->Fill("Event_Scale",deltats,evtsize);
	HistogramManager->Fill("Event_Delta",historydelta);
	HistogramManager->Fill("Event_Spacing",historyspacing);
	HistogramManager->Fill("History_Width",historyts*1.0e-3);

	auto Raw = HistogramManager->GetPlot<TH2*>("Raw");
	auto InternalRaw = HistogramManager->GetPlot<TH2*>("InternalRaw");
	auto IntegralRaw = HistogramManager->GetPlot<TH2*>("IntegralRaw");

	auto Scalar = HistogramManager->GetPlot<TH2*>("Scalar");
	auto Scalar_M = HistogramManager->GetPlot<TH2*>("Scalar_M");
	auto Scalar_5M = HistogramManager->GetPlot<TH2*>("Scalar_5M");

	auto Cal = HistogramManager->GetPlot<TH2*>("Cal");
	auto InternalCal = HistogramManager->GetPlot<TH2*>("InternalCal");
	auto IntegralCal = HistogramManager->GetPlot<TH2*>("IntegralCal");

	auto Event_Mult = HistogramManager->GetPlot<TH2*>("Event_Mult");

	auto Trace_Size = HistogramManager->GetPlot<TH2*>("Trace_Size");

	auto Total_Rate = HistogramManager->GetPlot<TH2*>("Total_Rate");
	auto Total_Rate_M = HistogramManager->GetPlot<TH2*>("Total_Rate_M");
	auto Total_Rate_5M = HistogramManager->GetPlot<TH2*>("Total_Rate_5M");

	auto Total_Pileup = HistogramManager->GetPlot<TH2*>("Total_Pileup");

	auto Total_Saturate = HistogramManager->GetPlot<TH2*>("Total_Saturate");

	auto Total_Hits = HistogramManager->GetPlot<TH2*>("Total_Hits");

	std::vector<TH2*> QDCs = {
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[0]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[1]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[2]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[3]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[4]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[5]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[6]),
		HistogramManager->GetPlot<TH2*>(this->QDCHisNames[7])
	};

	for( const auto& evt : RawEvents ){
		auto gChanID = evt.GetGlobalChannelID();
		auto gBoardID = evt.GetGlobalBoardID();

		auto scalartime = 1.0e-9*(evt.GetTimeStamp()-this->FirstTimeStamp);
		auto scalartime_m = scalartime/60.0;
		auto scalartime_5m = scalartime_m/5.0;

		int rate_y = scalartime/scalarsize;
		int rate_x = static_cast<int>(scalartime)%scalarsize;
		int rate_m_y = scalartime_m/scalarsize;
		int rate_m_x = static_cast<int>(scalartime_m)%scalarsize;
		int rate_5m_y = scalartime_5m/scalarsize;
		int rate_5m_x = static_cast<int>(scalartime_5m)%scalarsize;

		Raw->Fill(evt.GetRawEnergyWRandom(),gChanID);
		InternalRaw->Fill(evt.GetInternalFilterRaw(),gChanID);
		IntegralRaw->Fill(evt.GetInternalIntegralRaw(),gChanID);

		Scalar->Fill(scalartime,gChanID);
		Scalar_M->Fill(scalartime_m,gChanID);
		Scalar_5M->Fill(scalartime_5m,gChanID);

		Cal->Fill(evt.GetEnergy(),gChanID);
		InternalCal->Fill(evt.GetInternalFilterEnergy(),gChanID);
		IntegralCal->Fill(evt.GetInternalIntegralEnergy(),gChanID);

		Event_Mult->Fill(gChanID,evtsize);

		Trace_Size->Fill(gChanID,evt.GetRawTrace().size());

		Total_Rate->Fill(rate_x,rate_y);
		Total_Rate_M->Fill(rate_m_x,rate_m_y);
		Total_Rate_5M->Fill(rate_5m_x,rate_5m_y);

		if( evt.GetPileup() ){
			Total_Pileup->Fill(gBoardID,evt.GetChannel());
		}

		if( evt.GetSaturation() ){
			Total_Saturate->Fill(gBoardID,evt.GetChannel());
		}

		Total_Hits->Fill(gBoardID,evt.GetChannel());

		auto qdcs = evt.GetQDCSums();
		for( size_t ii = 0; ii < qdcs.size(); ++ii ){
			QDCs[ii]->Fill(qdcs[ii],gChanID);
		}
	}
}

void ProcessorList::Finalize(){
	std::set<std::string> names;
	for( const auto& proc : this->known_processors ){
		names.insert(proc->GetProcessorName());
	}
	bool HasRootDev = names.find("RootDevProcessor") != names.end();
	bool EXPMode = true;
	if( this->known_processors.size() == 2 and HasRootDev ){
		this->console->critical("Found RootDevProcessor and one other, staying in experiment processor mode");
	}else if( this->known_processors.size() != 1 ){
		EXPMode = false;
		this->console->critical("Found Multiple Processors other than RootDev, disabling experiment processor mode");
	}else{
		this->console->critical("Found exactly 1 processor, staying in experiment processor mode");
		EXPMode = true;
	}
	if( not EXPMode ){
		this->console->critical("Not in experiment processor mode, notifying all processors declared");
		for( auto& proc : this->known_processors ){
			proc->ToggleExpProcessorMode();
		}
	}
	for( auto& anal : this->known_analyzers ){
		anal->Finalize();
	}
	for( auto& proc : this->known_processors ){
		proc->Finalize();
	}
}

void ProcessorList::CleanupTrees(){
	for( auto& proc : this->known_processors )
		proc->CleanupTree();
}

const std::vector<std::shared_ptr<Processor>>& ProcessorList::GetProcessors() const{
	return this->known_processors;
}

const std::vector<std::shared_ptr<Analyzer>>& ProcessorList::GetAnalyzers() const{
	return this->known_analyzers;
}
