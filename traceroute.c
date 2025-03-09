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

#define MAX_HOPS 30
#define PACKET_SIZE 64
#define TIMEOUT 1  // Таймаут у секундах

struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE - sizeof(struct icmphdr)];
};

// Функція для розрахунку контрольної суми ICMP-пакета
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *) buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Створення ICMP Echo Request
void create_icmp_packet(struct icmp_packet *pkt, int seq) {
    memset(pkt, 0, sizeof(struct icmp_packet));
    pkt->hdr.type = ICMP_ECHO;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = getpid();
    pkt->hdr.un.echo.sequence = seq;
    pkt->hdr.checksum = checksum(pkt, sizeof(struct icmp_packet));
}

// Визначення доменного імені по IP-адресі
void resolve_hostname(char *ip) {
    struct hostent *host = gethostbyaddr(ip, sizeof(struct in_addr), AF_INET);
    if (host)
        printf(" (%s)", host->h_name);
}

// Основна функція traceroute
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

    struct timeval timeout = {TIMEOUT, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    printf("Tracing route to %s\n", dest_ip);

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++) {
        setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

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

        if (recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &sender, &addr_len) < 0) {
            printf("%d *\n", ttl);
            continue;
        }

        gettimeofday(&end, NULL);
        double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender.sin_addr, ip, sizeof(ip));

        printf("%d %s", ttl, ip);
        resolve_hostname(ip);
        printf("  %.2f ms\n", rtt);

        if (strcmp(ip, dest_ip) == 0) {
            printf("Trace complete.\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
