#ifndef __GENERIC_PROCESSOR_HPP__
#define __GENERIC_PROCESSOR_HPP__

#include "Processor.hpp"

class GenericProcessor : public Processor{
	public:
		GenericProcessor();
		bool PreProcess() final;
		bool Process() final;
		bool PostProcess() final;
};

#endif
