/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file link_parsers.h

#ifndef BM_BM_LINKER_PARSERS_H_
#define BM_BM_LINKER_PARSERS_H_

#include <vector>
#include <string>
#include <memory>
#include <set>
#include <unordered_map> 
#include <utility>

#include <bm/bm_sim/P4Objects.h>
#include <bm/bm_linker/link_headers.h>
#include <bm/bm_linker/p4objects_linker_ext.h>

namespace bm {

enum UIDTableLinkOPCode {
  MAP_EXISTING_ID = 0,
  INSERT_NEW_ID,
  NO_OPS
};

class ParseStateExt {


 public:
  //! This class is used to carry the information about p4objects being added, 
  // temporay bindings of Ids and other contexts required to create a merged 
  // parse state. 
  // P4ObjectsLinkerExt* p4_ext is the extended P4Object of the one being
  // added.
  explicit ParseStateExt(const std::string &name, p4object_id_t id,
                         UIDTableLinkOPCode id_op, P4ObjectsLinkerExt* p4_ext)
    : uid_table_link_opcode(id_op), p4objects_ext(p4_ext), 
    new_parse_state(new ParseState(name, id)) {}

  UIDTableLinkOPCode get_uid_table_link_code() const {
    return uid_table_link_opcode;
  }

  bool switch_case_merged() const {
    return has_switch_case_merged;
  }

  void switch_case_merged(bool flag) {
    has_switch_case_merged = flag;
  }

  const std::string&  get_p4_name() const {
    return p4objects_ext->get_p4_name();
  }

  ParseState* get_new_parse_state() {
    return new_parse_state.get();
  }

  P4ObjectsLinkerExt* get_p4objects_linker_ext() {
    return p4objects_ext;
  }

  void add_header_id_state_mapping(header_id_t ps_header_id, 
                                   header_id_t rp_header_id);


  const std::unordered_map<header_id_t, header_id_t>& get_header_ids_map() const;

  enum ParserOpTypes {
    EXTRACT,
    EXTRACT_VL,
    EXTRACT_STACK,
    EXTRACT_UNION_STACK,
    EXTRACT_TYPE_MAX,
    VERIFY,
    PRIMITIVE,
    SHIFT,
    SET,
    UNSUPPORTED_TYPE
  };


 private:
  UIDTableLinkOPCode uid_table_link_opcode;

  // Make it shared_ptr, extension of P4Objects being added.
  P4ObjectsLinkerExt* p4objects_ext;
  
  bool has_switch_case_merged{false};

  std::unique_ptr<ParseState> new_parse_state;

  // key - id in ParseState of P4Objects being added 
  // val = id of ParseState of running P4Objects
  std::unordered_map<header_id_t, header_id_t> header_id_map{};
};



class LinkParsers {

  using SwitchCaseKeyStateMap = std::unordered_map<ByteContainer,
                                const ParseState*, ByteContainerKeyHash>;
  using ParserOpTypes = ParseStateExt::ParserOpTypes;
 public:

  explicit LinkParsers(
           std::shared_ptr<const HeaderTypeUIDTable> header_type_uid_table,
           std::shared_ptr<HeaderUIDTable> header_uid_table) 
    : header_type_uid_table_(header_type_uid_table),
      header_uid_table_(header_uid_table) {
  }


  ObjectLinkStateCode link_parser(std::shared_ptr<P4ObjectsLinkerExt> p4objs_ext,
                                  std::shared_ptr<P4Objects> p4objects);
 
  void reset_p4objects_linker_ext(std::shared_ptr<P4ObjectsLinkerExt> ptr) {
    p4objects_linker_ext_ = ptr;
  }

  // Disabling copying, but allow moving
  LinkParsers(const LinkParsers &other) = delete;
  LinkParsers &operator=(const LinkParsers &) = delete;

 private:


  ObjectLinkStateCode map_ordered_parser_states(
                      const std::vector<const ParseState* >& parse_states,
                      const std::vector<const ParseState* >& rp_parse_states,
                      P4ObjectsLinkerExt* p4objects_linker_ext);

  ObjectLinkStateCode can_link_parser_states(const ParseState *parse_state,
                                             const ParseState *rp_parse_state,
                                             ParseStateExt* parse_state_ext);

