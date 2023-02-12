#include <string>
#include <iostream>

//#include <getopt.h>

#include "genscan.hpp"
#include "ArgParser.hpp"

int main(int argc, char *argv[]) {

	//struct option longOpts[] = {
	//	{"config",         required_argument, 0, 'c'},
	//	{"outputfile",     required_argument, 0, 'o'},
	//	{"inputfile",      required_argument, 0, 'i'},
	//	{"evtbuild",       no_argument,       0, 'e'},
	//	{"help",       no_argument,      0, 'h'},
	//	{0,0,0,0} 
	//};
	
	ArgParser* cmdArgs = new ArgParser();
	cmdArgs->AddArgument("c","config",ArgType::required_argument,"config filename");
	cmdArgs->AddArgument("o","outputfile",ArgType::required_argument,"output filename");
	cmdArgs->AddArgument("i","inputfile",ArgType::required_argument,"input filename");
	cmdArgs->AddArgument("e","evtbuild",ArgType::no_argument,"build events");
	cmdArgs->AddArgument("h","help",ArgType::no_argument,"show this message");
	cmdArgs->ParseArgs(argc,argv);

	genscan* scan = new genscan();

	//int retval = 0;
	//int idx = 0;
	////getopt_long is not POSIX compliant. It is provided by GNU. This may mean
	////that we are not compatable with some systems. If we have enough
	////complaints we can either change it to getopt, or implement our own class.
	//while ((retval = getopt_long(argc, argv, "c:o:i:eh", longOpts, &idx)) != -1) {
	//	switch (retval) {
	//		case 'c':
	//			if (optarg != 0) {
	//				scan->SetCfgFile(std::string(optarg));
	//			}
	//			break;
	//		case 'h':
	//			ShowUsage(argv[0]);
	//			return 0;
	//			break;
	//		default:
	//			ShowUsage(argv[0]);
	//			return 1;
	//			break;
	//	};
	//}

	return 0;
}
