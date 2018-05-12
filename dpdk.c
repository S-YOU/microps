/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"dpdk.h"
#include"microps.h"

//#include<net/ethernet.h>
//#include<netinet/if_ether.h>


struct device {
	int fd;
};

struct rte_mempool *mbuf_pool;

static const struct rte_eth_conf port_conf_default = {
	.rxmode = { .max_rx_pkt_len = ETHER_MAX_LEN }
};


void hexdump(u_int16_t *buf, int size){
  int i;
  for (i = 0;i < size; i++){
    fprintf(stdout, "%04x ", *(buf + i));
    if ((i + 1) % 8 == 0){ 
      fprintf(stdout, "\n");
    }   
  }
  fprintf(stdout, "\nfin\n");
}

void print_mbuf(const struct rte_mbuf *bufs){
	printf("-----print_mbuf-----\n");

	//printf("rearm_data: %u\n", bufs->rearm_data);

	printf("data_off: %u\n", bufs->data_off);
	printf("refcnt: %u\n", bufs->refcnt);
	printf("nb_segs: %u\n", bufs->nb_segs);
	printf("port: %u\n", bufs->port);
	printf("ol_flags: %u\n", bufs->ol_flags);
	printf("packet_type: %u\n", bufs->packet_type);
	printf("pkt_len: %u\n", bufs->pkt_len);
	printf("data_len: %u\n", bufs->data_len);
	printf("vlan_tci: %u\n", bufs->vlan_tci);
	printf("rss: %u\n", bufs->hash.rss);
	printf("vlan_tci_outer: %u\n", bufs->vlan_tci_outer);
	printf("buf_len: %u\n", bufs->buf_len);
	printf("timestamp: %u\n", bufs->timestamp);
	printf("udata: %u\n", bufs->udata64);
	printf("tx_offlead: %u\n", bufs->tx_offload);
	printf("priv_size: %u\n", bufs->priv_size);
	printf("timesync: %u\n", bufs->timesync);
	printf("seqn: %u\n", bufs->seqn);
	printf("--------------------\n");
}


/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline int
port_init(uint16_t port/*, struct rte_mempool *mbuf_pool*/)
{ 
  struct rte_eth_conf port_conf = port_conf_default;
  const uint16_t rx_rings = 1, tx_rings = 1;
  uint16_t nb_rxd = RX_RING_SIZE;
  uint16_t nb_txd = TX_RING_SIZE;
  int retval;
  uint16_t q;
  
  if (port >= rte_eth_dev_count())
    return -1;
  
  /* Configure the Ethernet device. */ 
  retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
  if (retval != 0)
    return retval;
  
  retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
  if (retval != 0)
    return retval;
  
  /* Allocate and set up 1 RX queue per Ethernet port. */
  for (q = 0; q < rx_rings; q++) {
    retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
        rte_eth_dev_socket_id(port), NULL, mbuf_pool);
    if (retval < 0)
      return retval;
  }
  
  /* Allocate and set up 1 TX queue per Ethernet port. */
  for (q = 0; q < tx_rings; q++) {
    retval = rte_eth_tx_queue_setup(port, q, nb_txd,
        rte_eth_dev_socket_id(port), NULL);
    if (retval < 0)
      return retval;
  }
  
  /* Start the Ethernet port. */
  retval = rte_eth_dev_start(port);
  if (retval < 0)
    return retval;

  /* Display the port MAC address. */
  struct ether_addr addr;
  rte_eth_macaddr_get(port, &addr);
  printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
         " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
      port,
      addr.addr_bytes[0], addr.addr_bytes[1],
      addr.addr_bytes[2], addr.addr_bytes[3],
      addr.addr_bytes[4], addr.addr_bytes[5]);
  
  /* Enable RX in promiscuous mode for the Ethernet device. */
  rte_eth_promiscuous_enable(port);

  return 0;
}


void dpdk_init(int argc, char **argv){
	//struct rte_mempool *mbuf_pool;
	int ret;
	unsigned nb_ports;
	uint16_t portid;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");                  

	argc -= ret;
	argv += ret;

	nb_ports = rte_eth_dev_count();
	if (nb_ports != 1)
		rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");
	
	/* Creates a new mempool in memory to hold the mbufs. */
  mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
    MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

  if (mbuf_pool == NULL)
    rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

  /* Initialize all ports. */
  for (portid = 0; portid < nb_ports; portid++)
    if (port_init(portid/*, mbuf_pool*/) != 0)
      rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n",
          portid);

  if (rte_lcore_count() > 1)
    printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");
}









device_t *device_open (const char *name) {
	printf("head of device_open\n");
	device_t *device;
	if ((device = malloc(sizeof(*device))) == NULL) {
		perror("malloc");
		exit(1);
	}
	return device;
}

void device_close (device_t *device) {
    //if (device->fd != -1) {
    //    close(device->fd);
    //}
    free(device);
}




