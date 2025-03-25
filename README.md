This repository contains a utility written in the C programming language, that allows you to test the availability of intermediate nodes on the network path from the client machine to the selected FQDN.
The main steps of the code are:
Creating a raw socket for sending ICMP packets.
Sending an ICMP Echo Request with an incremental TTL (Time-to-Live).
Waiting for an ICMP response (ICMP Time Exceeded or ICMP Echo Reply).
Measuring the response time of each node.
Determining the domain name of the node (if possible).
Terminating when the target host is reached or when the maximum TTL is reached.

Also for integration with Openwrt, a package with the utility was created as part of the build.
