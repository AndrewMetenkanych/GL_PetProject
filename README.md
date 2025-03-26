# This repository contains a Linux compatible utility written in the C programming language, that allows you to test the availability of intermediate nodes on the network path from the client machine to the selected FQDN.
**Repository structure
<br />📂 GL_traceroute.c — source code of the utility
<br />📂 GL_traceroute — compiled executable file
<br />📂 openwrt-x86-64-generic-ext4-combined.img.gz — compiled OpenWrt image with the utility**
<br />📂 **.gitignore file**  
## The main steps of the code are:
<br /> 1. Creating a raw socket for sending ICMP packets.
<br /> 2. Sending an ICMP Echo Request with an incremental TTL (Time-to-Live).
<br /> 3. Waiting for an ICMP response (ICMP Time Exceeded or ICMP Echo Reply).
<br /> 4. Measuring the response time of each node.
<br /> 5. Determining the domain name of the node (if possible).
<br /> 6. Terminating when the target host is reached or when the maximum TTL is reached.
## <br /> How to start?
 **On linux:** 
<br /> **Compilation:**
<br /> gcc GL_traceroute.c -o GL_traceroute
<br /> **Start the utility(root access required for RAW sockets):**
<br /> sudo ./GL_traceroute 8.8.8.8 (or any other address)
<br /> **Output example:**
![image](https://github.com/user-attachments/assets/628125d3-8ce8-4313-b4ba-3eac9a128e5f)
![image](https://github.com/user-attachments/assets/ac1e9dea-e8de-4af3-9e2d-379833289037)
<br /> **On OpenWrt**
<br /> Use the OpenWrt Installation guide from (https://openwrt.org/docs/guide-user/virtualization/virtualbox-vm) to install OS on virtual machine.
<br /> **Start the utility:**
<br /> GL_traceroute 8.8.8.8 (or any other address).
<br /> **Output example:**
![image](https://github.com/user-attachments/assets/e7828f9b-0474-49e1-8dfb-de13204aed56)
![image](https://github.com/user-attachments/assets/a164db7b-be1f-4c4b-b968-50cda65c7a2e)
