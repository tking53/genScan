#include "BitDecoder.hpp"
#include "ChannelMap.hpp"
#include <stdexcept>
#include <string>

XiaDecoder::XiaDecoder(ChannelMap::FirmwareVersion firmware,int Frequency) : 
	EventLengthMask(0x7FFE0000,17),
	TimeLowMask(0xFFFFFFFF, 0),
	TimeHighMask(0xFFFF, 0),
	EventEnergyMask(0xFFFF, 0),
	TraceLengthMask(0x7FFF0000, 16),
	TraceOutRangeMask(0x80000000, 31),

	CFDTimeMask(0x7FFF0000, 16),
	CFDForceMask(0x80000000, 31),
	CFDTrigSourceMask(0xE0000000, 29),

	ESumTrailingMask(0xFFFFFFFF, 0),
	ESumLeadingMask(0xFFFFFFFF, 0),
	ESumGapMask(0xFFFFFFFF, 0),
	BaselineMask(0xFFFFFFFF, 0),

	QDCSumsMask(0xFFFFFFFF, 0),
	QDCSize(655536)
{
	if( Frequency == 100 ){
		switch( firmware ){
			case ChannelMap::R17562:
			case ChannelMap::R29432:
				CFDTimeMask = Mask(0xFFFF0000,16);
				break;
			case ChannelMap::R30474:
			case ChannelMap::R30980:
			case ChannelMap::R30981:
			case ChannelMap::R34688:
				CFDTimeMask = Mask(0x7FFF0000,16);
				break;
			default:
				break;
	
		}
	}else if( Frequency == 250 ){
		switch( firmware ){
			case ChannelMap::R20466:
				CFDTimeMask = Mask(0xFFFF0000,16);
				break;
			case ChannelMap::R27361:
			case ChannelMap::R29432:
				CFDTimeMask = Mask(0x7FFF0000,16);
				break;
			case ChannelMap::R30474:
			case ChannelMap::R30980:
			case ChannelMap::R30981:
				CFDTimeMask = Mask(0x3FFF0000,16);
				break;
			case ChannelMap::R34688:
				CFDTimeMask = Mask(0x3FFF0000,16);
				break;
			default:
				break;
		}
	}else if( Frequency == 500 ){
		switch( firmware ) {
			case ChannelMap::R29432:
			case ChannelMap::R30474:
			case ChannelMap::R30980:
			case ChannelMap::R30981:
			case ChannelMap::R34688:
				CFDTimeMask = Mask(0x1FFF0000,16);
				break;
			case ChannelMap::R35207:
				CFDTimeMask = Mask(0x1FFF0000,16);
				break;
			default:
				break;
		}
	}else{
		throw std::runtime_error("Unknown Frequency : "+std::to_string(Frequency)+", Known are 100,250,500");
	}
	switch( firmware ){
		case ChannelMap::R17562:
		case ChannelMap::R20466:
		case ChannelMap::R27361:
			EventLengthMask = Mask(0x3FFE0000,17);
			break;
		case ChannelMap::R29432:
		case ChannelMap::R30474:
		case ChannelMap::R30980:
		case ChannelMap::R30981:
		case ChannelMap::R34688:
		case ChannelMap::R35207:
			EventLengthMask = Mask(0x7FFE0000,17);
			break;
		default:
			break;
	}

	if( Frequency ==  100 ){
		CFDTrigSourceMask = Mask(0,0);
	}else if (Frequency == 250) {
		switch( firmware ){
			case ChannelMap::R27361:
			case ChannelMap::R29432:
				CFDTrigSourceMask = Mask(0x80000000,31);
				break;
			case ChannelMap::R30474:
			case ChannelMap::R30980:
			case ChannelMap::R30981:
			case ChannelMap::R34688:
				CFDTrigSourceMask = Mask(0x40000000,30);
				break;
			default:
				break;
		}
	}else if( Frequency == 500 ) {
		switch( firmware ){
			case ChannelMap::R29432:
			case ChannelMap::R30474:
			case ChannelMap::R30980:
			case ChannelMap::R30981:
			case ChannelMap::R34688:
			case ChannelMap::R35207:
				CFDTrigSourceMask = Mask(0xE0000000,29);
				break;
			default:
				break;
		}
	}
	switch( firmware ){
		case ChannelMap::R29432:
		case ChannelMap::R30474:
		case ChannelMap::R30980:
		case ChannelMap::R30981:
			EventEnergyMask = Mask(0x00007FFF,0);
			break;
		case ChannelMap::R17562:
		case ChannelMap::R20466:
		case ChannelMap::R27361:
		case ChannelMap::R35207:
		case ChannelMap::R34688:
			EventEnergyMask = Mask(0x0000FFFF,0);
			break;
		default:
			break;
	}
	switch( firmware ){
		case ChannelMap::R17562:
		case ChannelMap::R20466:
		case ChannelMap::R27361:
			TraceOutRangeMask = Mask(0x40000000,30);
			break;
		case ChannelMap::R29432:
		case ChannelMap::R30474:
		case ChannelMap::R30980:
		case ChannelMap::R30981:
			TraceOutRangeMask = Mask(0x00008000,15);
			break;
		case ChannelMap::R35207:
		case ChannelMap::R34688:
			TraceOutRangeMask = Mask(0x80000000,31);
			break;
		default:
			break;
	}
	switch( firmware ){
		case ChannelMap::R17562:
		case ChannelMap::R20466:
		case ChannelMap::R27361:
		case ChannelMap::R29432:
		case ChannelMap::R30474:
		case ChannelMap::R30980:
		case ChannelMap::R30981:
			TraceLengthMask = Mask(0xFFFF0000,16);
			break;
		case ChannelMap::R35207:
		case ChannelMap::R34688:
			TraceLengthMask = Mask(0x7FFF0000,16);
			break;
		default:
			break;
	}

	if( Frequency == 500){
		this->QDCSize = 8192;
	}else if( Frequency == 100) {
		switch( firmware ){
			case ChannelMap::R17562:
			case ChannelMap::R29432:
				this->QDCSize = 65536;
				break;
			case ChannelMap::R30474:
			case ChannelMap::R30980:
			case ChannelMap::R30981:
			case ChannelMap::R34688:
				this->QDCSize = 32768;
				break;
			default:
				break;
		}
	}else if( Frequency  == 250 ){
		switch( firmware ){
			case ChannelMap::R20466:
				this->QDCSize = 65536;
				break;
			case ChannelMap::R27361:
			case ChannelMap::R29432:
				this->QDCSize = 32768;
				break;
			case ChannelMap::R30980:
			case ChannelMap::R30981:
			case ChannelMap::R34688:
			case ChannelMap::R30474:
				this->QDCSize = 16384;
				break;
			default:
				break;
		}
	}


}

