#ifndef __MTAS_STRUCT_LINKDEF_HPP
#define __MTAS_STRUCT_LINKDEF_HPP
#ifdef __CINT__

#pragma link C++ struct ProcessorStruct::MtasSegment+;
#pragma link C++ class std::vector<ProcessorStruct::MtasSegment>+;

#pragma link C++ struct ProcessorStruct::MtasTotal+;
#pragma link C++ class std::vector<ProcessorStruct::MtasTotal>+;

#endif
#endif //PAASS_LINKDEF_HPP