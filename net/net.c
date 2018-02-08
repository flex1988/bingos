#include "net/net.h"
#include "kernel/malloc.h"
#include "kernel/process.h"

static int tasklet_pid = 0;

struct netif {
    void *extra;

    get_mac_func_t get_mac;
    get_packet_func_t get_packet;
    send_packet_func_t send_packet;

    uint8_t hwaddr[6];
    uint32_t source;

    char *driver;
    uint32_t gateway;
};

static struct netif _netif = {0};
static uint32_t _dns_server;

uint32_t ip_aton(const char *in) {
    char ip[16];
    char *c = ip;
    uint32_t out[4];
    char *i;
    memcpy(ip, in, strlen(in) < 15 ? strlen(in) + 1 : 15);
    ip[15] = '\0';

    i = (char *)lfind(c, '.');
    *i = '\0';
    out[0] = atoi(c);
    c += strlen(c) + 1;

    i = (char *)lfind(c, '.');
    *i = '\0';
    out[1] = atoi(c);
    c += strlen(c) + 1;

    i = (char *)lfind(c, '.');
    *i = '\0';
    out[2] = atoi(c);
    c += strlen(c) + 1;

    out[3] = atoi(c);

    return ((out[0] << 24) | (out[1] << 16) | (out[2] << 8) | (out[3]));
}

void ip_ntoa(uint32_t src_addr, char *out) {
    sprintf(out, "%d.%d.%d.%d", (src_addr & 0xFF000000) >> 24,
            (src_addr & 0xFF0000) >> 16, (src_addr & 0xFF00) >> 8,
            (src_addr & 0xFF));
}

uint16_t calculate_ipv4_checksum(struct ipv4_packet *p) {
    uint32_t sum = 0;
    uint16_t *s = (uint16_t *)p;

    /* TODO: Checksums for options? */
    for (int i = 0; i < 10; ++i) {
        sum += ntohs(s[i]);
    }

    if (sum > 0xFFFF) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    return ~(sum & 0xFFFF) & 0xFFFF;
}

static size_t write_dhcp_packet(uint8_t *buffer) {
    size_t offset = 0;
    size_t payload_size = sizeof(struct dhcp_packet);

    uint8_t dhcp_options[] = {
        53, 1, 1, 55, 2, 3, 6, 255,
    };

    payload_size += sizeof(dhcp_options);

    struct ethernet_packet eth_out = {
        .source = {_netif.hwaddr[0], _netif.hwaddr[1], _netif.hwaddr[2],
                   _netif.hwaddr[3], _netif.hwaddr[4], _netif.hwaddr[5]},
        .destination = BROADCAST_MAC,
        .type = htons(0x0800),
    };

    memcpy(&buffer[offset], &eth_out, sizeof(struct ethernet_packet));
    offset += sizeof(struct ethernet_packet);

    uint16_t _length = htons(sizeof(struct ipv4_packet) +
                             sizeof(struct udp_packet) + payload_size);
    uint16_t _ident = htons(1);

    struct ipv4_packet ipv4_out = {
        .version_ihl = ((0x4 << 4) | (0x5 << 0)),
        .dscp_ecn = 0,
        .length = _length,
        .ident = _ident,
        .flags_fragment = 0,
        .ttl = 0x40,
        .protocol = IPV4_PROT_UDP,
        .checksum = 0,
        .source = htonl(ip_aton("0.0.0.0")),
        .destination = htonl(ip_aton("255.255.255.255")),
    };

    uint16_t checksum = calculate_ipv4_checksum(&ipv4_out);
    ipv4_out.checksum = htons(checksum);

    memcpy(&buffer[offset], &ipv4_out, sizeof(struct ipv4_packet));
    offset += sizeof(struct ipv4_packet);

    uint16_t _udp_source = htons(68);
    uint16_t _udp_destination = htons(67);
    uint16_t _udp_length = htons(sizeof(struct udp_packet) + payload_size);

    struct udp_packet udp_out = {
        .source_port = _udp_source,
        .destination_port = _udp_destination,
        .length = _udp_length,
        .checksum = 0,
    };

    memcpy(&buffer[offset], &udp_out, sizeof(struct udp_packet));
    offset += sizeof(struct udp_packet);

    struct dhcp_packet bootp_out = {
        .op = 1,
        .htype = 1,
        .hlen = 6,
        .hops = 0,
        .xid = htonl(0x1337),
        .secs = 0,
        .flags = 0,

        .ciaddr = 0x000000,
        .yiaddr = 0x000000,
        .siaddr = 0x000000,
        .giaddr = 0x000000,
        .chaddr = {_netif.hwaddr[0], _netif.hwaddr[1], _netif.hwaddr[2],
                   _netif.hwaddr[3], _netif.hwaddr[4], _netif.hwaddr[5]},
        .sname = {0},
        .file = {0},
        .magic = htonl(DHCP_MAGIC),
    };

    memcpy(&buffer[offset], &bootp_out, sizeof(struct dhcp_packet));
    offset += sizeof(struct dhcp_packet);

    memcpy(&buffer[offset], &dhcp_options, sizeof(dhcp_options));
    offset += sizeof(dhcp_options);

    return offset;
}

