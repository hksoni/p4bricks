/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file linker_commons.h

#ifndef BM_BM_LINKER_COMMONS_H_
#define BM_BM_LINKER_COMMONS_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <boost/lexical_cast.hpp>

#include <bm/bm_sim/named_p4object.h>

#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/debugger.h>
#include <bm/bm_sim/event_logger.h>

#include <bm/bm_linker/linker_trace_macro.h>


namespace std {
  std::string to_string( std::string value ); 
}

namespace bm {


// Alter if the type of "name" member changes in class NamedP4Object of 
// named_p4objects.h file
using p4object_name_t = std::string;

//! To store result of linkers unique way to identify NamedP4Object
//! independent of P4Object types
using linker_uid_t = std::string;

//! Types in the P4Objects of the Linker
//! Should be same as defined in NamedP4Object
using linker_p4object_id_t = p4object_id_t;
using linker_p4object_name_t = p4object_name_t;


enum ObjectLinkStateCode {
  HEADER_TYPE_DEF_MATCH = 0,
  HEADER_TYPE_DEF_MATCH_FAILED,
  HEADER_TYPE_SELECT_FIELD_MATCH,
  HEADER_TYPE_VL_FIELD_MATCH,
  HEADER_TYPE_LINKED,
  HEADER_LINKED,
  HEADER_LINK_FAILED,
  STATE_HEADER_OPS_LINKED,
  STATE_HEADER_OPS_LINK_FAILED,
  STATE_PARSE_SWITCH_KEY_MATCH,
  STATE_PARSE_SWITCH_KEY_MATCH_FAILED,
  STATE_LINKING_FAILED,
  STATE_LINKED,
  STATE_MATCHED,
  PARSER_LINKING_FAILED,
  LINKING_FAILED,
  DEFAULT
};

class Enums {

 public:
  static std::string to_string(ObjectLinkStateCode val) {
    switch(val) {
      case HEADER_TYPE_DEF_MATCH: return "HEADER_TYPE_DEF_MATCH";
      case HEADER_TYPE_DEF_MATCH_FAILED: return "HEADER_TYPE_DEF_MATCH_FAILED";
      case HEADER_TYPE_SELECT_FIELD_MATCH: 
          return "HEADER_TYPE_SELECT_FIELD_MATCH";
      case HEADER_TYPE_VL_FIELD_MATCH: return "HEADER_TYPE_VL_FIELD_MATCH";
      case HEADER_TYPE_LINKED: return "HEADER_TYPE_LINKED";
      case HEADER_LINKED: return "HEADER_LINKED";
      case HEADER_LINK_FAILED: return "HEADER_LINK_FAILED";
      case STATE_HEADER_OPS_LINKED: return "STATE_HEADER_OPS_LINKED";
      case STATE_HEADER_OPS_LINK_FAILED: return "STATE_HEADER_OPS_LINK_FAILED";
      case STATE_PARSE_SWITCH_KEY_MATCH: return "STATE_PARSE_SWITCH_KEY_MATCH";
      case STATE_PARSE_SWITCH_KEY_MATCH_FAILED: 
          return "STATE_PARSE_SWITCH_KEY_MATCH_FAILED";
      case STATE_LINKING_FAILED: return "STATE_LINKING_FAILED";
      case STATE_LINKED: return "STATE_LINKED";
      case STATE_MATCHED: return "STATE_MATCHED";
      case PARSER_LINKING_FAILED: return "PARSER_LINKING_FAILED";
      case LINKING_FAILED: return "LINKING_FAILED";
      default: return "DEFAULT";
    }
  }

};


template<typename TValue>
struct LinkValueState {
  std::string pname;
  TValue value;
  ObjectLinkStateCode link_state;

  LinkValueState() {} 

  LinkValueState(std::string p4_name, TValue val) 
    : pname(p4_name), value(val), link_state(ObjectLinkStateCode::DEFAULT) { }
  LinkValueState(std::string p4_name, TValue val, ObjectLinkStateCode state) 
    : pname(p4_name), value(val), link_state(state) { }

  inline bool operator==(const LinkValueState<TValue>& x) const {
    //return (x.value == value && x.link_state == link_state);
    return (x.pname == pname && x.value == value);
  }

