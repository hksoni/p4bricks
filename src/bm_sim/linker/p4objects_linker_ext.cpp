/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file p4objects_linker_ext.cpp

#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/debugger.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_linker/p4objects_linker_ext.h>



namespace bm {

P4ObjectsLinkerExt::P4ObjectsLinkerExt(std::string program_name, 
                                       std::shared_ptr<P4Objects> objs)
                                      : p4_program_name(program_name),
                                        p4objects(objs) {
}

const std::string&
P4ObjectsLinkerExt::get_p4_name() const {
  return p4_program_name;
}

void
P4ObjectsLinkerExt::init_extended_objects() {

  TRACE_START;
  // Creates a map with header is as a key
  init_header_id_to_header_type_map(p4objects->header_ids_map,
                                    p4objects->header_to_type_map);
  expanded_parser = expand_all_sub_parser_calls(p4objects->parsers);
  bfs_topological_ordering_of_parser_states(expanded_parser->init_state);
  TRACE_END;
}

const Parser* 
P4ObjectsLinkerExt::expand_all_sub_parser_calls( 
    const std::unordered_map<std::string, std::unique_ptr<Parser> >& 
        p4objects_parser_map) {
  TRACE_START;
  if (p4objects_parser_map.empty())
    return nullptr;
  const auto iter = p4objects_parser_map.cbegin();
  TRACE_END;
  return iter->second.get();
}


void
P4ObjectsLinkerExt::init_header_id_to_header_type_map(
    const std::unordered_map<std::string, header_id_t>& name_to_id,
    const std::unordered_map<std::string, HeaderType *>& name_to_header_type) {

  TRACE_START;
  max_seen_header_id = 0;
  for (const auto &name_id : name_to_id) {
    if (max_seen_header_id < name_id.second)
      max_seen_header_id = name_id.second;
    header_id_name_map[name_id.second] = name_id.first;
    const auto name_type = name_to_header_type.find(name_id.first);
    header_id_type_map[name_id.second] = name_type->second;
  }
  TRACE_PRINT_MAP_UDT_VAL(header_id_type_map, get_name());
  TRACE_END;
}

const HeaderType*
P4ObjectsLinkerExt::get_header_type_of_header_id(header_id_t id) const {
  auto search = header_id_type_map.find(id);
  if (search == header_id_type_map.end())
    return nullptr;
  return search->second;
}

p4object_name_t 
P4ObjectsLinkerExt::get_header_name(header_id_t id) const {
  const auto search = header_id_name_map.find(id);
  return search->second;
}

std::unordered_set<header_id_t>
P4ObjectsLinkerExt::get_header_ids() const {
  std::unordered_set<header_id_t>  ids;
  for (const auto& id_type : header_id_type_map) {
    ids.insert(id_type.first);
  }
  return ids;
}

/*
const HeaderType*
P4ObjectsLinkerExt::get_header_type_of_header_stack_id(header_stack_id_t id) const {

  return nullptr;
}
*/


void
P4ObjectsLinkerExt::bfs_topological_ordering_of_parser_states(
                    const ParseState* init_state) {

  TRACE_START;
  std::unordered_map<std::string, unsigned> name_degree_state_map {};
  std::unordered_map<std::string, const ParseState*> name_parse_state_map {};
  std::unordered_map<std::string, std::vector<std::string> > adjacency_map {};

  // indegree map and adjacency map for graph representation
  // computing indegree of all the states-nodes
  for (const auto& state_uptr : p4objects->parse_states) {
    if (max_seen_state_id < state_uptr->get_id())
      max_seen_state_id = state_uptr->get_id();

    //TRACE_PRINT(state_uptr->get_name());
    name_parse_state_map.insert(std::make_pair(state_uptr->get_name(), 
                                               state_uptr.get()));
    std::vector<std::string> neighbours{};
    for (const auto& switch_case : state_uptr->parser_switch) {
      std::string s_name;
      if (const ParseSwitchCase* parse_switch_case = 
          dynamic_cast<const ParseSwitchCase*> (switch_case.get())) {
        s_name = parse_switch_case->next_state->get_name();
      }
      else if (const ParseSwitchCaseWithMask* parse_switch_case_mask =
          dynamic_cast<const ParseSwitchCaseWithMask*> (switch_case.get())) {
        s_name = parse_switch_case_mask->next_state->get_name();
      } else {
        continue;
        //log.... not supported
      }
      ++name_degree_state_map[s_name];
      neighbours.push_back(s_name);
    }
    //TRACE_PRINT_VECTOR(neighbours);
    adjacency_map.insert(std::make_pair(state_uptr->get_name(), neighbours));
  }

  //TRACE_PRINT_MAP_UDT_VAL(name_parse_state_map, get_name());
  
  std::list<std::string> state_queue = {init_state->get_name()};
  std::vector<std::string> topological_order {};

  while (!state_queue.empty()) {
    std::string current_state = state_queue.front();
    state_queue.pop_front();
    topological_order.push_back(current_state);
    std::vector<std::string>& neighbours = adjacency_map[current_state];
    for (const auto& state : neighbours) {
      --name_degree_state_map[state];
      if (name_degree_state_map[state] == 0)
        state_queue.push_back(state);
    }
  }

  for (const auto& state_name : topological_order) {
    ordered_parse_states.push_back(name_parse_state_map[state_name]);
  }

  TRACE_PRINT_CONST("Topological Order");
  TRACE_PRINT_VECTOR(topological_order);

  TRACE_END;
}

}  // namespace bm
