#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PACKET_SIZE 64


struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE - sizeof(struct icmphdr)];
};

// Function to create an ICMP Echo Request
void create_icmp_packet(struct icmp_packet *pkt, int seq) {
    memset(pkt, 0, sizeof(struct icmp_packet));
    pkt->hdr.type = ICMP_ECHO;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = getpid();
    pkt->hdr.un.echo.sequence = seq;
    pkt->hdr.checksum = 0;  // Checksum will be calculated later
}

// Function to send the ICMP packet
int send_icmp(int sockfd, struct sockaddr_in *dest, int ttl) {
    struct icmp_packet pkt;
    create_icmp_packet(&pkt, ttl);

    // Set TTL
    setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    // Send packet
    return sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr*)dest, sizeof(*dest));
}

int main() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    printf("Raw socket created successfully.\n");
    close(sockfd);
    return 0;
}
