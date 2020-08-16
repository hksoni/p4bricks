/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file linker.cpp

#include <istream>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>
#include <set>

#include <bm/bm_linker/linker_commons.h>
#include <bm/bm_linker/linker.h>

namespace bm {


Linker::Linker()
  : header_types_uid_table_(std::make_shared<HeaderTypeUIDTable> ()),
    header_uid_table_(std::make_shared<HeaderUIDTable> (header_types_uid_table_)),
    parser_linker(header_types_uid_table_, header_uid_table_) {
   
}

// TODO: Will have to do something for if the whole linking is successful then
// only modify linker mapping objects like  HeaderTypeUIDTable objects 
std::shared_ptr<P4Objects>
Linker::add_p4objects(const std::string& p4_name,
                      std::shared_ptr<P4Objects> p4objects) {
  TRACE_START;

  std::shared_ptr<P4Objects> merged_p4objects = std::make_shared<P4Objects> (
                                                  std::cout, true);

  ObjectLinkStateCode error_code;

  // initialize the extended data structures for new p4 program
  std::shared_ptr<P4ObjectsLinkerExt> p4objects_linker_ext = 
    std::make_shared<P4ObjectsLinkerExt> (p4_name, p4objects);
  p4objects_linker_ext->init_extended_objects();
  
  /**********************Linking Starts****************/
  // Add HeaderType objects and store the map with __name__ key
  error_code = init_header_types_linking(p4_name, p4objects->header_types_map);
  if (error_code != ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH) 
    return nullptr;
  header_types_uid_table_->get_linker_header_types_map(
      merged_p4objects->header_types_map);

  TRACE_PRINT(header_types_uid_table_->to_string());

  // Add Parser
  // Parser* merged_parser = nullptr;
  error_code = parser_linker.link_parser(p4objects_linker_ext, merged_p4objects);
//merged_p4objects->add_parser("parser", std::unique_ptr<Parser>(merged_parser));

  TRACE_PRINT("---------------after link_parser----------------");
  TRACE_PRINT(header_types_uid_table_->to_string());
  TRACE_PRINT(header_uid_table_->to_string());

  // Add Headers related objects
  header_uid_table_->get_linked_header_ids_map(merged_p4objects->header_ids_map);
  std::unordered_map<std::string, p4object_name_t> linked_header_name_type;
  header_uid_table_->get_linked_headers_name_type_map(linked_header_name_type);
  for (const auto& name_type : linked_header_name_type) {
    merged_p4objects->header_to_type_map[name_type.first] = 
      merged_p4objects->header_types_map[name_type.second].get();
  }

  p4object_ = merged_p4objects;
  p4objects_linker_ext_.reset(new P4ObjectsLinkerExt("Linker", merged_p4objects));
  p4objects_linker_ext_->init_extended_objects();
  parser_linker.reset_p4objects_linker_ext(p4objects_linker_ext_);
  TRACE_END;
  return p4object_;
}


ObjectLinkStateCode
Linker::init_header_types_linking(const std::string& p4_name,
        const std::unordered_map<std::string, std::unique_ptr<HeaderType> >& 
          header_types_map) { 
  return header_types_uid_table_->init_type_mapping(p4_name, 
                                                    header_types_map);
}



}