  ObjectLinkStateCode add_parser_state(const ParseState *ps,
                                       ParseStateExt* parse_state_ext);

  ObjectLinkStateCode merge_parser_states_switch_cases(
                      const ParseState *parse_state,
                      const ParseState *rp_parse_state,
                      ParseStateExt* parse_state_ext);

  void get_switch_case_hexstr_key_state_map(
      const std::vector<std::unique_ptr<ParseSwitchCaseIface> >& parser_switch,
      SwitchCaseKeyStateMap& key_state_map);

  ObjectLinkStateCode merge_parser_ops(
                      const std::vector<std::unique_ptr<ParserOp> >& ops,
                      const std::vector<std::unique_ptr<ParserOp> >& rp_ops,
                      ParseStateExt* parse_state_ext);

  ObjectLinkStateCode add_parser_ops(
                      const std::vector<std::unique_ptr<ParserOp> >& parse_ops,
                      ParseStateExt* parse_state_ext);
  
  ObjectLinkStateCode link_parse_switch_key(
                      const ParseSwitchKeyBuilder& key_builder,
                      const ParseSwitchKeyBuilder& rp_key_builder,
                      ParseSwitchKeyBuilder& merged_key_builder,
                      const ParseStateExt* parse_state_ext);


  ObjectLinkStateCode  link_extract_parser_op(const ParserOp* op, 
                                              const ParserOp* rp_op,
                                              ParserOpTypes extract_type_op,
                                              ParseStateExt* parse_state_ext);

  ObjectLinkStateCode link_extract_parser_op_regular(const ParserOp* op,
                                                     const ParserOp* rp_op,
                                                     ParseStateExt* state_ext);

  ObjectLinkStateCode add_extract_parser_op_regular(const ParserOp* op, 
                                                    ParseStateExt* state_ext);

  /*
  ObjectLinkStateCode link_extract_parser_op_stack(const ParserOp* op,
                                                   const ParserOp* rp_op,
                                                   ParseStateExt* state_ext);

  ObjectLinkStateCode link_extract_parser_op_VL(const ParserOp* parser_op, 
                                                const ParserOp* rp_parse_op,
                                                ParseStateExt* state_ext);
  */

  ParserOpTypes get_parser_op_type(const ParserOp* parser_op);


  ObjectLinkStateCode add_parse_state_link(const ParseState* parse_state, 
                                           const std::string& p4_name,
                                           ObjectLinkStateCode state);

  ObjectLinkStateCode get_parse_state_link(const ParseState* parse_state, 
                                           const std::string& p4_name,
                                           std::string& out_name);

  ObjectLinkStateCode get_parse_states_link_code(const ParseState* parse_st,
                                                 const ParseState* rp_parse_st,
                                                 const std::string& p4_name);

  ObjectLinkStateCode set_parse_states_link_code(const ParseState* parse_state,
                                                 const ParseState* rp_state,
                                                 const std::string& p4_name,
                                                 ObjectLinkStateCode state);
 
  ObjectLinkStateCode set_parse_states_link_code(const p4object_name_t& name,
                                                 const p4object_name_t& rp_name,
                                                 const std::string& p4_name,
                                                 ObjectLinkStateCode state);


  p4object_id_t get_new_state_id() {
    return ++state_id;
  }
 private:

///////////////////////////////////////////////////////////////////////
// Objects with life time of Linker 

  //This table comes from Linker object, which initializes LinkParsers
  std::shared_ptr<const HeaderTypeUIDTable> header_type_uid_table_;

  //This table comes from Linker object, which initializes LinkParsers
  std::shared_ptr<HeaderUIDTable> header_uid_table_;

  // The reference comes from the Linker.
  std::shared_ptr<P4ObjectsLinkerExt> p4objects_linker_ext_;

  p4object_id_t state_id{0};

  LinkerUIDTableGeneric<p4object_name_t> parse_state_link_table{};
///////////////////////////////////////////////////////////////////////////

  // Lifetime - only during add parser,
  // after that move it to P4ObjectsLinkerExt
  std::unordered_map<p4object_name_t, ParseStateExt*> linked_parse_state{};

};



}  // namespace bm

#endif  // BM_BM_LINKER_PARSERS_H_
