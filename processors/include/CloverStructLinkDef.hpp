#ifndef __CLOVER_STRUCT_LINKDEF_HPP__
#define __CLOVER_STRUCT_LINKDEF_HPP__
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::Leaf+;
#pragma link C++ class std::vector<ProcessorStruct::Leaf>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::Leaf>+

#pragma link C++ struct ProcessorStruct::Clover+;
#pragma link C++ class std::vector<ProcessorStruct::Clover>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::Clover>+;

#endif
#endif //PAASS_LINKDEF_HPP
