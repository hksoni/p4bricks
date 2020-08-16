/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file link_headers.cpp

#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/debugger.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_sim/expressions.h>
#include <bm/bm_linker/link_headers.h>

namespace bm {

HeaderTypeUIDTable::HeaderTypeUIDTable(){

}

// Iterates through the header_types_map.
// computes the linker_uid for each type and  adds the mapping between
// fully-qualified-name(FQN) and computed linker uid.
// FQN is <p4_name>.<header_type name>
ObjectLinkStateCode
HeaderTypeUIDTable::init_type_mapping(
    const std::string& p4_name,
    const std::unordered_map<std::string, std::unique_ptr<HeaderType> >& 
          p4objects_header_types_map) {

  TRACE_START;
  for (const auto &name_typeptr: p4objects_header_types_map){ 
    // get uid
    std::string linker_uid_str;
    std::string type_name = name_typeptr.first;
    const HeaderType* header_type = name_typeptr.second.get();
    get_header_type_uid_from_field_seq(header_type, linker_uid_str);

    auto ret_code = header_types_link_table.insert_p4objects_link(
                      p4_name, type_name, linker_uid_str,
                      ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH);

    switch (ret_code) {
      case TableRetCode::P4_OBJECT_EXIST:
        TRACE_PRINT_CONST(p4_name+"."+type_name+" already ecist");
        break;
      case TableRetCode::LINK_OBJECT_P4_NAME_EXIST:
        TRACE_PRINT_CONST(linker_uid_str +  "already exist");
        break;
      case TableRetCode::SUCCESS: {
        std::unique_ptr<HeaderType> new_obj(dc_header_type(linker_uid_str, 
                                                           get_next_id(), 
                                                           header_type)); 

        linker_header_types[linker_uid_str] = std::move(new_obj);
        TRACE_PRINT_CONST(p4_name+"."+type_name+" mapped to "+linker_uid_str);
        break;
      }
      default:
        TRACE_PRINT_CONST("Unexpected return from insert_p4objects_link");
    }
  }
  TRACE_END;
  return ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH;
}


// Checks if header_type_name is mapped to rp_header_type_name in
// p4object_name_to_linker_uid_map
ObjectLinkStateCode
HeaderTypeUIDTable::get_header_type_mapping(const std::string& p4_name,
                      const p4object_name_t& header_type_n, 
                      const p4object_name_t& rp_header_type_n) const {

  ObjectLinkStateCode link_state_code;
  auto ret_code = header_types_link_table.get_p4objects_link_state(p4_name, 
                  header_type_n, rp_header_type_n, link_state_code);
  switch (ret_code) {
    case TableRetCode::P4_OBJECT_MATCH_FAILED:
    case TableRetCode::LINK_OBJECT_MATCH_FAILED:
      return ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH_FAILED;
    case TableRetCode::SUCCESS:
      return link_state_code;
    default:
      TRACE_PRINT_CONST("Unexpected return from get_p4objects_link_state");
      return ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH_FAILED;
  }
}


ObjectLinkStateCode 
HeaderTypeUIDTable::get_linker_header_type(const std::string& p4_name, 
                      const p4object_name_t& header_type_name,
                      p4object_name_t& rp_header_type_name) const {
  LinkValueState<p4object_name_t> out;
  auto ret_code = header_types_link_table.get_p4object_link(
                    p4_name, header_type_name, out);
  if (ret_code == TableRetCode::SUCCESS) {
    rp_header_type_name = out.value;
    return out.link_state;
  }
  return ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH_FAILED;
}


void
HeaderTypeUIDTable::get_linker_header_types_map(
                    std::unordered_map<p4object_name_t, 
                      std::unique_ptr<HeaderType> >& map_obj) const { 
  for (const auto& kv : linker_header_types) {
    std::unique_ptr<HeaderType> obj(dc_header_type(kv.second->get_name(),
                                                   kv.second->get_id(),
                                                   kv.second.get()));
    map_obj[kv.second->get_name()] = std::move(obj);
  }
}

// Computes the linker uid from the definition of the header_type and 
// sets the uid in uid_name out parameter
// If, non-supported header_type is  found to be linked, return error.
void
HeaderTypeUIDTable::get_header_type_uid_from_field_seq(
                    const HeaderType *header_type, std::string& uid_name) {

  // TRACE_START;
  for (const HeaderType::FInfo& finfo : header_type->fields_info) {
    if (finfo.is_VL) {
      // TODO: get field's offset and width on which variable 
      // length expression depends
      uid_name += std::to_string(header_type->VL_max_header_bytes) + "-";
    } else {
      uid_name += std::to_string(finfo.bitwidth) + "-";
    }
  }
  // TRACE_END;
}


//TODO: Copy std::unique_ptr<VLHeaderExpression> VL_expr_raw
HeaderType* 
HeaderTypeUIDTable::dc_header_type(const std::string name, p4object_id_t id,
                                   const HeaderType* obj) const {
  HeaderType* header_type = new HeaderType(name, id);
  header_type->fields_info = obj->fields_info; 
  header_type->VL_offset = obj->VL_offset;
  header_type->VL_max_header_bytes = obj->VL_max_header_bytes;
  //header_type->VL_expr_raw = nullptr;
  return header_type;
}


ObjectLinkStateCode
HeaderUIDTable::link_header_ids(const std::string& p4_name, 
                                header_id_t header_id,
                                header_id_t rp_header_id) {

  auto ret_code = header_ids_link_table.insert_p4objects_link(p4_name,
                    header_id, rp_header_id, ObjectLinkStateCode::HEADER_LINKED);

  std::string msg1 = p4_name+"."+std::to_string(header_id)+" already exist";
  std::string msg2 = "Linker id "+std::to_string(rp_header_id)
                    +" already mapped to "+p4_name;
  std::string msg3 = p4_name+"."+std::to_string(header_id)+" linked to "
                    + "Linker id "+std::to_string(rp_header_id);
  switch (ret_code) {
    case TableRetCode::P4_OBJECT_EXIST:
        bm::Logger::get()->trace(msg1); break;
    case TableRetCode::LINK_OBJECT_P4_NAME_EXIST:
        bm::Logger::get()->trace(msg2); break;
    case TableRetCode::SUCCESS:
        bm::Logger::get()->trace(msg3);
        return ObjectLinkStateCode::HEADER_LINKED;
    default:
        bm::Logger::get()->trace("Unexpected return from insert_p4objects_link");
  }
  return ObjectLinkStateCode::HEADER_LINK_FAILED;
}

ObjectLinkStateCode
HeaderUIDTable::link_new_header_id(const std::string& p4_name,
                                   const p4object_name_t& header_name, 
                                   header_id_t header_id, 
                                   const p4object_name_t& header_type_name,
                                   header_id_t& hint_header_id_out) {
  p4object_name_t fqn = p4_name+"."+std::to_string(header_id);
  p4object_name_t fq_name = p4_name+"."+header_name;

  auto header_ids_set = header_ids_link_table.get_all_linker_uids();
  if (header_ids_set.find(hint_header_id_out) != header_ids_set.end()) {
    hint_header_id_out = get_new_header_id(header_ids_set); 
  }
  auto ret = header_ids_link_table.insert_p4objects_link(p4_name, header_id,
               hint_header_id_out, ObjectLinkStateCode::HEADER_LINKED);
  switch (ret) {
    case TableRetCode::SUCCESS:{
      TRACE_PRINT_CONST(fqn+" mapped to "+std::to_string(hint_header_id_out));
      linker_header_ids_map[fq_name] = hint_header_id_out;
      p4object_name_t linked_h_type_name;
      header_type_uid_table_->get_linker_header_type(p4_name, header_type_name,
                                                     linked_h_type_name);
      linker_header_id_type_name_map[hint_header_id_out] = linked_h_type_name;
      return ObjectLinkStateCode::HEADER_LINKED;
    }
    case TableRetCode::P4_OBJECT_EXIST:
      TRACE_PRINT_CONST(fqn+" already exist");
      break;
    case TableRetCode::LINK_OBJECT_P4_NAME_EXIST:
      TRACE_PRINT_CONST("return not expected");
      break;
    default:
      TRACE_PRINT_CONST("Unexpected return from \
          header_ids_link_table.insert_p4objects_link");
  }
  return ObjectLinkStateCode::HEADER_LINK_FAILED;
}


void 
HeaderUIDTable::get_linked_header_ids_map(
                std::unordered_map<std::string, header_id_t>& out) const {
  out = linker_header_ids_map;
}


void
HeaderUIDTable::get_linked_headers_name_type_map(
                std::unordered_map<std::string, p4object_name_t>& out) const {
  for (const auto& name_id : linker_header_ids_map) {
    auto search  = linker_header_id_type_name_map.find(name_id.second);
    out[name_id.first] = search->second;
  }
}




header_id_t
HeaderUIDTable:: get_new_header_id(const std::unordered_set<header_id_t>& ids) {
  header_id_t i = 0;
  while(1) {
    auto search  = ids.find(i);
    if (search == ids.end())
      return i;
    else
      i++;
  }
}



} // namespace bm
