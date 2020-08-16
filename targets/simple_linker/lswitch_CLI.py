#!/usr/bin/env python2


#
# Hardik Soni (hardik.soni@inria.fr)
#
#

import runtime_CLI

import sys
import os
import json

from lswitch_runtime import LinkerSwitch

class LinkerSwitchAPI(runtime_CLI.RuntimeAPI):
    @staticmethod
    def get_thrift_services():
        return [("linker_switch", LinkerSwitch.Client)]

    def __init__(self, pre_type, standard_client, mc_client, lswitch_client):
        runtime_CLI.RuntimeAPI.__init__(self, pre_type,
                                        standard_client, mc_client)
        self.lswitch_client = lswitch_client

    @runtime_CLI.handle_bad_input
    def do_p4_program_config_add(self, line):
        "Add P4 program compiled into json file : \
                p4_program_config_add <program name> <path to .json file>"
        args = line.split()
        self.exactly_n_args(args, 2)
        program_name = args[0]
        filename = args[1]
        if not os.path.isfile(filename):
            raise runtime_CLI.UIn_Error("Not a valid filename")
        print "Adding  Json config"
        with open(filename, 'r') as f:
            json_str = f.read()
            try:
                json.loads(json_str)
            except:
                raise runtime_CLI.UIn_Error("Not a valid JSON file")
            ret = self.lswitch_client.p4_program_config_add(program_name, json_str)
            print ret

    @runtime_CLI.handle_bad_input
    def do_p4_program_config_delete(self, line):
        "Remove p4 program config json : p4_program_config_delete <program name>"
        program_name = str(line)
        ret = self.lswitch_client.p4_program_config_delete(program_name)
        print ret

def main():
    args = runtime_CLI.get_parser().parse_args()

    args.pre = runtime_CLI.PreType.SimplePreLAG

    services = runtime_CLI.RuntimeAPI.get_thrift_services(args.pre)
    services.extend(LinkerSwitchAPI.get_thrift_services())

    standard_client, mc_client, lswitch_client = runtime_CLI.thrift_connect(
        args.thrift_ip, args.thrift_port, services
    )

    runtime_CLI.load_json_config(standard_client, args.json)

    LinkerSwitchAPI(args.pre, standard_client, mc_client, lswitch_client).cmdloop()

if __name__ == '__main__':
    main()
