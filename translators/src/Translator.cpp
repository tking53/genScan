#include "Translator.hpp"
#include <stdexcept>

Translator::Translator(const std::string& log,const std::string& translatorname){
	this->LogName = log;
	this->TranslatorName = translatorname;

	this->console = spdlog::get(this->LogName)->clone(this->TranslatorName);
	this->console->info("Created Translator [{}]",this->TranslatorName);
}
		
bool Translator::AddFile(const std::string& filename){
	std::ifstream file(filename);
	if( file.is_open() ){
		file.seekg(0, std::ios::end);
		this->FileSizes.push_back(file.tellg());
		file.close();
		this->InputFiles.push_back(filename);
		this->console->info("Added File {} to list of files to translate, File Size : {}",this->InputFiles.back(),this->FileSizes.back());
	}else{
		return false;
	}
}

[[noreturn]] void Translator::Parse(){
	console->error("Called Translator::Parse(), not the overload");
	throw std::runtime_error("Called Translator::Parse(), not the overload");
}
