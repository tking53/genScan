#include <fstream>
#include <stdexcept>

#include "Translator.hpp"

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
		return true;
	}else{
		return false;
	}
}

[[noreturn]] void Translator::Parse([[maybe_unused]] boost::container::devector<PhysicsData>& RawEvents){
	console->error("Called Translator::Parse(), not the overload");
	throw std::runtime_error("Called Translator::Parse(), not the overload");
}

void Translator::FinalizeFiles(){
	NumTotalFiles = InputFiles.size();
	NumFilesRemaining = NumTotalFiles;
	CurrentFileIndex = 0;
	FinishedCurrentFile = true;
}

bool Translator::OpenNextFile(){
	FinishedCurrentFile = false;
	if( CurrentFileIndex == 0 ){
		CurrentFile.open(InputFiles.at(CurrentFileIndex),std::ifstream::binary);
		return true;
	}else if(CurrentFileIndex == NumTotalFiles){
		CurrentFile.close();
		return false;
	}else{
		CurrentFile.close();
		++CurrentFileIndex;
		CurrentFile.open(InputFiles.at(CurrentFileIndex),std::ifstream::binary);
		return true;
	}
}