//target api of input
void
device_input (device_t *device, void (*callback)(uint8_t *, size_t), int timeout) {
    //struct pollfd pfd;
    //ssize_t ret, length;
    //uint8_t buffer[2048];

    //pfd.fd = device->fd;
    //pfd.events = POLLIN;
    //ret = poll(&pfd, 1, timeout);
    //if (ret <= 0) {
    //    return;
    //}   
    //length = read(device->fd, buffer, sizeof(buffer));
    //if (length <= 0) {
    //    return;
    //}   
    //callback(buffer, length);

 //return write(device->fd, buffer, length);

 //printf("head of device_input\n");

	const uint16_t nb_ports = rte_eth_dev_count();
	uint16_t port;


  /*
   * Check that the port is on the same NUMA node as the polling thread
   * for best performance.
   */ 
  for (port = 0; port < nb_ports; port++)
    if (rte_eth_dev_socket_id(port) > 0 &&
        rte_eth_dev_socket_id(port) !=
            (int)rte_socket_id())
      printf("WARNING, port %u is on remote NUMA node to "
          "polling thread.\n\tPerformance will "
          "not be optimal.\n", port);
  
 // printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
 //     rte_lcore_id());

	

	port = 0;
	/* Recv burst of RX packets */
	//struct rte_mbuf *bufs[BURST_SIZE];

	//printf("before rx_burst\n");

	struct rte_mbuf *bufs[BURST_SIZE];
	const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);

	int i;
	for (i = 0; i < nb_rx ; i++){
		uint8_t *p = rte_pktmbuf_mtod(bufs[i], uint8_t*);
		size_t size = rte_pktmbuf_pkt_len(bufs[i]);
	
	//FILE *fp;
	//fp = fopen("txit.txt", "a");
	hexdump(p, size);
	print_mbuf(bufs[i]);

	//fprintf(fp, "%s\n", hexdump(bufs, nb_rx));
	//close(fp);

	callback(p, size);
	}
	//return rte_eth_tx_burst(port, 0, buffer, 1);

}









//target api of output
ssize_t
device_output (device_t *device, const uint8_t *buffer, size_t length) {
 //return write(device->fd, buffer, length);

	printf("head of device_output\n");
	struct rte_mbuf *bufs[BURST_SIZE];
	const uint16_t nb_ports = rte_eth_dev_count();
	uint16_t port;


  /*
   * Check that the port is on the same NUMA node as the polling thread
   * for best performance.
   */ 
  for (port = 0; port < nb_ports; port++)
    if (rte_eth_dev_socket_id(port) > 0 &&
        rte_eth_dev_socket_id(port) !=
            (int)rte_socket_id())
      printf("WARNING, port %u is on remote NUMA node to "
          "polling thread.\n\tPerformance will "
          "not be optimal.\n", port);
  
  printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
      rte_lcore_id());

	port = 0;
	/* Send burst of TX packets */
	//struct rte_mbuf *bufs[BURST_SIZE];
	
	printf("rte_pktmbuf_alloc\n");
	bufs[0] = rte_pktmbuf_alloc(mbuf_pool);
	print_mbuf(bufs[0]);
	bufs[0]->pkt_len = length;
	bufs[0]->data_len = length;
	//print_mbuf(bufs[0]);

	printf("strncpy\n");
	//uint8_t *p = rte_pktmbuf_append(bufs[0], length);
	uint8_t *p = rte_pktmbuf_mtod(bufs[0], uint8_t*);
	memcpy(p, buffer, length);
	
	/******/
	//struct ether_arp *pp = p + sizeof(struct ether_header);
	//struct ether_arp *buffer_e += sizeof(struct ether_header);
	//strncpy(pp->ea_hdr, buffer_e->ea_hdr, strlen(buffer->ea_hdr));
	/******/

	print_mbuf(bufs[0]);
	printf("port\n");
	bufs[0]->port = 0;
	printf("packet_type\n");
	bufs[0]->packet_type = 1;
	print_mbuf(bufs[0]);
	printf("len\n");
	//printf("rte_pktmbuf_read\n");
	//bufs[0] = rte_pktmbuf_read(bufs[0], 0, length, buffer);
	////print_mbuf(bufs[0]);

	//printf("rte_get_ptypte_name\n");
	////bufs[0]->packet_type = (uint32_t)193;
	//if (rte_get_ptype_name(bufs[0]->packet_type, buffer, length) == -1){
	//	printf("rte_get_ptype_name\n");
	//	exit(1);
	//}

	//print_mbuf(bufs[0]);
	
	//printf("mbuf test\n");
	//uint8_t *p = rte_pktmbuf_mtod(bufs[0], uint8_t*);
	//hexdump(buffer, length);
	printf("*****\n");
	hexdump(p, length);

	printf("before tx_burst\n");

	if (rte_eth_tx_burst(port, 0, bufs, 1) > 0){
		return length;
	}
	return -1;
}



