
/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file linker_switch.h
//! This file contains 2 classes: bm::LinkerSwitch and //bm::PacketProgramControlBlock
//! @endcode

#ifndef SIMPLE_LINKER_LINKER_SWITCH_H_
#define SIMPLE_LINKER_LINKER_SWITCH_H_

#include <boost/thread/shared_mutex.hpp>

#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <set>
#include <vector>
#include <iosfwd>
#include <condition_variable>
#include <unordered_map>

#include <bm/bm_sim/P4Objects.h>
#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/logger.h>
#include <bm/bm_linker/linker.h>

namespace ls {

//! The base class for basic functionality to merge two p4 json config
class PacketProramControlBlock {
 public:
  // The constructor takes program name and initiated P4Object 
  explicit PacketProramControlBlock(const std::string &program_name, 
                                    std::shared_ptr<bm::P4Objects> p4object):
                                    program_id(program_name),
                                    p4object_p(p4object) {
    bm::Logger::get()->trace("*********************************");
    bm::Logger::get()->trace(__func__);
    bm::Logger::get()->trace("*********************************");
  }

 private:
  std::string program_id;
  std::shared_ptr<bm::P4Objects> p4object_p;
};


//! This is a Linker Switch class, providing functionality to  merge multiple
//! json configuration files of differnt programs and restructuring the pipeline.
//! This is the base class for the standard Linker switch target
//! implementation.
class LinkerSwitch : public bm::Switch {
 public:
  //! See SwitchWContexts::SwitchWContexts()
  explicit LinkerSwitch();

  // ---------- RuntimeInterface 

  virtual RuntimeInterface::ErrorCode
  add_config(const std::string &program_name, 
             const std::string &config);

  virtual RuntimeInterface::ErrorCode
  delete_config(const std::string &program_name);


