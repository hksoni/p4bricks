
/*
 * Hardik Soni (hardik.soni@inria.fr)
 *
 */

//! @file datacenter.p4

header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header_type vlan_t {
    fields {
        pcp : 3;
        cfi : 1;
        vid : 12;
        ethertype : 16;
    }
}

header_type ipv4_t {
    fields {
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr: 32;
    }
}

header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNum : 32;
        actNum : 32;
        dataOffset : 4;
        reserved : 3;
        flags : 9;
        windowSize : 16;
        checksum : 16; 
        urgentPointer : 16;
    }
}

header_type udp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        payloadLength : 16;
        checksum : 16; 
    }
}

header_type gre_t {
    fields {
        C : 1;
        R : 1;
        K : 1;
        S : 1;
        sr : 1;
        recur : 3;
        flags : 5;
        version : 3;
        protocol : 16;
    }
}

header_type nvgre_t {
    fields {
        vsid : 24;
        flowid : 8;
    }
}

header_type vxlan_t {
    fields {
        r0 : 1;
        r1 : 1;
        r2 : 1;
        r3 : 1;
        i : 1;
        r5 : 1;
        r6 : 1;
        r7 : 1;
        reserved1 : 24;
        vni : 24;
        reserved2 : 8;
    }
}

#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_IPV6 0x86DD
#define ETHERTYPE_VLAN1 0x8100


#define PROTOCOL_TCP 0x06
#define PROTOCOL_UDP 0x11
#define PROTOCOL_GRE 0x2f

#define UDP_DEST_VXLAN 0x12B5
#define PROTOCOL_NVGRE 0x6558

header ethernet_t ethernet;
header vlan_t vlan1;
header vlan_t vlan2;
header ipv4_t ipv4;
header tcp_t tcp;
header udp_t udp;
header vxlan_t vxlan;
header gre_t gre;
header nvgre_t nvgre;


parser start {
    return parse_ethernet;
}

parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4;
        ETHERTYPE_VLAN1 : parse_vlan1;
        default: ingress;
    }
}

parser parse_vlan1 {
    extract(vlan1);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4;
        ETHERTYPE_VLAN1 : parse_vlan2;
        default: ingress;
    }
}

parser parse_vlan2 {
    extract(vlan2);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4;
        default: ingress;
    }
}

parser parse_ipv4 {
    extract(ipv4);
    return select(latest.protocol) {
        PROTOCOL_TCP : parse_tcp;
        PROTOCOL_UDP : parse_udp;
        PROTOCOL_GRE : parse_gre;
        default: ingress;
    }
}

parser parse_gre {
    extract(gre);
    return select(latest.protocol) {
        PROTOCOL_NVGRE : parse_nvgre;
        default: ingress;
    }
}

parser parse_udp {
    extract(udp);
    return select(latest.dstPort) {
        UDP_DEST_VXLAN : parse_vxlan;
        default: ingress;
    }
}

parser parse_vxlan {
    extract(vxlan);
    return ingress;
}

parser parse_nvgre {
    extract(nvgre);
    return ingress;
}

field_list ipv4_checksum_list {
        ipv4.version;
        ipv4.ihl;
        ipv4.diffserv;
        ipv4.totalLen;
        ipv4.identification;
        ipv4.flags;
        ipv4.fragOffset;
        ipv4.ttl;
        ipv4.protocol;
        ipv4.srcAddr;
        ipv4.dstAddr;
}

field_list_calculation ipv4_checksum {
    input {
        ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4.hdrChecksum  {
    verify ipv4_checksum;
    update ipv4_checksum;
}

action _drop() {
    drop();
}

header_type routing_metadata_t {
    fields {
        nhop_ipv4 : 32;
    }
}

metadata routing_metadata_t routing_metadata;

action set_nhop(nhop_ipv4, port) {
    modify_field(routing_metadata.nhop_ipv4, nhop_ipv4);
    modify_field(standard_metadata.egress_spec, port);
    modify_field(ipv4.ttl, ipv4.ttl - 1);
}

table ipv4_lpm {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        set_nhop;
        _drop;
    }
    size: 1024;
}

action set_dmac(dmac) {
    modify_field(ethernet.dstAddr, dmac);
}

table forward {
    reads {
        routing_metadata.nhop_ipv4 : exact;
    }
    actions {
        set_dmac;
        _drop;
    }
    size: 512;
}

action rewrite_mac(smac) {
    modify_field(ethernet.srcAddr, smac);
}

table send_frame {
    reads {
        standard_metadata.egress_port: exact;
    }
    actions {
        rewrite_mac;
        _drop;
    }
    size: 256;
}

control ingress {
    if(valid(ipv4) and ipv4.ttl > 0) {
        apply(ipv4_lpm);
        apply(forward);
    }
}

control egress {
    apply(send_frame);
}


