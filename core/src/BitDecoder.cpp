#include "BitDecoder.hpp"

XiaDecoder::XiaDecoder(ChannelMap::FirmwareVersion firmware,int Frequency) : 
	TimeLowMask(0xFFFFFFFF, 0),
	TimeHighMask(0xFFFF, 0),
	EventEnergyMask(0xFFFF, 0),
	TraceLengthMask(0x7FFF0000, 16),
	TraceOutRangeMask(0x80000000, 31),

	CFDTime100Mask(0x7FFF0000, 16),
	CFDTime250Mask(0x3FFF0000, 16),
	CFDTime500Mask(0x1FFF0000, 16),
	CFDForce100Mask(0x80000000, 31),
	CFDForce250Mask(0x80000000, 31),
	CFDTrigSource250Mask(0x40000000, 30),
	CFDTrigSource500Mask(0xE0000000, 29),

	ESumTrailingMask(0xFFFFFFFF, 0),
	ESumLeadingMask(0xFFFFFFFF, 0),
	ESumGapMask(0xFFFFFFFF, 0),
	BaselineMask(0xFFFFFFFF, 0),

	QDCSumsMask(0xFFFFFFFF, 0)
{
}

unsigned int XiaDecoder::DecodeTimeLow(const unsigned int & val) const {
	return this->TimeLowMask(val);
}

unsigned int XiaDecoder::DecodeTimeHigh(const unsigned int & val) const {
	return this->TimeHighMask(val);
}

unsigned int XiaDecoder::DecodeEventEnergy(const unsigned int & val) const {
	return this->EventEnergyMask(val);
}

unsigned int XiaDecoder::DecodeTraceLength(const unsigned int & val) const {
	return this->TraceLengthMask(val);
}

unsigned int XiaDecoder::DecodeTraceOutRange(const unsigned int & val) const {
	return this->TraceOutRangeMask(val);
}

unsigned int XiaDecoder::DecodeCFDTime100(const unsigned int & val) const {
	return this->CFDTime100Mask(val);
}

unsigned int XiaDecoder::DecodeCFDTime250(const unsigned int & val) const {
	return this->CFDTime250Mask(val);
}

unsigned int XiaDecoder::DecodeCFDTime500(const unsigned int & val) const {
	return this->CFDTime500Mask(val);
}

unsigned int XiaDecoder::DecodeCFDForce100(const unsigned int & val) const {
	return this->CFDForce100Mask(val);
}

unsigned int XiaDecoder::DecodeCFDForce250(const unsigned int & val) const {
	return this->CFDForce250Mask(val);
}

unsigned int XiaDecoder::DecodeCFDTrigSource250(const unsigned int & val) const {
	return this->CFDTrigSource250Mask(val);
}

unsigned int XiaDecoder::DecodeCFDTrigSource500(const unsigned int & val) const {
	return this->CFDTrigSource500Mask(val);
}

unsigned int XiaDecoder::DecodeESumTrailing(const unsigned int & val) const {
	return this->ESumTrailingMask(val);
}

unsigned int XiaDecoder::DecodeESumLeading(const unsigned int & val) const {
	return this->ESumLeadingMask(val);
}

unsigned int XiaDecoder::DecodeESumGap(const unsigned int & val) const {
	return this->ESumGapMask(val);
}

unsigned int XiaDecoder::DecodeBaseline(const unsigned int & val) const {
	return this->BaselineMask(val);
}

unsigned int XiaDecoder::DecodeQDCSums(const unsigned int & val) const {
	return this->QDCSumsMask(val);
}
