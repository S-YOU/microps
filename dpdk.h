#ifndef __DPDK_H
#define __DPDK_H

//#include<stdint.h> //rte_eal.h
#include<rte_eal.h>

#include <inttypes.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include<rte_hexdump.h>
#include <rte_ether.h>
#include"microps.h"



/*****/
#define BURST_SIZE 32
#define RX_RING_SIZE 128
#define TX_RING_SIZE 512

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32



/*****/
static inline int port_init(uint16_t port/*, struct rte_mempool *mbuf_pool*/);
void dpdk_init(int argc, char **argv);
device_t *device_open(const char *name);
void device_input(device_t *device, void (*callback)(uint8_t *, size_t), int timeout);
ssize_t device_output(device_t *device, const uint8_t *buffer, size_t length);


#endif
