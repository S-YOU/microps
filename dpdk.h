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




/*****/
#define BURST_SIZE 32
#define RX_RING_SIZE 128
#define TX_RING_SIZE 512

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

#endif
