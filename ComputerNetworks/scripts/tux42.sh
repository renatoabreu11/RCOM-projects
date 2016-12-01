#!/bin/bash
/etc/init.d/networking restart
ifconfig eth0 up
ifconfig
ifconfig eth0 172.16.41.1/24
route add default gw 172.16.41.254
route -n
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
echo 1 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 1 > /proc/sys/net/ipv4/conf/all/accept_redirects
