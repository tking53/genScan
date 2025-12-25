#ifndef __PID_STRUCT_LINKDEF_HPP__
#define __PID_STRUCT_LINKDEF_HPP__
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::DBOX+;
#pragma link C++ class std::vector<ProcessorStruct::DBOX>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::DBOX>+;

#pragma link C++ struct ProcessorStruct::FP+;
#pragma link C++ class std::vector<ProcessorStruct::FP>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::FP>+;

#pragma link C++ struct ProcessorStruct::SCINT+;
#pragma link C++ class std::vector<ProcessorStruct::SCINT>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::SCINT>+;

#pragma link C++ struct ProcessorStruct::PPAC+;
#pragma link C++ class std::vector<ProcessorStruct::PPAC>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::PPAC>+;

#pragma link C++ struct ProcessorStruct::PidDet+;
#pragma link C++ class std::vector<ProcessorStruct::PidDet>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::PidDet>+;

#endif
#endif //PAASS_LINKDEF_HPP
