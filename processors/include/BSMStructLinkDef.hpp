#ifndef __BSM_STRUCT_LINKDEF_HPP
#define __BSM_STRUCT_LINKDEF_HPP
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::BSMTraceFit+;
#pragma link C++ class std::vector<ProcessorStruct::BSMTraceFit>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::BSMTraceFit>+;

#pragma link C++ struct ProcessorStruct::BSMSingle+;
#pragma link C++ class std::vector<ProcessorStruct::BSMSingle>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::BSMSingle>+;

#pragma link C++ struct ProcessorStruct::BSMSegment+;
#pragma link C++ class std::vector<ProcessorStruct::BSMSegment>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::BSMSingle>+;

#pragma link C++ struct ProcessorStruct::BSMTotal+;
#pragma link C++ class std::vector<ProcessorStruct::BSMTotal>+;
#pragma link C++ class ROOT::VecOps::RVec<ProcessorStruct::BSMTotal>+;

#endif
#endif //PAASS_LINKDEF_HPP
