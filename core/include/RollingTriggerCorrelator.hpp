#ifndef __ROLLINGTRIGGER_CORRELATOR_HPP__
#define __ROLLINGTRIGGER_CORRELATOR_HPP__

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/heap/priority_queue.hpp>

#include "Correlator.hpp"

class RollingTriggerCorrelator : public Correlator{
	public:
		RollingTriggerCorrelator(const std::string&,double);
		~RollingTriggerCorrelator() = default;

		//switch based on the type of correlation we were constructed with
		//what's passed in is the timestamp (in ns), crate, module, channel of the last parsed event
		virtual bool IsWithinCorrelationWindow(const double&,const int&,const int&,const int&);
		
		virtual void DumpSelf() const final;

		virtual void Pop() final;
		virtual void Clear() final;

		template<typename OStream>
		friend OStream& operator<<(OStream& os,const RollingTriggerCorrelator& corr){
			os << "RollingWindowCorrelator( Window: " << corr.Width << " ns";
			return os;
		}

};

#endif
