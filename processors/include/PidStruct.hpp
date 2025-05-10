#ifndef __PIDSTRUCT_HPP__
#define __PIDSTRUCT_HPP__

#include <vector>
namespace ProcessorStruct{
struct PidDet{
	double energy = -999;
	double time = -999;
	bool pileup = false;
	bool saturation = false;
};
static const PidDet DEFAULT_PIDDET_STRUCT;

struct PPAC {
	PidDet up = DEFAULT_PIDDET_STRUCT ;
	PidDet down = DEFAULT_PIDDET_STRUCT ;
	PidDet left = DEFAULT_PIDDET_STRUCT ;
	PidDet right = DEFAULT_PIDDET_STRUCT ;
	PidDet anode = DEFAULT_PIDDET_STRUCT ;
	double xpos = -999.0;
	double ypos = -999.0;
};
static const PPAC DEFAULT_PPAC_STRUCT;

struct SCINT {
	PidDet left = DEFAULT_PIDDET_STRUCT; 
	PidDet right = DEFAULT_PIDDET_STRUCT;
};
static const SCINT DEFAULT_SCINT_STRUCT;

struct DBOX {
	SCINT scint = DEFAULT_SCINT_STRUCT;
	PPAC ppac0 = DEFAULT_PPAC_STRUCT;
	PPAC ppac1 = DEFAULT_PPAC_STRUCT;
};
static const DBOX DEFAULT_DBOX_STRUCT;

struct FP {
	std::vector<PidDet> xplas = std::vector<PidDet>(4,DEFAULT_PIDDET_STRUCT);
	std::vector<PidDet> pin = std::vector<PidDet>(4,DEFAULT_PIDDET_STRUCT);
};
static const FP DEFAULT_FP_STRUCT;

};
#endif // !__PIDSTRUCT_HPP__
