#ifndef UNPACKER_HPP
#define UNPACKER_HPP

#include <string>

class Unpacker{
	public:
		Unpacker() = default;
		virtual ~Unpacker() = default;
		virtual void SetCurrFile(std::string*);
		virtual std::string* GetCurrFile();
	protected:
		std::string* currfile;
};

#endif 
