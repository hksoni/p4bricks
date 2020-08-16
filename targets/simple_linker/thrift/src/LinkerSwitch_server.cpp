
/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

#include <bm/LinkerSwitch.h>

#ifdef P4THRIFT
#include <p4thrift/protocol/TBinaryProtocol.h>
#include <p4thrift/server/TThreadedServer.h>
#include <p4thrift/transport/TServerSocket.h>
#include <p4thrift/transport/TBufferTransports.h>

namespace thrift_provider = p4::thrift;
#else
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

namespace thrift_provider = apache::thrift;
#endif

#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/logger.h>

#include "linker_switch.h"

namespace lswitch_runtime {

class LinkerSwitchHandler : virtual public LinkerSwitchIf {
 public:
  explicit LinkerSwitchHandler(ls::LinkerSwitch *sw)
    : switch_(sw) { }

  int32_t p4_program_config_add(const std::string& program_name, 
                                const std::string& config_str) {

    bm::Logger::get()->trace("p4_program_config_add");
    bm::Logger::get()->trace(__func__);
    switch_->add_config(program_name, config_str);
    return 0;
  }

  int32_t p4_program_config_delete(const std::string& program_name) {
    bm::Logger::get()->trace(__func__);
    bm::Logger::get()->trace("p4_program_config_delete");
    switch_->delete_config(program_name);
    return 0;
  }

 private:
  ls::LinkerSwitch *switch_;
};

boost::shared_ptr<LinkerSwitchIf> get_handler(ls::LinkerSwitch *sw) {
  return boost::shared_ptr<LinkerSwitchHandler>(new LinkerSwitchHandler(sw));
}

}  // namespace lswitch_runtime
