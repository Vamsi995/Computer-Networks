#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
using namespace std;

// Ethernet Header Structure
struct EthernetHeader {

  u_int8_t h_dest[6]; // Destination MAC Address
  u_int8_t h_source[6]; // Source MAC Address
  u_int16_t ether_type; // Protocol type

}__attribute__ ((__packed__));

struct IPHeader {
    unsigned char  ip_verlen;        // 4-bit IPv4 version 4-bit header length (in 32-bit words)
    unsigned char  ip_tos;           // IP type of service
    unsigned short ip_totallength;   // Total length
    unsigned short ip_id;            // Unique identifier
    unsigned short ip_offset;        // Fragment offset field
    unsigned char  ip_ttl;           // Time to live
    unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
    unsigned short ip_checksum;      // IP checksum
    int   ip_srcaddr;       // Source address
    int   ip_destaddr;      // Source address
}__attribute__ ((__packed__));


// Processes the packet and extracts MAC and IP addresses
void processPacket(unsigned char *buffer)
{
    struct sockaddr_in source, dest;
    cout << "\n===================================\n" << endl;

    // Extracting ethernet header and printing source and destination MAC addresses
    struct EthernetHeader *eth = (struct EthernetHeader *)(buffer);
    cout << "MAC Addresses" << endl;
    printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
    printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);

    // Extracting IP header and printing source and destination IP addresses
    struct IPHeader *iph = (struct IPHeader *)(buffer + sizeof(struct EthernetHeader));
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));
    source.sin_addr.s_addr = iph->ip_srcaddr;
    dest.sin_addr.s_addr = iph->ip_destaddr;
    string source_ip(inet_ntoa(source.sin_addr));
    string dest_ip(inet_ntoa(dest.sin_addr));
    cout << "\nIP Addresses" << endl;
    cout << "\t|-Source IP: " << source_ip << endl;
    cout << "\t|-Destination IP: " << dest_ip << endl;
    cout << "\n===================================\n" << endl;
}

int main()
{

    // Initializing sockets
    int sock;
    int n, saddr_size;
    struct sockaddr_ll saddr;

    // Assigning interface to socket
    struct sockaddr_ll sladdr;
    sladdr.sll_family = AF_PACKET;
    sladdr.sll_ifindex = if_nametoindex("eth1");
    sladdr.sll_protocol = htons(ETH_P_IP);

    unsigned char *buffer = (unsigned char *) malloc(65536);

    // Creating raw sockets with ETH_P_IP protocol which listens to only IP packets on the network card
    if ((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
    {
        cerr << "Error: Creating socket" << endl;
        close(sock);
        exit(-1);
    }

    // Binding raw socket to interface eth1
    socklen_t sladdr_size = sizeof(sladdr);
    if(bind(sock, (const sockaddr *)&sladdr, sladdr_size) < 0)
    {
      cerr << "Error: Socket Binding Error" << endl;
      close(sock);
      exit(-1);
    }

     while(1)
    {
       saddr_size = sizeof(saddr);
       
       // recvfrom is a blocking call that listens on the interface
       n = recvfrom(sock, buffer, 65536, 0, (struct sockaddr*)&saddr, (socklen_t *)&saddr_size);
       
       // Filtering receiving packets only
       if((int)saddr.sll_pkttype != PACKET_OUTGOING)
       {
          if(n < 0)
          {
            perror("Error: Receive Error");
            close(sock);
            exit(-1);
          }

          // Extracting MAC, IP addresses
          processPacket(buffer);
       }
    }

    close(sock);
    return 0;
}