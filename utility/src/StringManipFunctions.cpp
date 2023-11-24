#include "StringManipFunctions.hpp"

namespace StringManip{
	std::string StripFileExtension(const std::string File){
		auto basefile = GetFileBaseName(File);
		return GetFilePath(File) + basefile.substr(0,basefile.find_last_of("."));
	}
	
	std::string GetFileExtension(const std::string& File){
		return File.substr(File.find_last_of(".")+1);
	}

	std::string GetFileBaseName(const std::string& File){
		return File.substr(File.find_last_of("/\\")+1);
	}

	std::string GetFilePath(const std::string& File){
		return File.substr(0,File.find_last_of("/\\")+1);
	}
}
