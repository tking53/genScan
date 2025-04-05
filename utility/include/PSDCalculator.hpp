#ifndef __PSD_CALCULATOR_HPP__
#define __PSD_CALCULATOR_HPP__

#include <stdexcept>
#include <vector>
#include <utility>

#include <pugixml.hpp>

#include <boost/regex.hpp>

struct PSDCalculator{
	std::pair<size_t,size_t> PreTriggerBounds;
	std::pair<size_t,size_t> PostTriggerBounds;
	std::vector<size_t> QDCBounds;

	bool HasPSD;
	std::tuple<size_t,size_t,size_t> FixedPSDBounds;
	std::tuple<size_t,size_t,float> FractionalPSDBounds;

	bool CalcDerivative;

	PSDCalculator(const pugi::xml_node& settings){
		this->CalcDerivative = settings.attribute("CalcDerivative").as_bool(false);

		if( pugi::xml_node curr = settings.child("PreTrigger") ){
			this->PreTriggerBounds = std::make_pair<size_t,size_t>(curr.attribute("min").as_int(0),curr.attribute("max").as_int(0));
		}else{
			throw std::runtime_error("missing PreTrigger node");
		}

		if( pugi::xml_node curr = settings.child("PostTrigger") ){
			this->PostTriggerBounds = std::make_pair<size_t,size_t>(curr.attribute("min").as_int(0),curr.attribute("max").as_int(0));
		}else{
			throw std::runtime_error("missing PostTrigger node");
		}
		if( pugi::xml_node curr = settings.child("QDC") ){
			std::string vals = curr.attribute("bounds").as_string("");
			if( vals.empty() ){
				throw std::runtime_error("missing bounds attribute in QDC node");
			}else{
				boost::regex digit("\\d{1,}");
				boost::sregex_iterator iter(vals.begin(),vals.end(),digit);
				boost::sregex_iterator end;
				for(; iter != end; ++iter){
					this->QDCBounds.push_back(static_cast<size_t>(std::stoi(iter->str())));
				}
				if( this->QDCBounds.size() < 2 ){
					throw std::runtime_error("need at least two value for the QDC bounds attribute");
				}
			}
		}else{
			throw std::runtime_error("missing QDC node");
		}

		if( pugi::xml_node curr = settings.child("PSD") ){
			this->HasPSD = true;
			this->FixedPSDBounds = std::make_tuple<size_t,size_t,size_t>(curr.attribute("begin").as_int(0),curr.attribute("middle").as_int(0),curr.attribute("end").as_int(0));
			this->FractionalPSDBounds = std::make_tuple<size_t,size_t,float>(curr.attribute("pre").as_int(0),curr.attribute("post").as_int(0),curr.attribute("fraction").as_float(2.0));
		}

	}

	~PSDCalculator() = default;

	PSDCalculator(const PSDCalculator&) = default;
	PSDCalculator(PSDCalculator&&) = default;
	PSDCalculator& operator=(const PSDCalculator&) = default;
	PSDCalculator& operator=(PSDCalculator&&) = default;

};

#endif