  std::string to_string() const {
  return ("("+pname+", "+boost::lexical_cast<std::string>(value)+", "
          +Enums::to_string(link_state)+")");
  }
};


class P4ObjectIdAllocator {

 public:
   
  P4ObjectIdAllocator() {}

  bool mark_id_range(p4object_id_t start, p4object_id_t stop) {
    for (p4object_id_t i = start; i<=stop; i++) 
      if(!mark_id(i))
        return false;
  }
  
  bool mark_id(p4object_id_t id) {
    return used_ids.insert(id).second;
  }

  p4object_id_t get_next_id() {
    id_gen = 0;
    while(!mark_id(id_gen++));
    return id_gen;
  }

 private:
  p4object_id_t id_gen;
  std::unordered_set<p4object_id_t> used_ids;

};

enum TableRetCode {
  SUCCESS = 0,
  P4_OBJECT_EXIST,
  LINK_OBJECT_P4_NAME_EXIST,
  P4_OBJECT_MATCH_FAILED,
  LINK_OBJECT_MATCH_FAILED,
  NO_P4_OBJECT_LINK_EXIST,
  NO_P4_OBJECT_EXIST,
  TABLE_INCONSISTENT
 };


//! This class provides one-linker-p4object-id to one-p4object-id for each P4
//! program in the system.
//! e.g. Two header instances of the same P4 program can not map to single header
//! instance of the linker P4 program
//!
//! One-to-one - from a P4 program's object to Linker's P4 object.
//! One-to-many - Linker's p4 object to many P4 program's one object
//! Linker's P4 object can map to a single object of multiple P4 program
template<typename T_P4Object_Unique>
class LinkerUIDTableGeneric {
 public:
  LinkerUIDTableGeneric() {}; 

  TableRetCode insert_p4objects_link(
      const std::string& p4_name, T_P4Object_Unique val, 
      T_P4Object_Unique link_val, ObjectLinkStateCode state) {

    // Check, if the val of the same program has mapping to any linker object
    //std::string fqn = p4_name + "."+std::to_string(val);
    std::string fqn = p4_name + "."+boost::lexical_cast<std::string>(val);
    auto search = fqn_to_linker_p4object_uid_map.find(fqn);
    if (search != fqn_to_linker_p4object_uid_map.end())
      return TableRetCode::P4_OBJECT_EXIST;

    // Check, if link_val maps to id of the p4_name
    LinkValueState<T_P4Object_Unique> p4name_val_state(p4_name, val, state);
    auto search_map  = linker_to_p4s_id_map.find(link_val);
    if (search_map != linker_to_p4s_id_map.end()) { //link_val exist
    //search if  link_val maps to any val in the same p4_name
      auto search_set = search_map->second.find(p4name_val_state);
      if (search_set != search_map->second.end()) {
        return TableRetCode::LINK_OBJECT_P4_NAME_EXIST;
      } else { // add into the set
        search_map->second.insert(p4name_val_state);
      }
    } else { // create set and add entry in to the linker_to_p4s_id_map
      std::unordered_set<LinkValueState<T_P4Object_Unique> > 
        set_p4_vals({p4name_val_state});
      linker_to_p4s_id_map.insert({link_val, set_p4_vals});
    }

    // LinkValueState<T_P4Object_Unique> lval_state("Linker", link_val, state);
    fqn_to_linker_p4object_uid_map.emplace(
        std::make_pair(fqn, LinkValueState<T_P4Object_Unique> 
                              ("Linker", link_val, state)));
    return TableRetCode::SUCCESS;
  }

  TableRetCode update_p4objects_link(
      const std::string& p4_name, T_P4Object_Unique val, 
      T_P4Object_Unique link_val, ObjectLinkStateCode state) {

    //std::string fqn = p4_name + "."+std::to_string(val);
    std::string fqn = p4_name + "."+boost::lexical_cast<std::string>(val);
    auto search = fqn_to_linker_p4object_uid_map.find(fqn);
    if (search == fqn_to_linker_p4object_uid_map.end())
      return TableRetCode::NO_P4_OBJECT_EXIST;
    if (search->second.value != link_val) {
      return TableRetCode::NO_P4_OBJECT_LINK_EXIST;
    }
    search->second.link_state = state;

    // No checking here, just update
    auto search_map = linker_to_p4s_id_map.find(link_val);
    if (search_map == linker_to_p4s_id_map.end()) {
      //log fatal
      return TableRetCode::TABLE_INCONSISTENT;
    }
    LinkValueState<T_P4Object_Unique> p4name_val_state(p4_name, val);
    size_t no_elem_removed = search_map->second.erase(p4name_val_state);
    if (!no_elem_removed) {
      return TableRetCode::TABLE_INCONSISTENT;
    }
    p4name_val_state.link_state = state;
    search_map->second.insert(p4name_val_state);
    return TableRetCode::SUCCESS;
  }


