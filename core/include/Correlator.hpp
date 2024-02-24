#ifndef __CORRELATOR_HPP__
#define __CORRELATOR_HPP__

#include <functional>
#include <iomanip>
#include <tuple>
#include <set>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/heap/priority_queue.hpp>

class Correlator{
	public:
		struct AnalogTrigger{
			int Crate;
			int Module;
			int Channel;

			bool operator<(const Correlator::AnalogTrigger& rhs) const{
				return std::tie(this->Crate,this->Module,this->Channel) < std::tie(rhs.Crate,rhs.Module,rhs.Channel);
			}

			bool operator>(const Correlator::AnalogTrigger& rhs) const{
				return rhs < (*this);
			}

			bool operator<=(const Correlator::AnalogTrigger& rhs) const{
				return !((*this) > rhs);
			}

			bool operator>=(const Correlator::AnalogTrigger& rhs) const{
				return !((*this) < rhs);
			}

			bool operator==(const Correlator::AnalogTrigger& rhs) const{
				return (this->Crate == rhs.Crate) and (this->Module == rhs.Module) and (this->Channel == rhs.Channel);
			}

			bool operator!=(const Correlator::AnalogTrigger& rhs) const{
				return !((*this) == rhs);
			}
		};

		Correlator(const std::string&,const std::string&,double);
		virtual ~Correlator() = default;

		//switch based on the type of correlation we were constructed with
		//what's passed in is the timestamp (in ns), crate, module, channel of the last parsed event
		[[noreturn]] virtual bool IsWithinCorrelationWindow(const double&,const int&,const int&,const int&);

		virtual void AddTriggerChannel(const int&,const int&,const int&) final;

		[[noreturn]] virtual void Pop();
		[[noreturn]] virtual void Clear();

		[[noreturn]] virtual void DumpSelf() const;

	protected:
		std::string LogName;
		std::string CorrelatorName;
		std::shared_ptr<spdlog::logger> console;

		double Width;
		
		//Used for analog triggering scheme
		std::set<AnalogTrigger> Triggers;

		//used for rolling window triggering scheme
		boost::heap::priority_queue<double,boost::heap::compare<std::less<double>>> MaxHeap;
		#ifdef CORRELATOR_DEBUG
		boost::heap::priority_queue<double,boost::heap::compare<std::less<double>>> gMaxHeap;
		#endif

		//used for fixed window rolling trigger scheme
		boost::heap::priority_queue<double,boost::heap::compare<std::greater<double>>> MinHeap;
		#ifdef CORRELATOR_DEBUG
		boost::heap::priority_queue<double,boost::heap::compare<std::greater<double>>> gMinHeap;
		#endif
};

#endif
