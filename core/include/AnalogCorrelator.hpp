#ifndef __ANALOG_CORRELATOR_HPP__
#define __ANALOG_CORRELATOR_HPP__

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/heap/priority_queue.hpp>

#include "Correlator.hpp"

class AnalogCorrelator : public Correlator{
	public:
		AnalogCorrelator(const std::string&,double);
		~AnalogCorrelator() = default;

		//switch based on the type of correlation we were constructed with
		//what's passed in is the timestamp (in ns), crate, module, channel of the last parsed event
		virtual bool IsWithinCorrelationWindow(const double&,const int&,const int&,const int&) final;
		
		virtual void DumpSelf() const final;

		template<typename OStream>
		friend OStream& operator<<(OStream& os,const AnalogCorrelator& corr){
			os << "AnalogCorrelator( Window: " << corr.Width << " ns";
			os << "\nKnown Analog Triggers: ( Crate | Module | Channel )";
			size_t ii = 0;
			for( const auto& trig : corr.Triggers ){
				os << "\nTrigger " << ii << " : ( " 
					<< trig.Crate << " | " 
					<< trig.Module << " | "
					<< trig.Channel << " ) ";
				++ii;
			}
			os << "\nEnd Known Analog Triggers";
			return os;
		}
};

#endif
