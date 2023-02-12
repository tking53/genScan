#ifndef GENSCAN_HPP
#define GENSCAN_HPP

#include <iostream>
#include <thread>
#include <utility>
#include <map>
#include <string>

#include <sys/stat.h> //For directory manipulation

#include "Unpacker.hpp"

class genscan{
	public:
		genscan();
		~genscan() = default;

		void SetCfgFile(std::string*);
		void SetInputFile(std::string*);
		void SetOutputFile(std::string*);
		void SetEvtBuild(bool);

		void DoScan();
	private:
		std::string* cfgFile;
		std::string* inputFile;
		std::string* outputFile;
		bool evtBuild;

		Unpacker* unpacker;
};
#endif
