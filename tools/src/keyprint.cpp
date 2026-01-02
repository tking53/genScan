#include <iostream>
#include <stdexcept>
#include <string>

#include <boost/program_options.hpp>

#include "TFile.h"
#include "TNamed.h"

int main(int argc, char *argv[]) {

	std::string inputfile;
	std::string keyname;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile),"file to get the histogram from")
		("keyname,k",boost::program_options::value<std::string>(&keyname),"TNamed value to extract")
		;

	boost::program_options::positional_options_description p;

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			std::cout << cmdline_options;
			exit(EXIT_SUCCESS);
		}

	}catch( std::exception& e){
		std::cerr << e.what();
		exit(EXIT_FAILURE);
	}    

	try{
		auto rfile = new TFile(inputfile.c_str(),"READ");
		auto key = rfile->Get<TNamed>(keyname.c_str()); 
		if( key != nullptr ){
			std::string keystring(key->GetTitle());
			std::cout << keystring << std::endl;
			exit(EXIT_SUCCESS);
		}else{
			throw std::runtime_error("histogram does not exist");
		}
	}catch( std::exception& e){
		std::cerr << e.what();
		exit(EXIT_FAILURE);
	}    


}
