
/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

#include <bm/bm_sim/_assert.h>
#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/options_parse.h>
#include <bm/bm_sim/logger.h>
#include <bm/bm_sim/debugger.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_sim/packet.h>

#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <streambuf>


#include "linker_switch.h"

namespace ls {


// Linker Switch class
// TODO: Sure, nothing to be done in CTOR??
LinkerSwitch::LinkerSwitch()
  : bm::Switch(true) { 
//  : bm::SwitchWContexts(true) { 
  TRACE_START;
}


bm::RuntimeInterface::ErrorCode
LinkerSwitch::add_config(const std::string &program_name, 
                         const std::string &config) {
  TRACE_START;
  TRACE_PRINT(program_name);
  std::shared_ptr<bm::P4Objects> p4objects = 
    std::make_shared<bm::P4Objects>(std::cout, true);

  std::istringstream is(config); 

  int rc = p4objects->init_objects(&is, get_lookup_factory(), device_id, 0u, 
                                   notifications_transport, required_fields, 
                                   arith_objects);

  if (rc != 0) {
    TRACE_PRINT_CONST("new p4objects initiation failed")
    TRACE_END;
    return RuntimeInterface::ErrorCode::SUCCESS;
  }
  TRACE_PRINT_CONST("new p4objects initiated")
  
  // these returns new merged P4 object
  std::shared_ptr<bm::P4Objects> new_p4objects = linker.add_p4objects(
                                                   program_name, p4objects);
  
  TRACE_END;
  return RuntimeInterface::ErrorCode::SUCCESS;
}


bm::RuntimeInterface::ErrorCode
LinkerSwitch::delete_config(const std::string &program_name) {
  TRACE_START;
  bm::Logger::get()->trace(program_name);
  TRACE_END;
  return RuntimeInterface::ErrorCode::SUCCESS;
}


bm::MatchErrorCode
LinkerSwitch::mt_get_num_entries(size_t cxt_id,
                                 const std::string &table_name,
                                 size_t *num_entries) const {
  TRACE_START;
  std::cout<<cxt_id<<"\n";
  std::cout<<table_name<<"\n";
  std::cout<<(*num_entries)<<"\n";
  bm::Logger::get()->trace("*********************************");
  return bm::MatchErrorCode::SUCCESS;
}


int
LinkerSwitch::init_objects_(std::istream *is, int dev_id,
                           std::shared_ptr<bm::TransportIface> transport) {
  TRACE_START;
  int status = 0;

  device_id = dev_id;

  std::shared_ptr<bm::P4Objects> p4objects = std::make_shared<bm::P4Objects>(
                                             std::cout, true);

  if (!transport) {
    notifications_transport = std::shared_ptr<bm::TransportIface>(
        bm::TransportIface::make_dummy());
  } else {
    notifications_transport = std::move(transport);
  }
  auto cxt = get_context();
  cxt->set_device_id(device_id);
  cxt->set_notifications_transport(notifications_transport);
  if (is != nullptr) {
    
    status = p4objects->init_objects(is, get_lookup_factory(), device_id, 
                                     0u, notifications_transport,
                                     required_fields, arith_objects);
    is->clear();
    is->seekg(0, std::ios::beg);
    if (status) 
      return status;
    // CONT
    bm::Logger::get()->trace("Calling P4 Linker switch init..");
    try {
    linker.add_p4objects("p4.1", p4objects);
    } catch (const std::exception& e) {
      TRACE_PRINT_CONST("Exception");
      TRACE_PRINT(e.what());
    }
  }

  cxt->init_objects(p4objects);
  phv_source->set_phv_factory(0u, &(cxt->get_phv_factory()));

  bm::Logger::get()->trace("*********************************");
  return 0;
}

bm::RuntimeInterface::ErrorCode
LinkerSwitch::load_new_config(const std::string &new_config) {
  bm::Logger::get()->trace("*********************************");
  bm::Logger::get()->trace(__func__);
  bm::Logger::get()->error("It is an error to call load_new_config command");
  bm::Logger::get()->trace(new_config);
  bm::Logger::get()->trace("*********************************");
  return RuntimeInterface::ErrorCode::CONFIG_SWAP_DISABLED;
}


bm::RuntimeInterface::ErrorCode
LinkerSwitch::remove_config(const std::string &config) {
  bm::Logger::get()->trace("*********************************");
  bm::Logger::get()->trace(__func__);
  bm::Logger::get()->error("It is an error to call remove_config command");
  bm::Logger::get()->trace(config);
  bm::Logger::get()->trace("*********************************");
  return RuntimeInterface::ErrorCode::CONFIG_SWAP_DISABLED;
}


bm::RuntimeInterface::ErrorCode
LinkerSwitch::swap_configs() {
  bm::Logger::get()->trace("*********************************");
  bm::Logger::get()->trace(__func__);
  bm::Logger::get()->error("It is an error to call swap_config command");
  bm::Logger::get()->trace("*********************************");
  return RuntimeInterface::ErrorCode::CONFIG_SWAP_DISABLED;
}



}  // namespace ls
