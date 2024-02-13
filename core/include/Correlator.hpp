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
		enum CORRELATIONWINDOWTYPE{
			ROLLINGWIDTH,
			ROLLINGTRIGGER,
			ANALOG,
			UNKNOWN
		};

		struct AnalogTrigger{
			int Crate;
			int Module;
			int Channel;

			bool operator<(const AnalogTrigger& rhs) const{
				return std::tie(this->Crate,this->Module,this->Channel) < std::tie(rhs.Crate,rhs.Module,rhs.Channel);
			}

			bool operator>(const Correlator::AnalogTrigger& rhs) const{
				return rhs < (*this);
			}

			bool operator<=(const AnalogTrigger& rhs) const{
				return !((*this) > rhs);
			}

			bool operator>=(const AnalogTrigger& rhs) const{
				return !((*this) < rhs);
			}

			bool operator==(const AnalogTrigger& rhs) const{
				return (this->Crate == rhs.Crate) and (this->Module == rhs.Module) and (this->Channel == rhs.Channel);
			}

			bool operator!=(const AnalogTrigger& rhs) const{
				return !((*this) == rhs);
			}
		};

		Correlator(const std::string&,double);
		~Correlator() = default;

		void SetCorrelationType(CORRELATIONWINDOWTYPE);

		//switch based on the type of correlation we were constructed with
		//what's passed in is the timestamp (in ns), crate, module, channel of the last parsed event
		bool IsWithinCorrelationWindow(const double&,const int&,const int&,const int&);

		void AddTriggerChannel(const int&,const int&,const int&);

		void Pop();
		void Clear();

		void DumpSelf() const;

		template<typename OStream>
		friend OStream& operator<<(OStream& os,const Correlator& corr){
			os << "Correlator( Window: " << corr.Width << " ns";
			switch (corr.CorrelationType){
				case Correlator::CORRELATIONWINDOWTYPE::ANALOG:
					os << " Type: ANALOG )";
					break;
				case Correlator::CORRELATIONWINDOWTYPE::ROLLINGTRIGGER:
					os << " Type: ROLLINGTRIGGER )";
					break;
				case Correlator::CORRELATIONWINDOWTYPE::ROLLINGWIDTH:
					os << " Type: ROLLINGWIDTH )";
					break;
				default:
					os << " Type: UNKNOWN )";
					break;
			}
			if( corr.CorrelationType == Correlator::CORRELATIONWINDOWTYPE::ANALOG ){
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
			}else{
				#ifdef CORRELATOR_DEBUG
				if( corr.gMaxHeap.size() > 0 and corr.gMinHeap.size() > 0 ){
					os << "\nRange of TimeStamps: [ " << std::fixed << std::setprecision(1) << corr.gMinHeap.top() << " , " << corr.gMaxHeap.top() << " ]";
					os << "\nglobal Min Heap TimeStamps : [ ";
					for( const auto& ts : corr.gMinHeap ){
						os << '\n' << std::fixed << std::setprecision(1) << ts;
					}
					os << "\n]";
					os << "\nglobal Max Heap TimeStamps : [ ";
					for( const auto& ts : corr.gMaxHeap ){
						os << '\n' << std::fixed << std::setprecision(1) << ts;
					}
					os << "\n]";
				}
				#endif
				if( corr.MinHeap.size() > 0 ){
					os << "\nMin Heap TimeStamps : [ ";
					for( const auto& ts : corr.MinHeap ){
						os << '\n' << std::fixed << std::setprecision(1) << ts;
					}
					os << "\n]";
				}
				if( corr.MaxHeap.size() > 0 ){
					os << "\nMax Heap TimeStamps : [ ";
					for( const auto& ts : corr.MaxHeap ){
						os << '\n' << std::fixed << std::setprecision(1) << ts;
					}
					os << "\n]";
				}
			}

			return os;
		}
	private:
		std::string LogName;
		std::string CorrelatorName;
		std::shared_ptr<spdlog::logger> console;

		//just check to see if it's one of the channels we reset the correlation on
		//False -> was a trigger channel
		//True -> not a trigger channel
		bool AnalogCorrelation(const int&,const int&,const int&) const;
		
		//Compare delta time from input timestamp and the largest timestamp we currently know about
		//True -> still trying to correlate
		//False -> clear the Heaps and put the new timestamp into it
		bool RollingWindowCorrelation(const double&);
		
		//Compare delta time from input timestamp and the smallest timestamp we know about
		//True -> still trying to correlate 
		//False -> clear the Heaps and put the new timestamp into it
		bool RollingTriggerCorrelation(const double&); 

		CORRELATIONWINDOWTYPE CorrelationType;
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
