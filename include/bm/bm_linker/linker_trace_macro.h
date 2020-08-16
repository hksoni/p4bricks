/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file linker_trace_macro.h

#ifndef BM_BM_LINKER_TRACE_MACRO_H_
#define BM_BM_LINKER_TRACE_MACRO_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/lexical_cast.hpp>

#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/debugger.h>
#include <bm/bm_sim/event_logger.h>



namespace bm {

#define TRACE_START \
  {\
    std::string str = "\n*****"+std::string(__PRETTY_FUNCTION__)+" START****";\
    bm::Logger::get()->trace(str); \
  }\


#define TRACE_PRINT_CONST(val) \
  {\
    bm::Logger::get()->trace(val);\
  }\

#define TRACE_PRINT(v) \
  {\
    std::string s = "\n"+std::string(#v) +":"\
                    +boost::lexical_cast<std::string>(v);\
    bm::Logger::get()->trace(s);\
  }\

#define TRACE_END \
  {\
    std::string str = "\nxxxxx"+std::string(__PRETTY_FUNCTION__)+" END xxxxx";\
    bm::Logger::get()->trace(str); \
  }\


#define TRACE_PRINT_MAP(map) \
  {\
    std::string st = "\n*****Printing Map : "+ std::string(#map)+"*****\n"; \
    for (const auto& kv : map) { \
      std::string strkv = "["+boost::lexical_cast<std::string>(kv.first) +", "+ \
                       boost::lexical_cast<std::string>(kv.second)+"]";\
      st += strkv;\
    } \
    st = "\n***********************************************\n"; \
    bm::Logger::get()->trace(st);\
  }\

#define TRACE_PRINT_MAP_UDT_VAL(map, value_element) \
  {\
    std::string st = "\n*****Printing Map : "+ std::string(#map)+"*****\n"; \
    for (const auto& kv : map) { \
      std::string strkv = "["+boost::lexical_cast<std::string>(kv.first) +", "+ \
                       boost::lexical_cast<std::string>(\
                           kv.second->value_element)+"]";\
      st += strkv;\
    } \
    st = "\n***********************************************\n"; \
    bm::Logger::get()->trace(st);\
  }\


#define TRACE_PRINT_VECTOR(vec) \
  {\
    std::string st = "\n*****Printing Vec : "+ std::string(#vec)+"*****\n"; \
    st += "[";\
    for (const auto& kv : vec) { \
      st += boost::lexical_cast<std::string>(kv)+",";\
    } \
    st += "]";\
    st += "\n***********************************************\n"; \
    bm::Logger::get()->trace(st);\
  }\


#define TRACE_PRINT_VECTOR_UDT_VAL(vec, value_element) \
  {\
    std::string st = "\n*****Printing Vec : "+ std::string(#vec)+"*****\n"; \
    st += "[";\
    for (const auto& kv : vec) { \
      st += boost::lexical_cast<std::string>(kv->value_element)+",";\
    } \
    st += "]";\
    st += "\n***********************************************\n"; \
    bm::Logger::get()->trace(st);\
  }\

}
#endif  // BM_BM_LINKER_TRACE_MACRO_H_