  bm::MatchErrorCode
  mt_get_num_entries(size_t cxt_id,
                     const std::string &table_name,
                     size_t *num_entries) const override;
  /*

  MatchErrorCode
  mt_clear_entries(size_t cxt_id,
                   const std::string &table_name,
                   bool reset_default_entry) override {
    return contexts.at(cxt_id).mt_clear_entries(table_name,
                                                reset_default_entry);
  }

  MatchErrorCode
  mt_add_entry(size_t cxt_id,
               const std::string &table_name,
               const std::vector<MatchKeyParam> &match_key,
               const std::string &action_name,
               ActionData action_data,
               entry_handle_t *handle,
               int priority = -1  ) override {
    return contexts.at(cxt_id).mt_add_entry(
        table_name, match_key, action_name,
        std::move(action_data), handle, priority);
  }

  MatchErrorCode
  mt_set_default_action(size_t cxt_id,
                        const std::string &table_name,
                        const std::string &action_name,
                        ActionData action_data) override {
    return contexts.at(cxt_id).mt_set_default_action(
        table_name, action_name, std::move(action_data));
  }

  MatchErrorCode
  mt_delete_entry(size_t cxt_id,
                  const std::string &table_name,
                  entry_handle_t handle) override {
    return contexts.at(cxt_id).mt_delete_entry(table_name, handle);
  }

  MatchErrorCode
  mt_modify_entry(size_t cxt_id,
                  const std::string &table_name,
                  entry_handle_t handle,
                  const std::string &action_name,
                  ActionData action_data) override {
    return contexts.at(cxt_id).mt_modify_entry(
        table_name, handle, action_name, std::move(action_data));
  }

  MatchErrorCode
  mt_set_entry_ttl(size_t cxt_id,
                   const std::string &table_name,
                   entry_handle_t handle,
                   unsigned int ttl_ms) override {
    return contexts.at(cxt_id).mt_set_entry_ttl(table_name, handle, ttl_ms);
  }

  // action profiles

  MatchErrorCode
  mt_act_prof_add_member(size_t cxt_id,
                         const std::string &act_prof_name,
                         const std::string &action_name,
                         ActionData action_data,
                         mbr_hdl_t *mbr) override {
    return contexts.at(cxt_id).mt_act_prof_add_member(
        act_prof_name, action_name, std::move(action_data), mbr);
  }

  MatchErrorCode
  mt_act_prof_delete_member(size_t cxt_id,
                            const std::string &act_prof_name,
                            mbr_hdl_t mbr) override {
    return contexts.at(cxt_id).mt_act_prof_delete_member(act_prof_name, mbr);
  }

  MatchErrorCode
  mt_act_prof_modify_member(size_t cxt_id,
                            const std::string &act_prof_name,
                            mbr_hdl_t mbr,
                            const std::string &action_name,
                            ActionData action_data) override {
    return contexts.at(cxt_id).mt_act_prof_modify_member(
        act_prof_name, mbr, action_name, std::move(action_data));
  }

  MatchErrorCode
  mt_act_prof_create_group(size_t cxt_id,
                           const std::string &act_prof_name,
                           grp_hdl_t *grp) override {
    return contexts.at(cxt_id).mt_act_prof_create_group(act_prof_name, grp);
  }

  MatchErrorCode
  mt_act_prof_delete_group(size_t cxt_id,
                           const std::string &act_prof_name,
                           grp_hdl_t grp) override {
    return contexts.at(cxt_id).mt_act_prof_delete_group(act_prof_name, grp);
  }

  MatchErrorCode
  mt_act_prof_add_member_to_group(size_t cxt_id,
                                  const std::string &act_prof_name,
                                  mbr_hdl_t mbr, grp_hdl_t grp) override {
    return contexts.at(cxt_id).mt_act_prof_add_member_to_group(
        act_prof_name, mbr, grp);
  }

  MatchErrorCode
  mt_act_prof_remove_member_from_group(size_t cxt_id,
                                       const std::string &act_prof_name,
                                       mbr_hdl_t mbr, grp_hdl_t grp) override {
    return contexts.at(cxt_id).mt_act_prof_remove_member_from_group(
        act_prof_name, mbr, grp);
  }

  std::vector<ActionProfile::Member>
  mt_act_prof_get_members(size_t cxt_id,
                          const std::string &act_prof_name) const override {
    return contexts.at(cxt_id).mt_act_prof_get_members(act_prof_name);
  }

  MatchErrorCode
  mt_act_prof_get_member(size_t cxt_id, const std::string &act_prof_name,
                         mbr_hdl_t mbr,
                         ActionProfile::Member *member) const override {
    return contexts.at(cxt_id).mt_act_prof_get_member(
        act_prof_name, mbr, member);
  }

  std::vector<ActionProfile::Group>
  mt_act_prof_get_groups(size_t cxt_id,
                         const std::string &act_prof_name) const override {
    return contexts.at(cxt_id).mt_act_prof_get_groups(act_prof_name);
  }

  MatchErrorCode
  mt_act_prof_get_group(size_t cxt_id, const std::string &act_prof_name,
                        grp_hdl_t grp,
                        ActionProfile::Group *group) const override {
    return contexts.at(cxt_id).mt_act_prof_get_group(act_prof_name, grp, group);
  }

  // indirect tables

  MatchErrorCode
  mt_indirect_add_entry(size_t cxt_id,
                        const std::string &table_name,
                        const std::vector<MatchKeyParam> &match_key,
                        mbr_hdl_t mbr,
                        entry_handle_t *handle,
                        int priority = 1) override {
    return contexts.at(cxt_id).mt_indirect_add_entry(
        table_name, match_key, mbr, handle, priority);
  }

  MatchErrorCode
  mt_indirect_modify_entry(size_t cxt_id,
                           const std::string &table_name,
                           entry_handle_t handle,
                           mbr_hdl_t mbr) override {
    return contexts.at(cxt_id).mt_indirect_modify_entry(
        table_name, handle, mbr);
  }

  MatchErrorCode
  mt_indirect_delete_entry(size_t cxt_id,
                           const std::string &table_name,
                           entry_handle_t handle) override {
    return contexts.at(cxt_id).mt_indirect_delete_entry(table_name, handle);
  }

  MatchErrorCode
  mt_indirect_set_entry_ttl(size_t cxt_id,
                            const std::string &table_name,
                            entry_handle_t handle,
                            unsigned int ttl_ms) override {
    return contexts.at(cxt_id).mt_indirect_set_entry_ttl(
        table_name, handle, ttl_ms);
  }

  MatchErrorCode
  mt_indirect_set_default_member(size_t cxt_id,
                                 const std::string &table_name,
                                 mbr_hdl_t mbr) override {
    return contexts.at(cxt_id).mt_indirect_set_default_member(table_name, mbr);
  }

  MatchErrorCode
  mt_indirect_ws_add_entry(size_t cxt_id,
                           const std::string &table_name,
                           const std::vector<MatchKeyParam> &match_key,
                           grp_hdl_t grp,
                           entry_handle_t *handle,
                           int priority = 1) override {
    return contexts.at(cxt_id).mt_indirect_ws_add_entry(
        table_name, match_key, grp, handle, priority);
  }

  MatchErrorCode
  mt_indirect_ws_modify_entry(size_t cxt_id,
                              const std::string &table_name,
                              entry_handle_t handle,
                              grp_hdl_t grp) override {
    return contexts.at(cxt_id).mt_indirect_ws_modify_entry(
        table_name, handle, grp);
  }

  MatchErrorCode
  mt_indirect_ws_set_default_group(size_t cxt_id,
                                   const std::string &table_name,
                                   grp_hdl_t grp) override {
    return contexts.at(cxt_id).mt_indirect_ws_set_default_group(
        table_name, grp);
  }

  MatchTableType
  mt_get_type(size_t cxt_id, const std::string &table_name) const override {
    return contexts.at(cxt_id).mt_get_type(table_name);
  }

  std::vector<MatchTable::Entry>
  mt_get_entries(size_t cxt_id, const std::string &table_name) const override {
    return contexts.at(cxt_id).mt_get_entries<MatchTable>(table_name);
  }

  std::vector<MatchTableIndirect::Entry>
  mt_indirect_get_entries(size_t cxt_id,
                          const std::string &table_name) const override {
    return contexts.at(cxt_id).mt_get_entries<MatchTableIndirect>(table_name);
  }

  std::vector<MatchTableIndirectWS::Entry>
  mt_indirect_ws_get_entries(size_t cxt_id,
                             const std::string &table_name) const override {
    return contexts.at(cxt_id).mt_get_entries<MatchTableIndirectWS>(table_name);
  }

  MatchErrorCode
  mt_get_entry(size_t cxt_id, const std::string &table_name,
               entry_handle_t handle, MatchTable::Entry *entry) const override {
    return contexts.at(cxt_id).mt_get_entry<MatchTable>(
        table_name, handle, entry);
  }

  MatchErrorCode
  mt_indirect_get_entry(size_t cxt_id, const std::string &table_name,
                        entry_handle_t handle,
                        MatchTableIndirect::Entry *entry) const override {
    return contexts.at(cxt_id).mt_get_entry<MatchTableIndirect>(
        table_name, handle, entry);
  }

  MatchErrorCode
  mt_indirect_ws_get_entry(size_t cxt_id, const std::string &table_name,
                           entry_handle_t handle,
                           MatchTableIndirectWS::Entry *entry) const override {
    return contexts.at(cxt_id).mt_get_entry<MatchTableIndirectWS>(
        table_name, handle, entry);
  }

  MatchErrorCode
  mt_get_default_entry(size_t cxt_id, const std::string &table_name,
                       MatchTable::Entry *entry) const override {
    return contexts.at(cxt_id).mt_get_default_entry<MatchTable>(
        table_name, entry);
  }

  MatchErrorCode
  mt_indirect_get_default_entry(
      size_t cxt_id, const std::string &table_name,
      MatchTableIndirect::Entry *entry) const override {
    return contexts.at(cxt_id).mt_get_default_entry<MatchTableIndirect>(
        table_name, entry);
  }

  MatchErrorCode
  mt_indirect_ws_get_default_entry(
      size_t cxt_id, const std::string &table_name,
      MatchTableIndirectWS::Entry *entry) const override {
    return contexts.at(cxt_id).mt_get_default_entry<MatchTableIndirectWS>(
        table_name, entry);
  }

  MatchErrorCode
  mt_get_entry_from_key(size_t cxt_id, const std::string &table_name,
                        const std::vector<MatchKeyParam> &match_key,
                        MatchTable::Entry *entry,
                        int priority = 1) const override {
    return contexts.at(cxt_id).mt_get_entry_from_key<MatchTable>(
        table_name, match_key, entry, priority);
  }

  MatchErrorCode
  mt_indirect_get_entry_from_key(size_t cxt_id, const std::string &table_name,
                                 const std::vector<MatchKeyParam> &match_key,
                                 MatchTableIndirect::Entry *entry,
                                 int priority = 1) const override {
    return contexts.at(cxt_id).mt_get_entry_from_key<MatchTableIndirect>(
        table_name, match_key, entry, priority);
  }

  MatchErrorCode
  mt_indirect_ws_get_entry_from_key(size_t cxt_id,
                                    const std::string &table_name,
                                    const std::vector<MatchKeyParam> &match_key,
                                    MatchTableIndirectWS::Entry *entry,
                                    int priority = 1) const override {
    return contexts.at(cxt_id).mt_get_entry_from_key<MatchTableIndirectWS>(
        table_name, match_key, entry, priority);
  }

  MatchErrorCode
  mt_read_counters(size_t cxt_id,
                   const std::string &table_name,
                   entry_handle_t handle,
                   MatchTableAbstract::counter_value_t *bytes,
                   MatchTableAbstract::counter_value_t *packets) override {
    return contexts.at(cxt_id).mt_read_counters(
        table_name, handle, bytes, packets);
  }

  MatchErrorCode
  mt_reset_counters(size_t cxt_id,
                    const std::string &table_name) override {
    return contexts.at(cxt_id).mt_reset_counters(table_name);
  }

  MatchErrorCode
  mt_write_counters(size_t cxt_id,
                    const std::string &table_name,
                    entry_handle_t handle,
                    MatchTableAbstract::counter_value_t bytes,
                    MatchTableAbstract::counter_value_t packets) override {
    return contexts.at(cxt_id).mt_write_counters(
        table_name, handle, bytes, packets);
  }

  MatchErrorCode
  mt_set_meter_rates(
      size_t cxt_id, const std::string &table_name, entry_handle_t handle,
      const std::vector<Meter::rate_config_t> &configs) override {
    return contexts.at(cxt_id).mt_set_meter_rates(table_name, handle, configs);
  }

  MatchErrorCode
  mt_get_meter_rates(
      size_t cxt_id, const std::string &table_name, entry_handle_t handle,
      std::vector<Meter::rate_config_t> *configs) override {
    return contexts.at(cxt_id).mt_get_meter_rates(table_name, handle, configs);
  }

  Counter::CounterErrorCode
  read_counters(size_t cxt_id,
                const std::string &counter_name,
                size_t index,
                MatchTableAbstract::counter_value_t *bytes,
                MatchTableAbstract::counter_value_t *packets) override {
    return contexts.at(cxt_id).read_counters(
        counter_name, index, bytes, packets);
  }

  Counter::CounterErrorCode
  reset_counters(size_t cxt_id,
                 const std::string &counter_name) override {
    return contexts.at(cxt_id).reset_counters(counter_name);
  }

  Counter::CounterErrorCode
  write_counters(size_t cxt_id,
                 const std::string &counter_name,
                 size_t index,
                 MatchTableAbstract::counter_value_t bytes,
                 MatchTableAbstract::counter_value_t packets) override {
    return contexts.at(cxt_id).write_counters(
        counter_name, index, bytes, packets);
  }

  MeterErrorCode
  meter_array_set_rates(
      size_t cxt_id, const std::string &meter_name,
      const std::vector<Meter::rate_config_t> &configs) override {
    return contexts.at(cxt_id).meter_array_set_rates(meter_name, configs);
  }

  MeterErrorCode
  meter_set_rates(size_t cxt_id,
                  const std::string &meter_name, size_t idx,
                  const std::vector<Meter::rate_config_t> &configs) override {
    return contexts.at(cxt_id).meter_set_rates(meter_name, idx, configs);
  }

  MeterErrorCode
  meter_get_rates(size_t cxt_id,
                  const std::string &meter_name, size_t idx,
                  std::vector<Meter::rate_config_t> *configs) override {
    return contexts.at(cxt_id).meter_get_rates(meter_name, idx, configs);
  }

  RegisterErrorCode
  register_read(size_t cxt_id,
                const std::string &register_name,
                const size_t idx, Data *value) override {
    return contexts.at(cxt_id).register_read(register_name, idx, value);
  }

  RegisterErrorCode
  register_write(size_t cxt_id,
                 const std::string &register_name,
                 const size_t idx, Data value) override {
    return contexts.at(cxt_id).register_write(
        register_name, idx, std::move(value));
  }

  RegisterErrorCode
  register_write_range(size_t cxt_id,
                       const std::string &register_name,
                       const size_t start, const size_t end,
                       Data value) override {
    return contexts.at(cxt_id).register_write_range(
        register_name, start, end, std::move(value));
  }

  RegisterErrorCode
  register_reset(size_t cxt_id, const std::string &register_name) override {
    return contexts.at(cxt_id).register_reset(register_name);
  }

  ParseVSet::ErrorCode
  parse_vset_add(size_t cxt_id, const std::string &parse_vset_name,
                 const ByteContainer &value) override {
    return contexts.at(cxt_id).parse_vset_add(parse_vset_name, value);
  }

  ParseVSet::ErrorCode
  parse_vset_remove(size_t cxt_id, const std::string &parse_vset_name,
                    const ByteContainer &value) override {
    return contexts.at(cxt_id).parse_vset_remove(parse_vset_name, value);
  }

*/
  // For debugging purpose from command line for these 4 runtime calls
  // Switch class handled it, so no intermediating
  /*
  RuntimeInterface::ErrorCode
  serialize(std::ostream *out) override;

  RuntimeInterface::ErrorCode
  reset_state() override;

  std::string get_config() const override;
  std::string get_config_md5() const override;
  */

