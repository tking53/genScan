#ifndef __BIT_DECODER_HPP__
#define __BIT_DECODER_HPP__

#include "PhysicsData.hpp"

#include "ChannelMap.hpp"

struct Mask{
	unsigned int MaskBits;
	unsigned int ShiftBits;
	Mask(const unsigned int& mask,const unsigned int& shift) : MaskBits(mask), ShiftBits(shift) {
	}

	unsigned int operator() (const unsigned int& data) const{
		return (data&MaskBits) >> ShiftBits;
	};
};

class XiaDecoder{
	public:
		XiaDecoder(ChannelMap::FirmwareVersion,int);
		unsigned int DecodeEventLength(const unsigned int&) const;
		unsigned int DecodeTimeLow(const unsigned int &) const;
		unsigned int DecodeTimeHigh(const unsigned int &) const;
		unsigned int DecodeEventEnergy(const unsigned int &) const;
		unsigned int DecodeTraceLength(const unsigned int &) const;
		unsigned int DecodeTraceOutRange(const unsigned int &) const;
		unsigned int DecodeCFDTime(const unsigned int &) const;
		unsigned int DecodeCFDForce(const unsigned int &) const;
		unsigned int DecodeCFDTrigSource(const unsigned int &) const;
		unsigned int DecodeESumTrailing(const unsigned int &) const;
		unsigned int DecodeESumLeading(const unsigned int &) const;
		unsigned int DecodeESumGap(const unsigned int &) const;
		unsigned int DecodeBaseline(const unsigned int &) const;
		unsigned int DecodeQDCSums(const unsigned int &) const;
		unsigned int DecodeExternalTSLow(const unsigned int &) const;
		unsigned int DecodeExternalTSHigh(const unsigned int &) const;
		double GetCFDSize() const;

		void DecodeFirstWords(const unsigned int*,uint32_t&,uint32_t&,uint32_t&,unsigned int&,unsigned int&,bool&) const;
		double DecodeCFDParams(const unsigned int*,const uint64_t&,PhysicsData&) const;
		void DecodeOtherWords(const unsigned int*,PhysicsData*) const;
	private:
		ChannelMap::FirmwareVersion Ver;
		int Freq;

		Mask EventLengthMask;
		Mask TimeLowMask;
		Mask TimeHighMask;
		Mask EventEnergyMask;
		Mask TraceLengthMask;
		Mask TraceOutRangeMask;

		Mask CFDTimeMask;
		Mask CFDForceMask;
		Mask CFDTrigSourceMask;

		Mask ESumTrailingMask;
		Mask ESumLeadingMask;
		Mask ESumGapMask;
		Mask BaselineMask;

		Mask QDCSumsMask;

		Mask ExtTSLowMask;
		Mask ExtTSHighMask;

		double CFDSize;
};

namespace PIXIE{
	const Mask ChannelNumberMask(0x0000000F,0);
	const Mask ModuleNumberMask(0x000000F0,4);
	const Mask CrateNumberMask(0x00000F00,8);
	const Mask HeaderLengthMask(0x0001F000, 12);
	const Mask FinishCodeMask(0x8000000, 31);
}

#endif