  TableRetCode get_p4objects_link_state(
      const std::string& p4_name, T_P4Object_Unique val, 
      T_P4Object_Unique link_val, ObjectLinkStateCode& ret_state_code) const {

    //std::string fqn = p4_name + "."+std::to_string(val);
    std::string fqn = p4_name + "."+boost::lexical_cast<std::string>(val);
    const auto search = fqn_to_linker_p4object_uid_map.find(fqn);
    if (search == fqn_to_linker_p4object_uid_map.end()) {
      return TableRetCode::P4_OBJECT_MATCH_FAILED;
    }
    if (search->second.value  != link_val)
      return TableRetCode::LINK_OBJECT_MATCH_FAILED;
    ret_state_code = search->second.link_state;
    return TableRetCode::SUCCESS;
  }


  TableRetCode get_p4object_link(const std::string& p4_name,
                                 T_P4Object_Unique val,
                                 LinkValueState<T_P4Object_Unique>& out) const {
    //std::string fqn = p4_name + "."+std::to_string(val);
    std::string fqn = p4_name + "."+boost::lexical_cast<std::string>(val);
    const auto search = fqn_to_linker_p4object_uid_map.find(fqn);
    if (search == fqn_to_linker_p4object_uid_map.end()) {
      return TableRetCode::P4_OBJECT_MATCH_FAILED;
    }
    out = search->second;
    return TableRetCode::SUCCESS;
  }

  std::unordered_set<T_P4Object_Unique> get_all_linker_uids() const {
    std::unordered_set<T_P4Object_Unique> uid_set;
    for (const auto& kv : linker_to_p4s_id_map) {
      uid_set.insert(kv.first);
    }
    return uid_set;
  }

/*
  TableRetCode remove_p4object_linker_object_mapping_state(
      std::string p4_name, T_P4Object_Unique val) {
    
  }
*/
  std::string to_string() {
    std::string str = "\nfqn_to_linker_p4object_uid_map:{";
    for (const auto& kv : fqn_to_linker_p4object_uid_map) {
      str += "["+kv.first+", "+kv.second.to_string()+"]"+"\n";
    }
    str += "} \n linker_to_p4s_id_map: {";
    for (const auto& kv : linker_to_p4s_id_map) {
      str += "["+boost::lexical_cast<std::string>(kv.first)+":(";
      for (const auto& v : kv.second) {
        str += v.to_string() +",";
      }
      str += ")]\n";
    }
    str += "}";
    return str;
  }



 protected:

  // P4-program's object's  FQN to linker's P4Object Id value
  // e.g. HeaderType mappings ->  p4_name.ipv4_t   Linker_ipv4_
  // e.g. Header Id mappings ->  p4_name.1   10
  // One FQN can map to one Linker Object
  std::unordered_map<std::string, LinkValueState<T_P4Object_Unique> > 
    fqn_to_linker_p4object_uid_map{};

  // reverse of above map
  // One linker Object  maps to multiple FQN, but one FQN from a P4_name.
  std::unordered_map<T_P4Object_Unique, 
                    std::unordered_set<LinkValueState<T_P4Object_Unique> > > 
                      linker_to_p4s_id_map{};


};

}  // namespace bm

namespace std {
template<typename TValue>
struct hash<bm::LinkValueState<TValue> >{
  using argument_type = bm::LinkValueState<TValue>;
  using result_type = std::size_t;
  result_type operator ()(argument_type const& key) const {
    //return std::hash<TValue>()(key.value);
    return std::hash<std::string>()(key.pname);
  }
};

}
#endif  // BM_BM_LINKER_COMMONS_H_
