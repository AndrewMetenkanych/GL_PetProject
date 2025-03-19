#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/ip.h>

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025  
#endif

#define MAX_HOPS 30
#define PACKET_SIZE 64
#define TIMEOUT 1  // Timeout in seconds

struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE - sizeof(struct icmphdr)];
};

// Function for calculating the checksum of an ICMP packet
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;
    for (; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Creation ICMP Echo Request
void create_icmp_packet(struct icmp_packet *pkt, int seq) {
    memset(pkt, 0, sizeof(struct icmp_packet));
    pkt->hdr.type = ICMP_ECHO;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = getpid();
    pkt->hdr.un.echo.sequence = seq;
    memset(pkt->msg, 0xAA, sizeof(pkt->msg));
    pkt->hdr.checksum = 0;
    pkt->hdr.checksum = checksum(pkt, sizeof(struct icmp_packet));
}

// Determining a domain name by IP address
void resolve_hostname(struct in_addr *addr) {
    char host[NI_MAXHOST];
    if (getnameinfo((struct sockaddr *)&(struct sockaddr_in){.sin_family = AF_INET, .sin_addr = *addr},
                    sizeof(struct sockaddr_in), host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {
        printf(" (%s)", host);
    }
}

// The main function of traceroute
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <destination IP>\n", argv[0]);
        return 1;
    }

    const char *dest_ip = argv[1];
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, dest_ip, &dest.sin_addr);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Tracing route to %s\n", dest_ip);

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            perror("setsockopt(IP_TTL) failed");
            close(sockfd);
            return 1;
        }

        int ttl_check;
        socklen_t len = sizeof(ttl_check);
        if (getsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl_check, &len) == 0) {
            printf("[DEBUG] TTL set to: %d\n", ttl_check);
        } else {
            perror("getsockopt(IP_TTL) failed");
        }

        struct icmp_packet pkt;
        create_icmp_packet(&pkt, ttl);

        struct sockaddr_in sender;
        char recv_buf[512];
        socklen_t addr_len = sizeof(sender);

        struct timeval start, end;
        gettimeofday(&start, NULL);

        if (sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
            perror("Packet send failed");
            break;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        struct timeval timeout = {TIMEOUT, 0};

        if (select(sockfd + 1, &readfds, NULL, NULL, &timeout) == 0) {
            printf("%d *\n", ttl);
            continue;
        }

        int bytes_received = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &sender, &addr_len);
        if (bytes_received < 0) {
            continue;
        }

        gettimeofday(&end, NULL);

        struct iphdr *ip_hdr = (struct iphdr *)recv_buf;
        struct icmphdr *icmp_hdr = (struct icmphdr *)(recv_buf + (ip_hdr->ihl * 4));

        printf("[DEBUG] Received ICMP type: %d, code: %d from %s\n", icmp_hdr->type, icmp_hdr->code, inet_ntoa(sender.sin_addr));

        if (icmp_hdr->type == ICMP_TIME_EXCEEDED) {
            printf("%d %s", ttl, inet_ntoa(sender.sin_addr));
            resolve_hostname(&sender.sin_addr);
            printf("  %.2f ms\n", (end.tv_usec - start.tv_usec) / 1000.0);
        } else if (icmp_hdr->type == ICMP_ECHOREPLY) {
            printf("%d %s", ttl, inet_ntoa(sender.sin_addr));
            resolve_hostname(&sender.sin_addr);
            printf("  %.2f ms\n", (end.tv_usec - start.tv_usec) / 1000.0);
            printf("Trace complete.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
