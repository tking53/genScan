#ifndef COMPASSDATA_HPP
#define COMPASSDATA_HPP

#include <vector>

//How compass does their flags on the events
//     0x1 dead time event occured before this event
//     0x2 time-stamp roll over
//     0x4 time-stamp reset from external
//     0x8 fake event
//    0x10 memory full occured before this event
//    0x20 trigger lost occured before this event
//    0x40 N triggers have been lost [found through certain bits of a register]
//    0x80 event saturating in the gate
//   0x100 1024 triggers have been counted
//   0x400 input is saturating
//   0x800 N triggers have been counted [found through certain bits of a register]
//  0x1000 event not matched in time correlation
//  0x4000 event with fine timestamp
//  0x8000 pileup event
// 0x80000 identifies fake event reporting a PLL lock loss
//0x100000 identifies fake event reporting over-temperature condition
//0x200000 identifies fake event reporting and ADC shutdown

struct RAWCOMPASSDATA{
	unsigned short energy;
	unsigned short energyshort;
	unsigned short channel;
	unsigned short board;
	unsigned long long timestamp;
	unsigned int flags;
	std::vector<short> trace;	
};

struct CompassData : public RAWCOMPASSDATA{
	double calenergy;
	double calenergyshort;
	double timestamp_ns;
	bool issaturating;
	bool isclipping;
	bool ispileup;
};

#endif 
