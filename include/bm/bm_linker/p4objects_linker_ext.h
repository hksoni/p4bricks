/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file p4objects_linker_ext.h

#ifndef BM_BM_P4OBJECTS_LINKER_EXT_H_
#define BM_BM_P4OBJECTS_LINKER_EXT_H_

#include <vector>
#include <string>
#include <set>
#include <unordered_map> 
#include <memory> 
#include <list>

#include <bm/bm_sim/headers.h>
#include <bm/bm_sim/P4Objects.h>

#include <bm/bm_linker/linker_commons.h>

namespace bm {


class P4ObjectsLinkerExt {

 public:
  P4ObjectsLinkerExt(std::string program_name, std::shared_ptr<P4Objects> objs);

  void init_extended_objects();

  const std::string& get_p4_name() const;

  const HeaderType* get_header_type_of_header_id(header_id_t id) const;

  const HeaderType* get_header_type_of_header_stack_id(
      header_stack_id_t id) const;

  const std::vector<const ParseState*>& get_ordered_parse_states() const {
    return ordered_parse_states; 
  }

  header_id_t get_max_header_id() const {
    return max_seen_header_id;
  }

  header_id_t get_max_state_id() const {
    return max_seen_state_id;
  }

  const Parser* get_expanded_parser() const {
    return expanded_parser;
  }

  std::shared_ptr<P4Objects> get_p4objects() {
    return p4objects;
  }

  std::unordered_set<header_id_t> get_header_ids() const;

  p4object_name_t get_header_name(header_id_t id) const;

  // Disabling copying, but allow moving
  P4ObjectsLinkerExt(const P4ObjectsLinkerExt &other) = delete;
  P4ObjectsLinkerExt &operator=(const P4ObjectsLinkerExt &) = delete;

 private:
  void init_header_id_to_header_type_map(
      const std::unordered_map<std::string, header_id_t>& name_to_id_map,
      const std::unordered_map<std::string, HeaderType *>& name_to_type_map);

  void bfs_topological_ordering_of_parser_states(const ParseState* init_state);

  const Parser* expand_all_sub_parser_calls(
      const std::unordered_map<std::string, std::unique_ptr<Parser> >& 
        p4objects_parser_map);

 private:
  std::string p4_program_name;
  std::shared_ptr<P4Objects> p4objects{nullptr};

  std::unordered_map<header_id_t, HeaderType*> header_id_type_map{};
  std::unordered_map<header_id_t, p4object_name_t> header_id_name_map{};
  /*
  std::unordered_map<header_stack_id_t, HeaderType*>
    header_stack_id_to_header_type_map{};
  */


  const Parser* expanded_parser{};
  
  std::vector<const ParseState* > ordered_parse_states{};

  // better to take sets
  header_id_t max_seen_header_id;
  header_id_t max_seen_state_id;

};

}  // namespace bm

#endif  // BM_BM_P4OBJECTS_LINKER_EXT_H_
