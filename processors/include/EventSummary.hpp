#ifndef __EVENT_SUMMARY_HPP__
#define __EVENT_SUMMARY_HPP__

#include <set>
#include <string>
#include <regex>

#include <boost/container/devector.hpp>
#include <boost/regex.hpp>

#include "PhysicsData.hpp"

class EventSummary{
	public:
		EventSummary();
		~EventSummary() = default;
		
		void BuildDetectorSummary();
		void GetDetectorSummary(const std::string&,std::vector<PhysicsData*>&);
		void GetDetectorSummary(const boost::regex&,std::vector<PhysicsData*>&);

		boost::container::devector<PhysicsData>& GetRawEvents();
		void ClearRawEvents();

		const std::set<std::string>& GetKnownTypes() const;

	private:
		boost::container::devector<PhysicsData> RawEvents;
		std::set<std::string> KnownTypes;
		boost::regex ColonParse;
};

#endif
