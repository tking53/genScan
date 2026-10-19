#ifndef __MUSES_STRUCT_LINKDEF_HPP__
#define __MUSES_STRUCT_LINKDEF_HPP__
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::MusesPixel + ;
#pragma link C++ class std::vector < ProcessorStruct::MusesPixel> + ;
#pragma link C++ class ROOT::VecOps::RVec < ProcessorStruct::MusesPixel> + ;

#endif
#endif // PAASS_LINKDEF_HPP
