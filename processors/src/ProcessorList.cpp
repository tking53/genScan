#include <random>
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

void ProcessorList::InitializeProcessors(ConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
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
		auto raw = evt.GetRawEnergy();
		auto erg = raw+this->randNum(this->randGen);
		try{
			const auto [cal,iraw,ical] = cmap->GetCalibratedEnergy(evt.GetCrate(),evt.GetModule(),evt.GetChannel(),erg,evt.GetRawTrace());
			evt.SetEnergy(erg,cal,iraw,ical);
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

	for( auto& evt : RawEvents ){
		auto gChanID = evt.GetGlobalChannelID();
		auto gBoardID = evt.GetGlobalBoardID();
		auto scalartime = 1.0e-9*(evt.GetTimeStamp()-this->FirstTimeStamp);
		int rate_y = scalartime/scalarsize;
		int rate_x = static_cast<int>(scalartime)%scalarsize;
		HistogramManager->Fill("Raw",evt.GetRawEnergyWRandom(),gChanID);
		HistogramManager->Fill("InternalRaw",evt.GetInternalFilterRaw(),gChanID);
		HistogramManager->Fill("Scalar",scalartime,gChanID);
		HistogramManager->Fill("Cal",evt.GetEnergy(),gChanID);
		HistogramManager->Fill("InternalCal",evt.GetInternalFilterEnergy(),gChanID);
		HistogramManager->Fill("Event_Mult",gChanID,evtsize);
		HistogramManager->Fill("Trace_Size",gChanID,evt.GetRawTrace().size());
		HistogramManager->Fill("Total_Rate",rate_x,rate_y);
		if( evt.GetPileup() ){
			HistogramManager->Fill("Total_Pileup",gBoardID,evt.GetChannel());
		}
		if( evt.GetSaturation() ){
			HistogramManager->Fill("Total_Saturate",gBoardID,evt.GetChannel());
		}
		HistogramManager->Fill("Total_Hits",gBoardID,evt.GetChannel());
		auto qdcs = evt.GetQDCSums();
		for( size_t ii = 0; ii < qdcs.size(); ++ii ){
			HistogramManager->Fill(this->QDCHisNames[ii],qdcs[ii],gChanID);
		}
	}
}

void ProcessorList::Finalize(){
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
