#ifndef __BIT_DECODER_HPP__
#define __BIT_DECODER_HPP__

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
		unsigned int DecodeTimeLow(const unsigned int &) const;
		unsigned int DecodeTimeHigh(const unsigned int &) const;
		unsigned int DecodeEventEnergy(const unsigned int &) const;
		unsigned int DecodeTraceLength(const unsigned int &) const;
		unsigned int DecodeTraceOutRange(const unsigned int &) const;
		unsigned int DecodeCFDTime100(const unsigned int &) const;
		unsigned int DecodeCFDTime250(const unsigned int &) const;
		unsigned int DecodeCFDTime500(const unsigned int &) const;
		unsigned int DecodeCFDForce100(const unsigned int &) const;
		unsigned int DecodeCFDForce250(const unsigned int &) const;
		unsigned int DecodeCFDTrigSource250(const unsigned int &) const;
		unsigned int DecodeCFDTrigSource500(const unsigned int &) const;
		unsigned int DecodeESumTrailing(const unsigned int &) const;
		unsigned int DecodeESumLeading(const unsigned int &) const;
		unsigned int DecodeESumGap(const unsigned int &) const;
		unsigned int DecodeBaseline(const unsigned int &) const;
		unsigned int DecodeQDCSums(const unsigned int &) const;
	private:
		Mask TimeLowMask;
		Mask TimeHighMask;
		Mask EventEnergyMask;
		Mask TraceLengthMask;
		Mask TraceOutRangeMask;

		Mask CFDTime100Mask;
		Mask CFDTime250Mask;
		Mask CFDTime500Mask;
		Mask CFDForce100Mask;
		Mask CFDForce250Mask;
		Mask CFDTrigSource250Mask;
		Mask CFDTrigSource500Mask;

		Mask ESumTrailingMask;
		Mask ESumLeadingMask;
		Mask ESumGapMask;
		Mask BaselineMask;

		Mask QDCSumsMask;
};

namespace PIXIE{
	const Mask ChannelNumberMask(0xF,0);
	const Mask ModuleNumberMask(0xF0,4);
	const Mask CrateNumberMask(0xF00,8);
	const Mask HeaderLengthMask(0x1F000, 12);
	const Mask EventLengthMask(0x7FFE0000, 17);
	const Mask FinishCodeMask(0x80000000, 31);
}

#endif
