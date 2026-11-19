#ifndef __DLS_STRUCT_LINKDEF_HPP
#define __DLS_STRUCT_LINKDEF_HPP
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::DLS + ;
#pragma link C++ class std::vector < ProcessorStruct::DLS> + ;
#pragma link C++ class ROOT::VecOps::RVec < ProcessorStruct::DLS> + ;

#endif
#endif // PAASS_LINKDEF_HPP
