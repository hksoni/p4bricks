/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file link_headers.h

#ifndef BM_BM_LINKER_HEADERS_H_
#define BM_BM_LINKER_HEADERS_H_

#include <unordered_map> 
#include <unordered_set> 
#include <vector>
#include <string>
#include <memory>

#include <bm/bm_sim/headers.h>
#include <bm/bm_linker/linker_commons.h>

namespace bm {


class HeaderTypeUIDTable {

 public:
  explicit HeaderTypeUIDTable();

  ObjectLinkStateCode init_type_mapping(
      const std::string& p4_name,
      const std::unordered_map<std::string, std::unique_ptr<HeaderType> >& 
            header_types_map);

  ObjectLinkStateCode get_header_type_mapping(const std::string& p4_name, 
                        const p4object_name_t& header_type_name,
                        const p4object_name_t& rp_header_type_name) const;


  ObjectLinkStateCode get_linker_header_type(const std::string& p4_name, 
                        const p4object_name_t& header_type_name,
                        p4object_name_t& linker_type_name) const;


  // creates a new copy of all the HeaderTypes in the table
  void get_linker_header_types_map(
      std::unordered_map<p4object_name_t, std::unique_ptr<HeaderType> >&) const;

  // Disabling copying, but allow moving
  HeaderTypeUIDTable(const HeaderTypeUIDTable &other) = delete;
  HeaderTypeUIDTable &operator=(const HeaderTypeUIDTable &) = delete;

  std::string to_string() {
    return header_types_link_table.to_string();
  }

 private:
  
  void get_header_type_uid_from_field_seq(const HeaderType* header_type, 
                                          std::string& uid_name);

  HeaderType* dc_header_type(const std::string name, p4object_id_t id,
                             const HeaderType* obj) const;

  p4object_id_t get_next_id() {
    return header_type_id++;
  }
 
 private:
  // header_type name mapping
  LinkerUIDTableGeneric<p4object_name_t> header_types_link_table;
  // independent object header_type
  std::unordered_map<p4object_name_t, std::unique_ptr<HeaderType> > 
    linker_header_types{};

  p4object_id_t header_type_id{0};

};



//TOOD:: managing PHV factory for each new P4object
class HeaderUIDTable {

 public:
  explicit HeaderUIDTable(std::shared_ptr<HeaderTypeUIDTable> table)
    : header_type_uid_table_(table){}

  ObjectLinkStateCode link_header_ids(const std::string& p4_name,
                                      header_id_t header_id,
                                      header_id_t rp_header_id);

  ObjectLinkStateCode link_new_header_id(const std::string& p4_name,
                                         const p4object_name_t& header_name,
                                         header_id_t header_id,
                                         const p4object_name_t& type_name,
                                         header_id_t& rp_header_id_out);

  void get_linked_header_ids_map(
      std::unordered_map<std::string, header_id_t>& out) const;

  void get_linked_headers_name_type_map(
      std::unordered_map<std::string, p4object_name_t>& out) const;

  // Disabling copying, but allow moving
  HeaderUIDTable(const HeaderUIDTable &other) = delete;
  HeaderUIDTable &operator=(const HeaderUIDTable &) = delete;

  std::string to_string() {
    return header_ids_link_table.to_string();
  }
 private:
  header_id_t get_new_header_id(const std::unordered_set<header_id_t>& ids);

 private:
  std::shared_ptr<HeaderTypeUIDTable> header_type_uid_table_;

  LinkerUIDTableGeneric<header_id_t> header_ids_link_table;

  // <program_name>.<header_name> to <header id in linker P4object>
  std::unordered_map<std::string, header_id_t> linker_header_ids_map{};
  std::unordered_map<header_id_t, p4object_name_t> 
    linker_header_id_type_name_map{};
};


}  // namespace bm

#endif  // BM_BM_LINKER_HEADERS_H_
