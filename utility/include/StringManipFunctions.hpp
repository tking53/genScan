#ifndef __STRING_MANIP_FUNCTIONS_HPP__
#define __STRING_MANIP_FUNCTIONS_HPP__

#include <string>
#include <vector>

namespace StringManip{
	std::string StripFileExtension(const std::string&);
	std::string GetFileExtension(const std::string&);
	std::string GetFileBaseName(const std::string&);
	std::string GetFilePath(const std::string&);

	void ParseCalString(const std::string& calstring,std::vector<double>& vals);
}

#endif