unsigned int XiaDecoder::DecodeEventLength(const unsigned int& val) const{
	return this->EventLengthMask(val);
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

unsigned int XiaDecoder::DecodeCFDTime(const unsigned int & val) const {
	return this->CFDTimeMask(val);
}

unsigned int XiaDecoder::DecodeCFDForce(const unsigned int & val) const {
	return this->CFDForceMask(val);
}

unsigned int XiaDecoder::DecodeCFDTrigSource(const unsigned int & val) const {
	return this->CFDTrigSourceMask(val);
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

double XiaDecoder::GetQDCSize() const{
	return this->QDCSize;
}

void XiaDecoder::DecodeFirstWords(const unsigned int* firstFour,uint32_t& eventlen,uint32_t& tslow,uint32_t& tshigh,unsigned int& erg,unsigned int& tracelen,bool& tracesat) const{
	eventlen = this->DecodeEventLength(firstFour[0]);
	tslow = this->DecodeTimeLow(firstFour[1]);
	tshigh = this->DecodeTimeHigh(firstFour[2]);
	erg = this->DecodeEventEnergy(firstFour[3]);
	tracelen = this->DecodeTraceLength(firstFour[3]);
	tracesat = static_cast<bool>(this->DecodeTraceOutRange(firstFour[3]));
}

void XiaDecoder::DecodeCFDParams(const unsigned int* firstFour,PhysicsData& pd) const{
}

void XiaDecoder::DecodeOtherWords(const unsigned int* otherWords,PhysicsData* pd) const{
	if( pd->GetHeaderLength() == 8 ){
		pd->SetESumTrailing(this->DecodeESumTrailing(otherWords[0]));
		pd->SetESumLeading(this->DecodeESumLeading(otherWords[1]));
		pd->SetESumGap(this->DecodeESumGap(otherWords[2]));
		pd->SetESumBaseline(this->DecodeBaseline(otherWords[3]));
	}else if( pd->GetHeaderLength() == 12 ){
		pd->SetRawQDCSumLength(8);
		for( unsigned int ii = 0; ii < 8; ++ii ){
			pd->SetQDCValue(ii,this->DecodeQDCSums(otherWords[ii]));
		}
	}else if( pd->GetHeaderLength() == 16 ){
		pd->SetESumTrailing(this->DecodeESumTrailing(otherWords[0]));
		pd->SetESumLeading(this->DecodeESumLeading(otherWords[1]));
		pd->SetESumGap(this->DecodeESumGap(otherWords[2]));
		pd->SetESumBaseline(this->DecodeBaseline(otherWords[3]));
		pd->SetRawQDCSumLength(8);
		for( unsigned int ii = 0; ii < 8; ++ii ){
			pd->SetQDCValue(ii,this->DecodeQDCSums(otherWords[ii+4]));
		}
	}
}
