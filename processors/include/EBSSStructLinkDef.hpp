#ifndef __EBSS_STRUCT_LINKDEF_HPP__
#define __EBSS_STRUCT_LINKDEF_HPP__
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::EBSSPaddle + ;
#pragma link C++ class std::vector < ProcessorStruct::EBSSPaddle> + ;
#pragma link C++ class ROOT::VecOps::RVec < ProcessorStruct::EBSSPaddle> + ;

#pragma link C++ struct ProcessorStruct::EBSStotal + ;

#endif
#endif // __EBSS_STRUCT_LINKDEF_HPP__
