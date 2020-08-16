/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */



/* -*- P4_16 -*- */
#include <core.p4>
#include <v1model.p4>

const bit<16> ETHERTYPE_IPV4 = 0x800;
const bit<16> ETHERTYPE_VLAN1 = 0x8100;
const bit<16> ETHERTYPE_VLAN2 = 0x9100;

const bit<8> PROTOCOL_TCP = 0x06;
const bit<8> PROTOCOL_UDP = 0x11;
const bit<8> PROTOCOL_GRE = 0x2f;

const bit<16> PROTOCOL_NVGRE = 0x6558;
const bit<16> UDP_DEST_VXLAN = 0x12B5;


/*************************************************************************
*********************** H E A D E R S  ***********************************
*************************************************************************/

typedef bit<9>  egressSpec_t;
typedef bit<48> macAddr_t;
typedef bit<32> ip4Addr_t;

header ethernet_t {
    macAddr_t dstAddr;
    macAddr_t srcAddr;
    bit<16>   etherType;
}

header vlan_t {
    bit<3>  pcp;
    bit<1>  cfi;
    bit<12> vid;
    bit<16> etherType;
}

header ipv4_t {
    bit<4>    version;
    bit<4>    ihl;
    bit<8>    diffserv;
    bit<16>   totalLen;
    bit<16>   identification;
    bit<3>    flags;
    bit<13>   fragOffset;
    bit<8>    ttl;
    bit<8>    protocol;
    bit<16>   hdrChecksum;
    ip4Addr_t srcAddr;
    ip4Addr_t dstAddr;
}

header tcp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<32> seqNum;
    bit<32> actNum;
    bit<4>  dataOffset;
    bit<3>  reserved;
    bit<9>  flags;
    bit<16> windowSize;
    bit<16> checksum; 
    bit<16> urgentPointer;
}

header udp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<16> payloadLength;
    bit<16> checksum; 
}

header gre_t {
    bit<1>  C;
    bit<1>  R;
    bit<1>  K;
    bit<1>  S;
    bit<1>  sr;
    bit<3>  recur;
    bit<5>  flags;
    bit<3>  version;
    bit<16> protocol;
}

header nvgre_t {
    bit<24> vsid;
    bit<8>  flowid;
}

header vxlan_t {
    bit<1>  r0;
    bit<1>  r1;
    bit<1>  r2;
    bit<1>  r3;
    bit<1>  i;
    bit<1>  r51;
    bit<1>  r61;
    bit<1>  r71;
    bit<24> reserved1;
    bit<24> vni;
    bit<8>  reserved2;
}

struct metadata {
    /* empty */
}

struct headers {
    ethernet_t  ethernet;
    ipv4_t      ipv4;
    vlan_t      vlan1;
    vlan_t      vlan2;
    tcp_t       tcp;
    udp_t       udp;
    gre_t       gre;
    nvgre_t     nvgre;
    vxlan_t     vxlan;
}


/*************************************************************************
*********************** P A R S E R  ***********************************
*************************************************************************/

parser ParserImpl(packet_in packet,
                  out headers hdr,
                  inout metadata meta,
                  inout standard_metadata_t standard_metadata) {

    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            ETHERTYPE_IPV4: parse_ipv4;
            ETHERTYPE_VLAN1: parse_vlan1;
            default: accept;
        }
    }

    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            PROTOCOL_TCP : parse_tcp;
            PROTOCOL_UDP : parse_udp;
            PROTOCOL_GRE : parse_gre;
            default: accept;
        }
    }

    state parse_vlan1 {
        packet.extract(hdr.vlan1);
        transition select(hdr.vlan1.etherType) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_VLAN1 : parse_vlan2;
            default: accept;
        }
    }
    
    state parse_vlan2 {
        packet.extract(hdr.vlan2);
        transition select(hdr.vlan2.etherType) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default: accept;
        }
    }

    state parse_tcp {
        packet.extract(hdr.tcp);
        transition accept;
    }


    state parse_udp {
        packet.extract(hdr.udp);
        transition select(hdr.udp.dstPort) {
            UDP_DEST_VXLAN : parse_vxlan;
            default: accept;
        }
    }

    state parse_gre {
        packet.extract(hdr.gre);
        transition select(hdr.gre.protocol) {
            PROTOCOL_NVGRE : parse_nvgre;
            default: accept;
        }
    }

    state parse_nvgre {
        packet.extract(hdr.nvgre);
        transition accept;
    }

    state parse_vxlan {
        packet.extract(hdr.vxlan);
        transition accept;
    }
}


/*************************************************************************
************   C H E C K S U M    V E R I F I C A T I O N   *************
*************************************************************************/

control verifyChecksum(in headers hdr, inout metadata meta) {   
    apply {  }
}


/*************************************************************************
**************  I N G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control ingress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    action drop() {
        mark_to_drop();
    }
    
    action ipv4_forward(macAddr_t dstAddr, egressSpec_t port) {
        standard_metadata.egress_spec = port;
        hdr.ethernet.srcAddr = hdr.ethernet.dstAddr;
        hdr.ethernet.dstAddr = dstAddr;
        hdr.ipv4.ttl = hdr.ipv4.ttl - 1;
    }
    
    table ipv4_lpm {
        key = {
            hdr.ipv4.dstAddr: lpm;
        }
        actions = {
            ipv4_forward;
            drop;
            NoAction;
        }
        size = 1024;
        default_action = NoAction();
    }
    
    apply {
        if (hdr.ipv4.isValid()) {
            ipv4_lpm.apply();
        }
    }
}

/*************************************************************************
****************  E G R E S S   P R O C E S S I N G   *******************
*************************************************************************/

control egress(inout headers hdr, inout metadata meta, inout standard_metadata_t standard_metadata) {
    apply {  }
}

/*************************************************************************
*************   C H E C K S U M    C O M P U T A T I O N   **************
*************************************************************************/

control computeChecksum(
    inout headers  hdr,
    inout metadata meta)
{
    Checksum16() ipv4_checksum;
    
    apply {
        if (hdr.ipv4.isValid()) {
            hdr.ipv4.hdrChecksum = ipv4_checksum.get(
                {    
                    hdr.ipv4.version,
                    hdr.ipv4.ihl,
                    hdr.ipv4.diffserv,
                    hdr.ipv4.totalLen,
                    hdr.ipv4.identification,
                    hdr.ipv4.flags,
                    hdr.ipv4.fragOffset,
                    hdr.ipv4.ttl,
                    hdr.ipv4.protocol,
                    hdr.ipv4.srcAddr,
                    hdr.ipv4.dstAddr
                });
        }
    }
}


/*************************************************************************
***********************  D E P A R S E R  *******************************
*************************************************************************/

control DeparserImpl(packet_out packet, in headers hdr) {
    apply {
        packet.emit(hdr.ethernet);
        packet.emit(hdr.ipv4);
    }
}

/*************************************************************************
***********************  S W I T C H  *******************************
*************************************************************************/

V1Switch(
ParserImpl(),
verifyChecksum(),
ingress(),
egress(),
computeChecksum(),
DeparserImpl()
) main;

