#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

//will be the generic processor class that all other processors will derive from
//this will act on every piece of data no matter what, will output to special root tree and make generic
//histograms inside root

#include <string>
#include <vector>

class Processor{
	public:
		Processor();
		Processor(std::string);
		virtual bool PreProcess();
		virtual bool Process();
		virtual bool PostProcess();
	protected:
		std::string ProcessorName;
};

#endif