static size_t write_arp_request(uint8_t *buffer, uint32_t ip) {
    size_t offset = 0;
    printk("Request ARP from gateway address %x", ip);

    struct ethernet_packet eth_out = {
        .source = {_netif.hwaddr[0], _netif.hwaddr[1], _netif.hwaddr[2],
                   _netif.hwaddr[3], _netif.hwaddr[4], _netif.hwaddr[5]},
        .destination = BROADCAST_MAC,
        .type = htons(0x0806),
    };

    memcpy(&buffer[offset], &eth_out, sizeof(struct ethernet_packet));
    offset += sizeof(struct ethernet_packet);

    struct arp arp_out;

    arp_out.htype = ntohs(1);

    arp_out.proto = ntohs(0x0800);
    arp_out.hlen = 6;
    arp_out.plen = 4;
    arp_out.oper = ntohs(1);

    arp_out.sender_ha[0] = _netif.hwaddr[0];
    arp_out.sender_ha[1] = _netif.hwaddr[1];
    arp_out.sender_ha[2] = _netif.hwaddr[2];
    arp_out.sender_ha[3] = _netif.hwaddr[3];
    arp_out.sender_ha[4] = _netif.hwaddr[4];
    arp_out.sender_ha[5] = _netif.hwaddr[5];
    arp_out.sender_ip = ntohl(_netif.source);

    arp_out.target_ha[0] = 0;
    arp_out.target_ha[1] = 0;
    arp_out.target_ha[2] = 0;
    arp_out.target_ha[3] = 0;
    arp_out.target_ha[4] = 0;
    arp_out.target_ha[5] = 0;

    arp_out.target_ip = ntohl(ip);

    memcpy(&buffer[offset], &arp_out, sizeof(struct arp));
    offset += sizeof(struct arp);

    return offset;
}

static void placeholder_dhcp(void) {
    printk("Sending DHCP discover");

    void *tmp = kmalloc(1024);

    size_t packet_size = write_dhcp_packet(tmp);
    _netif.send_packet(tmp, packet_size);

    kfree(tmp);

    while (1) {
        struct ethernet_packet *eth =
            (struct ethernet_packet *)_netif.get_packet();
        uint16_t eth_type = ntohs(eth->type);

        printk(
            "Ethernet II, Src: (%2x:%2x:%2x:%2x:%2x:%2x), Dst: "
            "(%2x:%2x:%2x:%2x:%2x:%2x) [type=%4x])",
            eth->source[0], eth->source[1], eth->source[2], eth->source[3],
            eth->source[4], eth->source[5], eth->destination[0],
            eth->destination[1], eth->destination[2], eth->destination[3],
            eth->destination[4], eth->destination[5], eth_type);

        if (eth_type != 0x0800) {
            printk("ARP packet while waiting for DHCP");
            kfree(eth);
            continue;
        }

        struct ipv4_packet *ipv4 = (struct ipv4_packet *)eth->payload;
        uint32_t src_addr = ntohl(ipv4->source);
        uint32_t dst_addr = ntohl(ipv4->destination);
        uint16_t length = ntohs(ipv4->length);

        char src_ip[16];
        char dst_ip[16];

        ip_ntoa(src_addr, src_ip);
        ip_ntoa(dst_addr, dst_ip);

        printk("IP packet [%s → %s] length=%d bytes", src_ip, dst_ip, length);
    }
}

static struct ethernet_packet *net_receive(void) {
    struct ethernet_packet *eth = _netif.get_packet();
    return eth;
}

void net_handler(char *name, void *data) {
    _netif.extra = NULL;

    _dns_server = ip_aton("10.0.2.3");

    placeholder_dhcp();

    void *tmp = malloc(1024);
    size_t packet_size = write_arp_request(tmp, ip_aton("10.0.2.4"));
    _netif.send_packet(tmp, packet_size);
    free(tmp);

    while (1) {
        struct ethernet_packet *eth = net_receive();

        if (!eth)
            continue;

        printk("0x%x", eth);

        process_sleep_until(CP, 1, 0);
        context_switch(0);
    }

    process_exit(0);
}

void init_netif_funcs(get_mac_func_t mac_func, get_packet_func_t get_func,
                      send_packet_func_t send_func, char *device) {
    _netif.get_mac = mac_func;
    _netif.get_packet = get_func;
    _netif.send_packet = send_func;
    _netif.driver = device;
    memcpy(_netif.hwaddr, _netif.get_mac(), sizeof(_netif.hwaddr));

    if (!tasklet_pid) {
        tasklet_pid = process_spawn_tasklet(net_handler, "[net]", NULL);
    }
}
