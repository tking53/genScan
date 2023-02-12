#ifndef GENSCAN_HPP
#define GENSCAN_HPP

#include <iostream>
#include <thread>
#include <utility>
#include <map>
#include <string>

#include <sys/stat.h> //For directory manipulation

class genscan{
	public:
		genscan();
		~genscan() = default;
		void SetCfgFile(std::string);

	private:
		std::string cfgFile;


};
#endif
