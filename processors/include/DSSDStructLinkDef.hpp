#ifndef __DSSD_STRUCT_LINKDEF_HPP
#define __DSSD_STRUCT_LINKDEF_HPP
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::STRIP + ;
#pragma link C++ class std::vector < ProcessorStruct::STRIP> + ;
#pragma link C++ class ROOT::VecOps::RVec < ProcessorStruct::STRIP> + ;

#pragma link C++ struct ProcessorStruct::DSSD + ;
#pragma link C++ class std::vector < ProcessorStruct::DSSD> + ;
#pragma link C++ class ROOT::VecOps::RVec < ProcessorStruct::DSSD> + ;

#endif
#endif // __DSSD_STRUCT_LINKDEF_HPP
