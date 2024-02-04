#ifndef __CORRELATOR_HPP__
#define __CORRELATOR_HPP__

#include <functional>
#include <tuple>
#include <set>

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

		Correlator(double);
		~Correlator() = default;

		void SetCorrelationType(CORRELATIONWINDOWTYPE);

		//switch based on the type of correlation we were constructed with
		//what's passed in is the timestamp (in ns), crate, module, channel of the last parsed event
		bool IsWithinCorrelationWindow(double,int,int,int);

		void AddTriggerChannel(int,int,int);
	private:
		//just check to see if it's one of the channels we reset the correlation on
		//False -> was a trigger channel
		//True -> not a trigger channel
		bool AnalogCorrelation(int,int,int) const;
		
		//Compare delta time from input timestamp and the largest timestamp we currently know about
		//True -> still trying to correlate
		//False -> clear the Heaps and put the new timestamp into it
		bool RollingWindowCorrelation(double);
		
		//Compare delta time from input timestamp and the smallest timestamp we know about
		//True -> still trying to correlate 
		//False -> clear the Heaps and put the new timestamp into it
		bool RollingTriggerCorrelation(double); 

		CORRELATIONWINDOWTYPE CorrelationType;
		double Width;
		
		//Used for analog triggering scheme
		std::set<AnalogTrigger> Triggers;

		//used for rolling window triggering scheme
		boost::heap::priority_queue<double,boost::heap::compare<std::less<double>>> MinHeap;

		//used for fixed window rolling trigger scheme
		boost::heap::priority_queue<double,boost::heap::compare<std::greater<double>>> MaxHeap;
};

#endif