  // These runtime services  are not available with linker

  RuntimeInterface::ErrorCode
  load_new_config(const std::string &new_config) override;

  RuntimeInterface::ErrorCode
  remove_config(const std::string &config);

  RuntimeInterface::ErrorCode
  swap_configs() override;


/*
  P4Objects::IdLookupErrorCode p4objects_id_from_name(
      size_t cxt_id, P4Objects::ResourceType type, const std::string &name,
      p4object_id_t *id) const;

  // conscious choice not to use templates here (or could not use virtual)
  CustomCrcErrorCode
  set_crc16_custom_parameters(
      size_t cxt_id, const std::string &calc_name,
      const CustomCrcMgr<uint16_t>::crc_config_t &crc16_config) override;

  CustomCrcErrorCode
  set_crc32_custom_parameters(
      size_t cxt_id, const std::string &calc_name,
      const CustomCrcMgr<uint32_t>::crc_config_t &crc32_config) override;
  */

  // ---------- End RuntimeInterface ----------



  //! TODO: Where is this function used?
  //! Return a raw, non-owning pointer to the FieldList with id \p
  //! field_list_id. This pointer will be invalidated if a configuration swap is
  //! performed by the target. See switch.h documentation for details.
  /*FieldList *get_field_list(const p4object_id_t field_list_id) {
    return get_context(0)->get_field_list(field_list_id);
  }

  //! TODO: Where is this function used?
  // Added for testing, other "object types" can be added if needed
  p4object_id_t get_table_id(const std::string &name) {
    return get_context(0)->get_table_id(name);
  }

  //! TODO: Where is this function used?
  p4object_id_t get_action_id(const std::string &table_name,
                              const std::string &action_name) {
    return get_context(0)->get_action_id(table_name, action_name);
  }

  // to avoid C++ name hiding
  using SwitchWContexts::get_learn_engine;
  //! Obtain a pointer to the LearnEngine for this Switch instance
  LearnEngineIface *get_learn_engine() {
    return get_learn_engine(0);
  }

  // to avoid C++ name hiding
  using SwitchWContexts::get_ageing_monitor;
  AgeingMonitorIface *get_ageing_monitor() {
    return get_ageing_monitor(0);
  }

  // to avoid C++ name hiding
  using SwitchWContexts::get_config_options;
  ConfigOptionMap get_config_options() const {
    return get_config_options(0);
  }
  */

 private:
  int init_objects_(std::istream *is, int dev_id,
                   std::shared_ptr<bm::TransportIface> transport) override;

  //Holds the already initiated P4 Objects of all the running program,
  //or loaded config
  std::unordered_map<std::string, std::shared_ptr<PacketProramControlBlock> > 
    program_name_p4objects_map{};

  bm::Linker linker;
  std::shared_ptr<bm::P4Objects> linker_p4objects;

};

}  // namespace sl

#endif  // SIMPLE_LINKER_LINKER_SWITCH_H_
