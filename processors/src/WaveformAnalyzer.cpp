#include "WaveformAnalyzer.hpp"
#include "boost/regex/v5/regex_iterator.hpp"

WaveformAnalyzer::WaveformAnalyzer(const std::string& log) : Analyzer(log,"WaveformAnalyzer",{}){
}

[[maybe_unused]] bool WaveformAnalyzer::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Analyzer::PreProcess();

	for( const auto& key : this->Types ){
		summary.GetDetectorSummary(this->AllDefaultRegex[key],this->SummaryData);
		//this->console->info("ROOTDEV Size for type {} : {}",key,this->SummaryData.size());
		for( const auto& evt : this->SummaryData ){
			//this->console->info("{} {}",evt->GetType(),this->SummaryData.size());
		}
	}
	Analyzer::EndProcess();
	return true;
}

[[maybe_unused]] bool WaveformAnalyzer::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool WaveformAnalyzer::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
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

	for(pugi::xml_node settings = config.child("settings"); settings; settings = settings.next_sibling("settings")){
		auto re = this->GenerateRegex(settings.attribute("Crate").as_string("[\\d]"),settings.attribute("Module").as_string("[\\d]"),settings.attribute("Channel").as_string("[\\d]"));
		this->console->info("Found settings node regex:{}",re.str());

		this->WaveSettings.push_back(std::make_pair(re,WaveFormParams()));
		this->ParsePreTrigger(settings,this->WaveSettings.back().second);
		this->ParsePostTrigger(settings,this->WaveSettings.back().second);
		this->ParseQDC(settings,this->WaveSettings.back().second);
		this->ParsePSD(settings,this->WaveSettings.back().second);

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


void WaveformAnalyzer::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) const{
	console->info("Finished Declaring Plots");
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
			this->console->info("Found QDC node : NumSums -> {}, BoundsString -> {})",wav.QDCBounds.size()-1,vals);
		}
	}else{
		this->console->error("missing QDC node in settings node {}",settings);
		throw "missing QDC node";
	}
}

void WaveformAnalyzer::ParsePSD(const pugi::xml_node& settings,WaveFormParams& wav){
	if( pugi::xml_node curr = settings.child("PSD") ){
		wav.HasPSD = true;
		wav.PSDBounds = std::make_tuple<size_t,size_t,size_t,float>(curr.attribute("begin").as_int(0),curr.attribute("middle").as_int(0),curr.attribute("end").as_int(0),curr.attribute("fraction").as_float(1.0));
		this->console->info("Found PSD node : [begin,middle,end,fraction)",std::get<0>(wav.PSDBounds),std::get<1>(wav.PSDBounds),std::get<2>(wav.PSDBounds),std::get<3>(wav.PSDBounds));

	}
}
