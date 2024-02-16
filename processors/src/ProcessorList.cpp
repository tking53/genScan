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
}

ProcessorList::~ProcessorList() = default;

void ProcessorList::PreAnalyze(){
	for( auto& anal : this->analyzers )
		anal->PreProcess();
}

void ProcessorList::Analyze(){
	for( auto& anal : this->analyzers )
		anal->Process();
}

void ProcessorList::PostAnalyze(){
	for( auto& anal : this->analyzers )
		anal->PostProcess();
}

void ProcessorList::PreProcess(){
	for( auto& proc : this->processors )
		proc->PreProcess();
}

void ProcessorList::Process(){
	for( auto& proc : this->processors )
		proc->Process();
}

void ProcessorList::PostProcess(){
	for( auto& proc : this->processors )
		proc->PostProcess();
}

void ProcessorList::InitializeProcessors(XMLConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		processors.back()->Init(cmap->GetProcessorXMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(XMLConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		analyzers.back()->Init(cmap->GetAnalyzerXMLInfo(name));
	}
}

void ProcessorList::InitializeProcessors(YAMLConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		processors.back()->Init(cmap->GetProcessorYAMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(YAMLConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		analyzers.back()->Init(cmap->GetAnalyzerYAMLInfo(name));
	}
}
void ProcessorList::InitializeProcessors(JSONConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		processors.back()->Init(cmap->GetProcessorJSONInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(JSONConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		analyzers.back()->Init(cmap->GetAnalyzerJSONInfo(name));
	}
}

void ProcessorList::RegisterOutputTrees(RootFileManager* rootnamager){
	for( auto& proc : this->processors )
		rootnamager->RegisterProcessor(proc.get());
}

void ProcessorList::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	for( auto& proc : this->processors )
		proc->DeclarePlots(hismanager);
	for( auto& anal : this->analyzers )
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
		#ifdef PROCESSOR_DEBUG
		this->console->info("raw : {}, rand : {}, cal : {}, cr : {}, mod : {} chan : {}, gchan : {}",raw,erg,cal,evt.GetCrate(),evt.GetModule(),evt.GetChannel(),evt.GetGlobalChannelID());
		#endif
	}
}

void ProcessorList::ProcessRaw(boost::container::devector<PhysicsData>& RawEvents,PLOTS::PlotRegistry* HistogramManager){
	auto evtsize = RawEvents.size();
	double deltats = 0.0;

	if( evtsize > 1 ){
		deltats = RawEvents.back().GetTimeStamp()-RawEvents.front().GetTimeStamp();
	}
	
	HistogramManager->Fill("Event_Size",evtsize);
	HistogramManager->Fill("Event_Width",deltats);
	HistogramManager->Fill("Event_Scale",deltats,evtsize);

	for( auto& evt : RawEvents ){
		auto gChanID = evt.GetGlobalChannelID();
		HistogramManager->Fill("Raw",evt.GetRawEnergy(),gChanID);
		HistogramManager->Fill("Scalar",1.0e-9*(evt.GetTimeStamp()-this->FirstTimeStamp),gChanID);
		HistogramManager->Fill("Cal",evt.GetEnergy(),gChanID);
		HistogramManager->Fill("Event_Mult",gChanID,evtsize);
	}
}
