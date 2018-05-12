microps
=======

Micro TCP/IP Protocol Stack

build:

 $ make


usage:

 $ sudo ./echo_server device-name ethernet-addr ip-addr netmask default-gw


example (echo_server):

 $ sudo ./echo_server eth0 02:00:00:00:00:01 192.168.0.100 255.255.255.0 192.168.0.1


arp test:

 $ sudo arping 192.168.0.100


ping test:

 $ ping 192.168.0.100


echo test:

 $ nc -u 192.168.0.100 7


### microps ver dpdk-enable

preparation:  
 build DPDK and setup your machin to use DPDK. (please see http://dpdk.org/)

build:  
 $ make
 
usage:  
 $ sudo ./build/echo_server
 
ping test:  
 $ping 10.0.0.1
