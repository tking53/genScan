#include <random>
#include <stdexcept>
#include <sstream>

#include "ProcessorList.hpp"

#include "Processor.hpp"
#include "GenericProcessor.hpp"

#include "Analyzer.hpp"
#include "GenericAnalyzer.hpp"

ProcessorList::ProcessorList(const std::string& log){
	this->LogName = log;
	this->console = spdlog::get(this->LogName)->clone("ProcessorList");
	std::random_device rd;
	this->randGen = std::mt19937_64(rd());
	this->randNum = std::uniform_real_distribution<double>(0.0,1.0);
	this->FirstTimeStamp = -1;
	this->EventStamp = 0;
}

ProcessorList::~ProcessorList() = default;

void ProcessorList::PreAnalyze(PLOTS::PlotRegistry* HistogramManager){
	for( auto& anal : this->known_analyzers ){
		anal->PreProcess();
	}
}

void ProcessorList::Analyze(PLOTS::PlotRegistry* HistogramManager){
	for( auto& anal : this->known_analyzers ){
		anal->Process();
	}
}

void ProcessorList::PostAnalyze(PLOTS::PlotRegistry* HistogramManager){
	for( auto& anal : this->known_analyzers ){
		anal->PostProcess();
	}
}

void ProcessorList::PreProcess(PLOTS::PlotRegistry* HistogramManager){
	for( auto& proc : this->known_processors ){
		proc->PreProcess();
	}
}

void ProcessorList::Process(PLOTS::PlotRegistry* HistogramManager){
	for( auto& proc : this->known_processors ){
		proc->Process();
	}
}

void ProcessorList::PostProcess(PLOTS::PlotRegistry* HistogramManager){
	for( auto& proc : this->known_processors ){
		proc->PostProcess();
	}
	++(this->EventStamp);
}

void ProcessorList::InitializeProcessors(XMLConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			known_processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		known_processors.back()->Init(cmap->GetProcessorXMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(XMLConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			known_analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		known_analyzers.back()->Init(cmap->GetAnalyzerXMLInfo(name));
	}
}

void ProcessorList::InitializeProcessors(YAMLConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			known_processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		known_processors.back()->Init(cmap->GetProcessorYAMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(YAMLConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			known_analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		known_analyzers.back()->Init(cmap->GetAnalyzerYAMLInfo(name));
	}
}
void ProcessorList::InitializeProcessors(JSONConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			known_processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		known_processors.back()->Init(cmap->GetProcessorJSONInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(JSONConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			known_analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		known_analyzers.back()->Init(cmap->GetAnalyzerJSONInfo(name));
	}
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
		auto cal = cmap->GetCalibratedEnergy(evt.GetCrate(),evt.GetModule(),evt.GetChannel(),erg);
		evt.SetEnergy(cal);
		cmap->SetChanConfigInfo(evt);
		#ifdef PROCESSOR_DEBUG
		#ifndef NDEBUG
		this->console->info("raw : {}, rand : {}, cal : {}, cr : {}, mod : {} chan : {}, gchan : {}",raw,erg,cal,evt.GetCrate(),evt.GetModule(),evt.GetChannel(),evt.GetGlobalChannelID());
		#endif
		#endif
	}
}

void ProcessorList::ProcessRaw(boost::container::devector<PhysicsData>& RawEvents,PLOTS::PlotRegistry* HistogramManager){
	auto evtsize = RawEvents.size();
	double deltats = 0.0;
	auto scalarsize = HistogramManager->GetScalarBins();

	if( evtsize > 1 ){
		deltats = RawEvents.back().GetTimeStamp()-RawEvents.front().GetTimeStamp();
	}
	
	HistogramManager->Fill("Event_Size",evtsize);
	HistogramManager->Fill("Event_Width",deltats);
	HistogramManager->Fill("Event_Scale",deltats,evtsize);

	for( auto& evt : RawEvents ){
		auto gChanID = evt.GetGlobalChannelID();
		auto scalartime = 1.0e-9*(evt.GetTimeStamp()-this->FirstTimeStamp);
		int rate_y = scalartime/scalarsize;
		int rate_x = static_cast<int>(scalartime)%scalarsize;
		HistogramManager->Fill("Raw",evt.GetRawEnergy(),gChanID);
		HistogramManager->Fill("Scalar",scalartime,gChanID);
		HistogramManager->Fill("Cal",evt.GetEnergy(),gChanID);
		HistogramManager->Fill("Event_Mult",gChanID,evtsize);
		HistogramManager->Fill("Total_Rate",rate_x,rate_y);
	}
}
