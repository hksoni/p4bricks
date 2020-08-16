
/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

#include <bm/LinkerSwitch.h>
#include <bm/bm_sim/queue.h>
#include <bm/bm_sim/packet.h>
#include <bm/bm_sim/parser.h>
#include <bm/bm_sim/tables.h>
#include <bm/bm_sim/switch.h>
#include <bm/bm_sim/event_logger.h>
#include <bm/bm_sim/logger.h>

#include <bm/bm_runtime/bm_runtime.h>

#include <unistd.h>

#include <iostream>
#include <memory>
#include <thread>
#include <fstream>
#include <string>
#include <chrono>

#include "linker_switch.h"

using bm::Switch;
using bm::Queue;
using bm::Packet;
using bm::PHV;
using bm::Parser;
using bm::Deparser;
using bm::Pipeline;


class SimpleLinker : public ls::LinkerSwitch {
 public:
  SimpleLinker()
    : ls::LinkerSwitch(),
      input_buffer(1024), output_buffer(128) {
    add_required_field("standard_metadata", "egress_spec");
    add_required_field("standard_metadata", "egress_port");
  }

  int receive_(int port_num, const char *buffer, int len) override {
    static int pkt_id = 0;

    auto packet = new_packet_ptr(port_num, pkt_id++, len,
                                 bm::PacketBuffer(2048, buffer, len));

    BMELOG(packet_in, *packet);

    input_buffer.push_front(std::move(packet));
    return 0;
  }

  void start_and_return_() override {
    std::thread t1(&SimpleLinker::pipeline_thread, this);
    t1.detach();
    std::thread t2(&SimpleLinker::transmit_thread, this);
    t2.detach();
  }

 private:
  void pipeline_thread();
  void transmit_thread();

 private:
  Queue<std::unique_ptr<Packet> > input_buffer;
  Queue<std::unique_ptr<Packet> > output_buffer;
};

void SimpleLinker::transmit_thread() {
  while (1) {
    std::unique_ptr<Packet> packet;
    output_buffer.pop_back(&packet);
    BMELOG(packet_out, *packet);
    BMLOG_DEBUG_PKT(*packet, "Transmitting packet of size {} out of port {}",
                    packet->get_data_size(), packet->get_egress_port());
    transmit_fn(packet->get_egress_port(),
                packet->data(), packet->get_data_size());
  }
}

void SimpleLinker::pipeline_thread() {
  // Linker Switch never invalidates original pointer, but may modify parser,
  // deparser and pipelines by locking them.
  // Hence, locks are applied in parse, apply and deparse function of these
  // objects.
  // TODO: revisit here, if the original pointers are required to change
  Pipeline *ingress_mau = this->get_pipeline("ingress");
  Pipeline *egress_mau = this->get_pipeline("egress");
  Parser *parser = this->get_parser("parser");
  Deparser *deparser = this->get_deparser("deparser");
  PHV *phv;

  while (1) {
    std::unique_ptr<Packet> packet;
    input_buffer.pop_back(&packet);
    phv = packet->get_phv();

    int ingress_port = packet->get_ingress_port();
    (void) ingress_port;
    BMLOG_DEBUG_PKT(*packet, "Processing packet received on port {}",
                    ingress_port);

    //TODO: change pointers here, if original pointers are required to change
    parser->parse(packet.get());
    ingress_mau->apply(packet.get());

    int egress_spec = phv->get_field("standard_metadata.egress_spec").get_int();
    BMLOG_DEBUG_PKT(*packet, "Egress port is {}", egress_spec);

    if (egress_spec == 511) {
      BMLOG_DEBUG_PKT(*packet, "Dropping packet");
    } else {
      packet->set_egress_port(egress_spec);
      phv->get_field("standard_metadata.egress_port").set(egress_spec);
      egress_mau->apply(packet.get());
      deparser->deparse(packet.get());
      output_buffer.push_front(std::move(packet));
    }
  }
}


namespace {
SimpleLinker *simple_linker_switch;
//bm::TargetParserBasic *simple_switch_parser;
}  // namespace

namespace lswitch_runtime {
shared_ptr<lswitch_runtime::LinkerSwitchIf> get_handler(ls::LinkerSwitch *sw);
}  // namespace lswitch_runtime

int
main(int argc, char* argv[]) {
  simple_linker_switch = new SimpleLinker();
  int status = simple_linker_switch->init_from_command_line_options(
      argc, argv);
  if (status != 0) std::exit(status);

  int thrift_port = simple_linker_switch->get_runtime_port();
  bm_runtime::start_server(simple_linker_switch, thrift_port);

  // adding service specific subclass of switch can be done in init of the
  // subclass, may be..
  using ::lswitch_runtime::LinkerSwitchIf;
  using ::lswitch_runtime::LinkerSwitchProcessor;
  bm_runtime::add_service<LinkerSwitchIf, LinkerSwitchProcessor>(
      "linker_switch", lswitch_runtime::get_handler(simple_linker_switch));
  
  simple_linker_switch->start_and_return();

  while (true) std::this_thread::sleep_for(std::chrono::seconds(100));

  return 0;
}
