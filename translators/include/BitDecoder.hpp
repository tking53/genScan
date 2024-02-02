#ifndef __BIT_DECODER_HPP__
#define __BIT_DECODER_HPP__

struct Mask{
	unsigned int MaskBits;
	unsigned int ShiftBits;
	Mask(const unsigned int& mask,const unsigned int& shift) : MaskBits(mask), ShiftBits(shift) {
	}

	unsigned int operator() (const unsigned int& data) const{
		return (data&MaskBits) >> ShiftBits;
	};
};

namespace PIXIE{
	const Mask ChannelNumberMask(0xF,0);
	const Mask ModuleNumberMask(0xF0,4);
	const Mask CrateNumberMask(0xF00,8);
	const Mask HeaderLengthMask(0x1F000, 12);
	const Mask EventLengthMask(0x7FFE0000, 17);
	const Mask FinishCodeMask(0x80000000, 31);
	const Mask TimeLowMask(0xFFFFFFFF, 0);
	const Mask TimeHighMask(0xFFFF, 0);
	const Mask EventEnergyMask(0xFFFF, 0);
	const Mask TraceLengthMask(0x7FFF0000, 16);
	const Mask TraceOutRangeMask(0x80000000, 31);

	const Mask CFDTime100Mask(0x7FFF0000, 16);
	const Mask CFDTime250Mask(0x3FFF0000, 16);
	const Mask CFDTime500Mask(0x1FFF0000, 16);
	const Mask CFDForce100Mask(0x80000000, 31);
	const Mask CFDForce250Mask(0x80000000, 31);
	const Mask CFDTrigSource250Mask(0x40000000, 30);
	const Mask CFDTrigSource500Mask(0xE0000000, 29);

	const Mask ESumTrailingMask(0xFFFFFFFF, 0);
	const Mask ESumLeadingMask(0xFFFFFFFF, 0);
	const Mask ESumGapMask(0xFFFFFFFF, 0);
	const Mask BaselineMask(0xFFFFFFFF, 0);

	const Mask QDCSumsMask(0xFFFFFFFF, 0);
}

#endif
