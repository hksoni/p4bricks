/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file link_parsers.cpp


#include <queue>
#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/debugger.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_linker/link_parsers.h>

namespace bm {



ObjectLinkStateCode
LinkParsers::link_parser(std::shared_ptr<P4ObjectsLinkerExt> p4objects_ext,
                         std::shared_ptr<P4Objects> merge_p4objects) {
  TRACE_START;
  ObjectLinkStateCode ret_code;

  // TODO: appropriate name and id generation for parser and errorcode merging
  ErrorCodeMap* error_code = new ErrorCodeMap(
      p4objects_ext->get_p4objects()->get_error_codes());
  Parser* merged_parser = new Parser("parser", 0, error_code);

  linked_parse_state.clear();

  const std::vector<const ParseState*>& new_ordered_states = 
    p4objects_ext->get_ordered_parse_states();

  // Assuming 0 states are init and they start with "start" state
  // TODO: here, align start state of both program if necessary.

  ParseStateExt* init_parse_state = nullptr;
  if (p4objects_linker_ext_ != nullptr) {
    const std::vector<const ParseState*>& rp_ordered_states = 
      p4objects_linker_ext_->get_ordered_parse_states();
    ret_code = map_ordered_parser_states(new_ordered_states, rp_ordered_states,
                                         p4objects_ext.get());

    init_parse_state = linked_parse_state[rp_ordered_states[0]->get_name()];

    TRACE_PRINT_MAP_UDT_VAL(linked_parse_state, get_new_parse_state()->get_name());
    ret_code = merge_parser_states_switch_cases(new_ordered_states[0],
                                                rp_ordered_states[0],
                                                init_parse_state);
  } else {
    std::vector<const ParseState*> rp_ordered_states;
    ret_code = map_ordered_parser_states(new_ordered_states, rp_ordered_states,
                                         p4objects_ext.get());

    std::string state_name = p4objects_ext->get_p4_name() + "."
                            + new_ordered_states[0]->get_name();
    init_parse_state = linked_parse_state[state_name];

    if (!init_parse_state) {
      TRACE_PRINT_CONST("Not null");
    }
    ret_code = merge_parser_states_switch_cases(new_ordered_states[0], nullptr,
                                                init_parse_state);
  }

  merged_parser->set_init_state(init_parse_state->get_new_parse_state());

  // TODO: move it in P4ObjectsLinkerExt before clearing
  merge_p4objects->add_parser("parser", std::unique_ptr<Parser>(merged_parser));
  for (const auto& name_state : linked_parse_state) {
    merge_p4objects->parse_states.emplace_back(
        name_state.second->get_new_parse_state());
  }

  TRACE_PRINT_MAP_UDT_VAL(linked_parse_state, get_new_parse_state()->get_name());
  linked_parse_state.clear();
  TRACE_END;
  return ret_code;
}


ObjectLinkStateCode
LinkParsers::map_ordered_parser_states(
             const std::vector<const ParseState* >& parse_states,
             const std::vector<const ParseState* >& rp_parse_states,
             P4ObjectsLinkerExt* p4objects_linker_ext) {

  TRACE_START;
  std::vector<bool> parse_state_linked(parse_states.size(), false);
  ObjectLinkStateCode ret_code = ObjectLinkStateCode::STATE_MATCHED;
  const std::string& p_name = p4objects_linker_ext->get_p4_name();

  ParseStateExt* new_parse_state_ext = nullptr;

  bool linked = false;

  // First, map all the runnig parser state, (the parser for which symbols are
  // already in the Linker's UID tables)
  // Then map all the remaining states of the new parser. 
  // New IDs need to be added for them

  // Mapping alll the states of running parser
  for (const auto& rp_parse_state : rp_parse_states) {
    linked = false;
    const std::string& rp_state_name = rp_parse_state->get_name();
    // Try to link running parser state to a state from new parser
    for (size_t i = 0; i < parse_states.size(); i++) {
      if (parse_state_linked[i])
        continue; // if the state is already linked 
      const ParseState* parse_state = parse_states[i];
      // id and name in new_parse_state_ext will be from rp_parse_state
      new_parse_state_ext = new ParseStateExt(rp_state_name, 
                                              rp_parse_state->get_id(), 
                                              UIDTableLinkOPCode::MAP_EXISTING_ID,
                                              p4objects_linker_ext);

      ret_code = can_link_parser_states(parse_state, rp_parse_state, 
                                        new_parse_state_ext);
      if (ret_code == ObjectLinkStateCode::STATE_MATCHED) {
        linked = true;
        set_parse_states_link_code(parse_state, rp_parse_state, p_name, ret_code);
        linked_parse_state[rp_state_name] = new_parse_state_ext;
        parse_state_linked[i] = true;
        TRACE_PRINT_CONST(rp_state_name+" matched to "+parse_state->get_name());
        break;
      } else { // deleting the object and continuing the loop
        delete new_parse_state_ext;
      }
    }
    if (!linked) {// copy running parse state
      TRACE_PRINT_CONST("Copying - "+ rp_parse_state->get_name());
      new_parse_state_ext = new ParseStateExt(rp_state_name, 
                                              rp_parse_state->get_id(),
                                              UIDTableLinkOPCode::NO_OPS,
                                              p4objects_linker_ext);
      add_parser_state(rp_parse_state, new_parse_state_ext);
      linked_parse_state[rp_state_name] = new_parse_state_ext;
      TRACE_PRINT(rp_state_name)
    }
  }

  for (size_t i = 0; i < parse_states.size(); i++) {
    if (parse_state_linked[i])
      continue;
    const ParseState* parse_state = parse_states[i];
    TRACE_PRINT_CONST("Adding - "+ parse_state->get_name());
    const std::string& fqn_state_name = p_name+"."+parse_state->get_name();
    new_parse_state_ext = new ParseStateExt(fqn_state_name, get_new_state_id(),
                                            UIDTableLinkOPCode::INSERT_NEW_ID,
                                            p4objects_linker_ext);
    add_parser_state(parse_state, new_parse_state_ext);
    linked_parse_state[fqn_state_name] = new_parse_state_ext;
    // this adds fqn as linked id of the state
    add_parse_state_link(parse_state, p_name, ObjectLinkStateCode::STATE_MATCHED);
  }
  // temporary return
  

  TRACE_PRINT_MAP_UDT_VAL(linked_parse_state,get_new_parse_state()->get_name());

  TRACE_PRINT_CONST("\n----- Printing State link table -----");
  TRACE_PRINT(parse_state_link_table.to_string());
  TRACE_PRINT_CONST("\n----- Printing Header link table -----");
  TRACE_PRINT(header_uid_table_->to_string());

  TRACE_END;
  return ObjectLinkStateCode::STATE_MATCHED;
}


//! Verify is both states are processing same header types and
//! have same select field, then only merging(linking) of the states 
//! are possible.
ObjectLinkStateCode
LinkParsers::can_link_parser_states(const ParseState *parse_state,
                                    const ParseState *rp_parse_state,
                                    ParseStateExt* parse_state_ext) {
  //TRACE_START;
  ObjectLinkStateCode ret_code;
  //TRACE_PRINT_CONST("Trying "+parse_state->get_name()+" - "+
  //    rp_parse_state->get_name());
  // Merge parse_ops, new parse_ops will be added into  new_parse_state
  ret_code = merge_parser_ops(parse_state->parser_ops, 
                              rp_parse_state->parser_ops,
                              parse_state_ext);

  //TRACE_PRINT(Enums::to_string(ret_code));
  // In case of failure, withdraw merging and return error
  if (ret_code == ObjectLinkStateCode::STATE_HEADER_OPS_LINK_FAILED) {
    return ObjectLinkStateCode::STATE_LINKING_FAILED;
  }
  // Match the switch key.
  // As of now no complex arithmatic is merged,
  // just the switch key field is matched
  ParseSwitchKeyBuilder merged_key_builder;
  ret_code = link_parse_switch_key(parse_state->key_builder, 
                                   rp_parse_state->key_builder,
                                   merged_key_builder,
                                   parse_state_ext);
  //TRACE_PRINT(Enums::to_string(ret_code));
  if (ret_code != ObjectLinkStateCode::STATE_PARSE_SWITCH_KEY_MATCH) {
    return ObjectLinkStateCode::STATE_LINKING_FAILED;
  }

  ParseState* new_parse_state = parse_state_ext->get_new_parse_state();
  new_parse_state->set_key_builder(merged_key_builder);

  const std::string& p_name = parse_state_ext->get_p4_name();
  for (const auto& kv : parse_state_ext->get_header_ids_map()) {
    header_uid_table_->link_header_ids(p_name, kv.first, kv.second);
  }
  //TRACE_PRINT_CONST("States Matched")
  TRACE_END;
  return ObjectLinkStateCode::STATE_MATCHED;
}


ObjectLinkStateCode
LinkParsers::add_parser_state(const ParseState *ps, 
                              ParseStateExt* parse_state_ext) {
  ObjectLinkStateCode ret_code = add_parser_ops(ps->parser_ops, parse_state_ext);
  ParseState* new_parse_state = parse_state_ext->get_new_parse_state();
  new_parse_state->set_key_builder(ps->key_builder);

  return ret_code;
}


// Match the switch cases.
// For now just hexstr without mask
// Create a map of <key, next_state> to make union of ParserSwitchCase easy
// TODO: create another map for with mask switch case and 
// think if any magic can be done...
ObjectLinkStateCode
LinkParsers::merge_parser_states_switch_cases(const ParseState *parse_state,
                                              const ParseState *rp_parse_state,
                                              ParseStateExt* parse_state_ext) {
  TRACE_START;
  std::string  dbg = std::string(__func__);
  dbg += "(parse_state=";
  if (parse_state != nullptr)
    dbg += parse_state->get_name();
  else
    dbg += "nullptr";

  dbg += ", rp_parse_state=";
  if (rp_parse_state != nullptr)
    dbg += rp_parse_state->get_name();
  else
    dbg += "nullptr";

  dbg += ", parse_state_ext="+
    parse_state_ext->get_new_parse_state()->get_name()+")";
  TRACE_PRINT(dbg);

  if (parse_state_ext->switch_case_merged()) {
    TRACE_PRINT_CONST("state "+
        parse_state_ext->get_new_parse_state()->get_name()+" merged");
    return ObjectLinkStateCode::STATE_LINKED;
  }
  ParseState* new_parse_state = parse_state_ext->get_new_parse_state();
  const std::string& p_name = parse_state_ext->get_p4_name();
  ObjectLinkStateCode ret_code;

  SwitchCaseKeyStateMap rp_key_next_state_map;
  if (rp_parse_state != nullptr)
    get_switch_case_hexstr_key_state_map(rp_parse_state->parser_switch,
                                         rp_key_next_state_map);

  ParseStateExt* new_ps_next_state_ext;
  ParseState* next_parse_state = nullptr;
  const ParseState* ps_next_state = nullptr;;
  const ParseState* rp_ps_next_state = nullptr;

  if (parse_state != nullptr) {
    auto ps_iter = parse_state->parser_switch.cbegin();
    while (ps_iter != parse_state->parser_switch.cend()) {
      const ParseSwitchCase* ps = dynamic_cast<const ParseSwitchCase*>(
                                  ps_iter->get());
      if (!ps)
        return ObjectLinkStateCode::PARSER_LINKING_FAILED;
    
      ps_next_state = ps->next_state;
      rp_ps_next_state = nullptr;
      //const string& ps_next_state_name = ps_next_state->get_name();

      auto search = rp_key_next_state_map.find(ps->key);
      if (search != rp_key_next_state_map.end()) {
        //the switch-case at ps_iter matches in both the states
        rp_ps_next_state = search->second;
        
        //const string& rp_ps_next_state_name = rp_ps_next_state->get_name();
        // The rp_ps_next_state and ps_next_state must be mapped.
        ret_code = get_parse_states_link_code(ps_next_state, rp_ps_next_state, 
                                              p_name);

        if (ret_code != ObjectLinkStateCode::STATE_MATCHED)
          return ObjectLinkStateCode::PARSER_LINKING_FAILED;

        new_ps_next_state_ext = linked_parse_state[rp_ps_next_state->get_name()];
        // remove the element from the map
        rp_key_next_state_map.erase(search);
      }
      else {
        // Find the mapping of ps_next_state from table, compare its link state
        // with ObjectLinkStateCode::STATE_MATCHED, assign it to
        // new_ps_next_state_ext
        p4object_name_t link_state_name;

        ret_code = get_parse_state_link(ps_next_state, p_name, link_state_name);
        //TRACE_PRINT(link_state_name);

        if (ret_code != ObjectLinkStateCode::STATE_MATCHED)
          return ObjectLinkStateCode::PARSER_LINKING_FAILED;
        new_ps_next_state_ext = linked_parse_state[link_state_name];
      }
      ret_code = merge_parser_states_switch_cases(ps_next_state, 
                                                  rp_ps_next_state, 
                                                  new_ps_next_state_ext);
      next_parse_state = new_ps_next_state_ext->get_new_parse_state();
      new_parse_state->add_switch_case(ps->key, next_parse_state);
      if (ret_code != ObjectLinkStateCode::STATE_LINKED)
        return ret_code;
      ++ps_iter;
    }
  }

  // Iterate through remaining of the switch_case runnign parser state.
  // Find a link state from the states of new parser being added.
  for (auto& key_next_state : rp_key_next_state_map) {
    rp_ps_next_state = key_next_state.second;
    const std::string& rp_ps_next_state_name = rp_ps_next_state->get_name();
    new_ps_next_state_ext = linked_parse_state[rp_ps_next_state_name];
    ret_code = merge_parser_states_switch_cases(nullptr, 
                                                rp_ps_next_state, 
                                                new_ps_next_state_ext); 
    next_parse_state = new_ps_next_state_ext->get_new_parse_state();
    new_parse_state->add_switch_case(key_next_state.first, next_parse_state);
    if (ret_code != ObjectLinkStateCode::STATE_LINKED)
      return ret_code;
  }
  parse_state_ext->switch_case_merged(true);

  TRACE_END;
  return ObjectLinkStateCode::STATE_LINKED;
}


void
LinkParsers::get_switch_case_hexstr_key_state_map(
    const std::vector<std::unique_ptr<ParseSwitchCaseIface> >& parser_switch,
    SwitchCaseKeyStateMap& key_state_map) {

  auto ps_iter = parser_switch.cbegin();
  while(ps_iter !=  parser_switch.cend()){
    const ParseSwitchCase* ps = 
      dynamic_cast<const ParseSwitchCase*>(ps_iter->get());
    if (ps)
      key_state_map.emplace(std::make_pair(ps->key, ps->next_state));
    ++ps_iter;
  }
}


// TODO: handle other parser ops, like extract_vl, stack, verify,set etc;
// Build dependency graph among ParserOps
ObjectLinkStateCode
LinkParsers::merge_parser_ops(
            const std::vector<std::unique_ptr<ParserOp> >& parser_ops,
            const std::vector<std::unique_ptr<ParserOp> >& rp_parser_ops,
            ParseStateExt* parse_state_ext) {

  //TRACE_START;
  //Iterate through parser_ops of parse_state type cast and check the type
  auto parser_ops_it = parser_ops.cbegin();
  auto rp_parser_ops_it = rp_parser_ops.cbegin();
  ObjectLinkStateCode ret_code = ObjectLinkStateCode::STATE_HEADER_OPS_LINKED;

  while (parser_ops_it !=  parser_ops.cend()) {
      const ParserOp* parser_op = parser_ops_it->get();
      ParserOpTypes op_type = get_parser_op_type(parser_op);
      
    //rp_parser_ops_it = rp_parser_ops.cbegin();
    while (rp_parser_ops_it != rp_parser_ops.cend()) {
      const ParserOp* rp_parser_op = rp_parser_ops_it->get();
      ParserOpTypes rp_op_type = get_parser_op_type(rp_parser_op);

      // Check for matching operation type
      if (op_type < ParserOpTypes::EXTRACT_TYPE_MAX && 
          rp_op_type < ParserOpTypes::EXTRACT_TYPE_MAX) {
        if (op_type != rp_op_type) {
          // extract_type do not match, so HeaderType is surely diferent.
          // No need to check further, return  failure
          return ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH_FAILED;
        }
        // Checking HeaderTypes
        ret_code = link_extract_parser_op(parser_op, rp_parser_op, rp_op_type, 
                                          parse_state_ext);
        // TRACE_PRINT_CONST("link_extract_parser_op ret: "
        // +Enums::to_string(ret_code));
        if (ret_code != ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH) {
          return ObjectLinkStateCode::STATE_HEADER_OPS_LINK_FAILED;
        }
      }
      ++rp_parser_ops_it;
    }
    ++parser_ops_it;
  }
  //TRACE_END;
  return ret_code;
}


ObjectLinkStateCode
LinkParsers::add_parser_ops(
             const std::vector<std::unique_ptr<ParserOp> >& parser_ops,
             ParseStateExt* parse_state_ext) {

  ObjectLinkStateCode ret_code = ObjectLinkStateCode::STATE_HEADER_OPS_LINKED;
  ObjectLinkStateCode failure = ObjectLinkStateCode::STATE_HEADER_OPS_LINKED;
  const auto parser_ops_it = parser_ops.cbegin();
  while (parser_ops_it !=  parser_ops.cend()) {
      const ParserOp* parser_op = parser_ops_it->get();
      ParserOpTypes op_type = get_parser_op_type(parser_op);
      switch (op_type) {
        case ParserOpTypes::EXTRACT:
          ret_code = add_extract_parser_op_regular(parser_op, parse_state_ext);
          if (ret_code == failure)
            return failure;
          break;
        default:
          return ObjectLinkStateCode::STATE_HEADER_OPS_LINK_FAILED;
      }
  }
  return ret_code;
}

//TODO:: Add not supported error flags for other fields
ObjectLinkStateCode
LinkParsers::link_parse_switch_key(const ParseSwitchKeyBuilder& key_builder,
                                   const ParseSwitchKeyBuilder& rp_key_builder,
                                   ParseSwitchKeyBuilder& merged_key_builder,
                                   const ParseStateExt* parse_state_ext) {
  //TRACE_START;
  ObjectLinkStateCode ret_match_code = 
    ObjectLinkStateCode::STATE_PARSE_SWITCH_KEY_MATCH;
  ObjectLinkStateCode ret_fail_code = 
    ObjectLinkStateCode::STATE_PARSE_SWITCH_KEY_MATCH_FAILED;

  size_t vectors_size = key_builder.entries.size();
  size_t rp_vectors_size = rp_key_builder.entries.size();

  // TODO: restructure the logic
  if (vectors_size == 0 || rp_vectors_size == 0) { 
    if (vectors_size == 0) merged_key_builder = rp_key_builder;
    if (rp_vectors_size == 0) merged_key_builder = key_builder;
    for (size_t i = 0; i< vectors_size; ++i) {
      if (merged_key_builder.entries[i].tag != 
          ParseSwitchKeyBuilder::Entry::FIELD)
        return ret_fail_code;
      if (rp_vectors_size == 0) {
        header_id_t id = merged_key_builder.entries[i].field.header;       
        header_id_t rid = parse_state_ext->get_header_ids_map().find(id)->second;
        merged_key_builder.entries[i].field.header = rid;
      }
    }
    return ret_match_code;
  }

  if (vectors_size != rp_vectors_size)
    return ret_fail_code;
  for (size_t i = 0; i<vectors_size; ++i) {
    ParseSwitchKeyBuilder::Entry entry = key_builder.entries[i];
    ParseSwitchKeyBuilder::Entry rp_entry = rp_key_builder.entries[i];
    int bw = key_builder.bitwidths[i];
    int rp_bw = rp_key_builder.bitwidths[i];
    if (entry.tag != rp_entry.tag)
      return ret_fail_code;
    switch (entry.tag) {
      case ParseSwitchKeyBuilder::Entry::FIELD: {
        if ( (bw == rp_bw) && (entry.field.offset == rp_entry.field.offset) ) {
            merged_key_builder.entries.push_back(rp_entry);
            merged_key_builder.bitwidths.push_back(rp_bw);
        }
        break;
      }
      default:
        return ret_fail_code;
    }
  }
  //TRACE_END;
  return ret_match_code;
}


ObjectLinkStateCode
LinkParsers::link_extract_parser_op(const ParserOp* parser_op, 
                                    const ParserOp* rp_parser_op,
                                    ParserOpTypes extract_type_op,
                                    ParseStateExt* parse_state_ext) {
  switch (extract_type_op) {
    case ParserOpTypes::EXTRACT:
      return link_extract_parser_op_regular(parser_op, rp_parser_op, 
                                            parse_state_ext);
/*
    case ParserOpTypes::EXTRACT_VL:
      return link_extract_parser_op_VL(parser_op, rp_parser_op,
                                       parse_state_ext);
    case ParserOpTypes::EXTRACT_STACK:
      return link_extract_parser_op_stack(parser_op, rp_parser_op, 
                                          parse_state_ext);
*/
    default:
      return ObjectLinkStateCode::STATE_HEADER_OPS_LINK_FAILED;
  }
}


ObjectLinkStateCode 
LinkParsers::link_extract_parser_op_regular(const ParserOp* parser_op,
                                            const ParserOp* rp_parser_op,
                                            ParseStateExt* parse_state_ext) {
  //TRACE_START;
  const ParserOpExtract* parser_op_extract = 
    dynamic_cast<const ParserOpExtract*>(parser_op);
  const ParserOpExtract* rp_parser_op_extract = 
    dynamic_cast<const ParserOpExtract*>(rp_parser_op);
  
  ObjectLinkStateCode ret  = STATE_HEADER_OPS_LINK_FAILED;
  P4ObjectsLinkerExt* p4objects_linker_ext_temp = 
                      parse_state_ext->get_p4objects_linker_ext();
  ParseState* new_parse_state = parse_state_ext->get_new_parse_state();

  // redundant check, still keeping it
  if (parser_op_extract && rp_parser_op_extract) {
    // get id of header instance
    header_id_t header_id = parser_op_extract->header;
    header_id_t rp_header_id = rp_parser_op_extract->header;

    const HeaderType* header_type = 
      p4objects_linker_ext_temp->get_header_type_of_header_id(header_id);
    const HeaderType* rp_header_type = 
      p4objects_linker_ext_->get_header_type_of_header_id(rp_header_id);

    //Does linker mapping exist for header_type in header_type_uid_table_
    ret = header_type_uid_table_->get_header_type_mapping(
                                  parse_state_ext->get_p4_name(),
                                  header_type->get_name(),
                                  rp_header_type->get_name());

    TRACE_PRINT(Enums::to_string(ret));
    if (ret == ObjectLinkStateCode::HEADER_TYPE_DEF_MATCH) {
      // Map header id(instances)
      parse_state_ext->add_header_id_state_mapping(header_id, rp_header_id);
      // Add parser_op into new state
      new_parse_state->add_extract(rp_header_id);
    }
  }
  //TRACE_END;
  return ret;
}

ObjectLinkStateCode 
LinkParsers::add_extract_parser_op_regular(const ParserOp* parser_op,
                                           ParseStateExt* parse_state_ext) {
  const ParserOpExtract* parser_op_extract = 
    dynamic_cast<const ParserOpExtract*>(parser_op);

  ObjectLinkStateCode ret  = ObjectLinkStateCode::STATE_HEADER_OPS_LINKED;
  P4ObjectsLinkerExt* p4_ext = parse_state_ext->get_p4objects_linker_ext();
  std::string p4_name = parse_state_ext->get_p4_name();

  header_id_t header_id = parser_op_extract->header;
  header_id_t new_header_id = header_id;

  UIDTableLinkOPCode uid_link_op = parse_state_ext->get_uid_table_link_code();
  if (uid_link_op == UIDTableLinkOPCode::INSERT_NEW_ID) {
    p4object_name_t header_name = p4_ext->get_header_name(header_id);
    const HeaderType* header_type = p4_ext->get_header_type_of_header_id(
                                    header_id);
    header_uid_table_->link_new_header_id(p4_name, header_name, header_id, 
                                          header_type->get_name(), 
                                          new_header_id);
  }
  parse_state_ext->get_new_parse_state()->add_extract(new_header_id);
  return ret;
}

/*
ObjectLinkStateCode
LinkParsers::link_extract_parser_op_stack(const ParserOp* parser_op, 
                                          const ParserOp* rp_parser_op,
                                          ParseStateExt* parse_state_ext) {
  const ParserOpExtractStack* parser_op_extract_stack = 
    dynamic_cast<const ParserOpExtractStack*>(parser_op);
  const ParserOpExtractStack* rp_parser_op_extract_stack = 
    dynamic_cast<const ParserOpExtractStack*>(rp_parser_op);

  parse_state_ext = nullptr;
  if(parser_op_extract_stack && rp_parser_op_extract_stack) {
    return ObjectLinkStateCode::LINKING_FAILED;
  }
  return ObjectLinkStateCode::LINKING_FAILED;
}


ObjectLinkStateCode
LinkParsers::link_extract_parser_op_VL(const ParserOp* parser_op, 
                                       const ParserOp* rp_parser_op,
                                       ParseStateExt* parse_state_ext) {
  const ParserOpExtractVL* parser_op_extract_vl = 
    dynamic_cast<const ParserOpExtractVL*>(parser_op);
  const ParserOpExtractVL* rp_parser_op_extract_vl = 
    dynamic_cast<const ParserOpExtractVL*>(rp_parser_op);
  parse_state_ext = nullptr;

  if (parser_op_extract_vl && rp_parser_op_extract_vl) {
    return ObjectLinkStateCode::LINKING_FAILED;
  }
  return ObjectLinkStateCode::LINKING_FAILED;
}
*/


ParseStateExt::ParserOpTypes
LinkParsers::get_parser_op_type(const ParserOp* parser_op) {

  const ParserOpExtract* o = dynamic_cast<const ParserOpExtract*>(parser_op);
  if (o)
    return ParseStateExt::ParserOpTypes::EXTRACT;

  /*
  if (const ParserOpExtractStack* d = 
      dynamic_cast<const ParserOpExtractStack*>(parser_op))
    return ParseStateExt::ParserOpTypes::EXTRACT_STACK;

  if (const ParserOpExtractVL* d =
      dynamic_cast<const ParserOpExtractVL*>(parser_op))
    return ParseStateExt::ParserOpTypes::EXTRACT_VL;

  */
  return ParseStateExt::ParserOpTypes::UNSUPPORTED_TYPE;

}

ObjectLinkStateCode
LinkParsers::add_parse_state_link(const ParseState* parse_state, 
                                  const std::string& p4_name,
                                  ObjectLinkStateCode state) {
  const std::string& state_name = parse_state->get_name();
  std::string fqn_state_name = p4_name+"."+state_name;
  auto p4object_name_set = parse_state_link_table.get_all_linker_uids();

  auto search = p4object_name_set.find(state_name);
  if (search != p4object_name_set.end())
    return ObjectLinkStateCode::STATE_LINKING_FAILED;

  return set_parse_states_link_code(state_name, fqn_state_name, p4_name, state);
}

ObjectLinkStateCode
LinkParsers::get_parse_state_link(const ParseState* parse_state,
                                  const std::string& p4_name,
                                  std::string& out_name) {
  // TRACE_START;
  LinkValueState<p4object_name_t>  p4name_val_state;
  auto ret_code = parse_state_link_table.get_p4object_link(
                    p4_name, parse_state->get_name(), p4name_val_state);
  if (ret_code == TableRetCode::SUCCESS) {
    out_name = p4name_val_state.value;
    // TRACE_PRINT(out_name);
    return p4name_val_state.link_state;
  }
  // TRACE_END;
  return ObjectLinkStateCode::STATE_LINKING_FAILED;
}



ObjectLinkStateCode
LinkParsers::get_parse_states_link_code(const ParseState* parse_state, 
                                        const ParseState* rp_parse_state,
                                        const std::string& p4_name) {

  ObjectLinkStateCode link_state;
  const std::string& state_name = parse_state->get_name();
  const std::string& rp_state_name = rp_parse_state->get_name();

  auto ret_code = parse_state_link_table.get_p4objects_link_state(p4_name, 
                    state_name, rp_state_name, link_state);
  if (ret_code != TableRetCode::SUCCESS)
    return ObjectLinkStateCode::STATE_LINKING_FAILED;
  else
    return link_state;
}

ObjectLinkStateCode
LinkParsers::set_parse_states_link_code(const ParseState* parse_state, 
                                        const ParseState* rp_parse_state,
                                        const std::string& p4_name,
                                        ObjectLinkStateCode state) {
  const std::string& state_name = parse_state->get_name();
  const std::string& rp_state_name = rp_parse_state->get_name();
  return set_parse_states_link_code(state_name, rp_state_name, p4_name, state);
}

ObjectLinkStateCode
LinkParsers::set_parse_states_link_code(const p4object_name_t& state_name, 
                                        const p4object_name_t& link_state_name,
                                        const std::string& p4_name,
                                        ObjectLinkStateCode state) {
  auto ret_code = parse_state_link_table.insert_p4objects_link(
                    p4_name, state_name, link_state_name, state);
  
  switch (ret_code) {
    case TableRetCode::P4_OBJECT_EXIST:
    case TableRetCode::LINK_OBJECT_P4_NAME_EXIST:
      return ObjectLinkStateCode::STATE_LINKING_FAILED;
    case TableRetCode::SUCCESS:
      return state;
    default:
      return ObjectLinkStateCode::STATE_LINKING_FAILED;
  }
}

void 
ParseStateExt::add_header_id_state_mapping(header_id_t ps_header_id, 
                                           header_id_t rp_header_id) {
  header_id_map[ps_header_id] = rp_header_id;
}

const std::unordered_map<header_id_t, header_id_t>& 
ParseStateExt::get_header_ids_map() const {
  return header_id_map;
}



}  // namespace bm
