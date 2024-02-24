#ifndef __ROLLINGWINDOW_CORRELATOR_HPP__
#define __ROLLINGWINDOW_CORRELATOR_HPP__

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/heap/priority_queue.hpp>

#include "Correlator.hpp"

class RollingWindowCorrelator : public Correlator{
	public:
		RollingWindowCorrelator(const std::string&,double);
		~RollingWindowCorrelator() = default;

		//switch based on the type of correlation we were constructed with
		//what's passed in is the timestamp (in ns), crate, module, channel of the last parsed event
		virtual bool IsWithinCorrelationWindow(const double&,const int&,const int&,const int&) final;
		
		virtual void DumpSelf() const final;

		virtual void Pop() final;
		virtual void Clear() final;

		template<typename OStream>
		friend OStream& operator<<(OStream& os,const RollingWindowCorrelator& corr){
			os << "RollingWindowCorrelator( Window: " << corr.Width << " ns";
			return os;
		}

};

#endif
