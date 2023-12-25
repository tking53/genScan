#ifndef __PLD_INFO_HPP__
#define __PLD_INFO_HPP__

#include <fstream>
#include <string>
#include <ctime>

#include "DataBuffer.hpp"

class PLDHeader : protected DataBuffer {
	public:
		PLDHeader();

		unsigned int GetBufferLength() const;
		const char* GetFacility() const;
		const char* GetFormat() const;
		const char* GetStartDate() const;
		const char* GetEndDate() const;
		const char* GetRunTitle() const;
		unsigned int GetRunNumber() const;
		unsigned int GetMaxSpillSize() const;
		float GetRunTime() const;

		void SetStartDateTime();
		void SetEndDateTime();
		void SetFacility(std::string&);
		void SetTitle(std::string&);
		void SetRunNumber(unsigned int&);
		void SetMaxSpillSize(unsigned int&);
		void SetRunNumber(float&);

		virtual bool Write(std::ofstream&) final;
		virtual bool Read(std::ifstream&) final;
		virtual void Reset() final;

		void Print() const;
		void PrintDelimited(const char&);
	private:
		unsigned int run_num;
		unsigned int max_spill_size;
		float run_time;
		char format[17];
		char facility[17];
		char start_date[25];
		char end_date[25];
		char* run_title;

		std::time_t runStartTime;
		std::time_t runStopTime;
};

class PLDData : protected DataBuffer{
	public:
		PLDData();
		virtual bool Write(std::ofstream&) final;
		virtual bool Read(std::ifstream&) final;
		virtual void Reset() final;
};

#endif
