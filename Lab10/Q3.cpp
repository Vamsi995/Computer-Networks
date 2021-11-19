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

// Finding the largest element in an array
int largest(int arr[], int len)
{
    int max = 0;
    for (int i = 0; i < len; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }
    }
    return max;
}

// Processes the packet and extracts MAC and IP addresses
char *processPacket(unsigned char* buffer)
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

    return inet_ntoa(dest.sin_addr);
}


int main()
{

    // Initializing sockets, buffer, fdset
    int sock_eth1_recv, sock_eth2_recv, sock_eth1_send, sock_eth2_send;
    int n, saddr_size, send_length;
    fd_set fdset;
    struct sockaddr_ll saddr;
    unsigned char *buffer = (unsigned char *) malloc(65536);

    // Assigning interfaces
    struct sockaddr_ll sladdr_eth1;
    sladdr_eth1.sll_family = AF_PACKET;
    sladdr_eth1.sll_ifindex = if_nametoindex("eth1");
    sladdr_eth1.sll_protocol = htons(ETH_P_IP);

    struct sockaddr_ll sladdr_eth2;
    sladdr_eth2.sll_family = AF_PACKET;
    sladdr_eth2.sll_ifindex = if_nametoindex("eth2");
    sladdr_eth2.sll_protocol = htons(ETH_P_IP);


    // Extracting IP addresses of the interfaces
    struct ifreq ifr;
    int sock_eth1_ip = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth1", IFNAMSIZ - 1);
    ioctl(sock_eth1_ip, SIOCGIFADDR, &ifr);
    string eth1_ip(inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    strncpy(ifr.ifr_name, "eth2", IFNAMSIZ - 1);
    ioctl(sock_eth1_ip, SIOCGIFADDR, &ifr);
    string eth2_ip(inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    close(sock_eth1_ip);

    // Creating raw receiving sockets
    if ((sock_eth1_recv = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
    {
        cerr << "Error: Creating socket" << endl;
        close(sock_eth1_recv);
        exit(-1);
    }

    if ((sock_eth2_recv = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
    {
        cerr << "Error: Creating socket" << endl;
        close(sock_eth2_recv);
        exit(-1);
    }

    // Creating raw sending sockets
    if ((sock_eth1_send = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        cerr << "Error: Creating socket" << endl;
        close(sock_eth1_send);
        exit(-1);
    }

    if ((sock_eth2_send = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        cerr << "Error: Creating socket" << endl;
        close(sock_eth2_send);
        exit(-1);
    }

    // Binding receiving interfaces
    socklen_t sladdr_eth1_size = sizeof(sladdr_eth1);
    if(bind(sock_eth1_recv, (const sockaddr *)&sladdr_eth1, sladdr_eth1_size) < 0)
    {
      cerr << "Error: Socket Binding Error" << endl;
      close(sock_eth1_recv);
      exit(-1);
    }

    socklen_t sladdr_eth2_size = sizeof(sladdr_eth2);
    if(bind(sock_eth2_recv, (const sockaddr *)&sladdr_eth2, sladdr_eth2_size) < 0)
    {
      cerr << "Error: Socket Binding Error" << endl;
      close(sock_eth2_recv);
      exit(-1);
    }

    // Finding the maximum file descriptor value
    int arr[] = {sock_eth1_recv, sock_eth2_recv};
    /*
    According to the man page of select nfds is defined to be the first parameter of select
        This argument should be set to the highest-numbered file descriptor in any of the 
        three sets, plus 1. The indicated file descriptors in each set are checked, up to
        this limit 
    */
    int nfds = largest(arr, 2) + 1;
    int err; 

    while(1)
    {
     // Resetting the file descriptor set
      FD_ZERO(&fdset); 
      FD_SET(sock_eth1_recv, &fdset);
      FD_SET(sock_eth2_recv, &fdset);
      
      // Select returns the error code
      // Using select to handle multiple recvfrom blocking calls
      err = select(nfds, &fdset, NULL, NULL, NULL);

      if(err == 0)
      {
        cerr << "Time Out" << endl;
      }
      else if(err == -1)
      {
        cerr << "Error: Select Failed" << endl;
      }
      else
      {
        // Once any connection is ready on a socket, FD_ISSET(socket, file descriptor set) will become true
        if(FD_ISSET(sock_eth1_recv, &fdset))
        {
          saddr_size = sizeof(saddr);

          // Receiving from eth1
          n = recvfrom(sock_eth1_recv, buffer, 65536, 0, (struct sockaddr *)&saddr, (socklen_t *)&saddr_size);
          if(n < 0)
          {
            perror("Error: Receive Error");
            close(sock_eth1_recv);
            exit(-1);
          }

           // Filtering only received packets 
           if((int)saddr.sll_pkttype != PACKET_OUTGOING)
           {
             char *dest_ip = processPacket(buffer);
             string destination_ip(dest_ip);

             // Filterting packets not sent to r1
             if(destination_ip != eth1_ip)
             {
                // Defining destination address
                struct sockaddr_in out_addr;
                out_addr.sin_family = AF_INET; 
                out_addr.sin_addr.s_addr = inet_addr(dest_ip);

                // Discarding Ethernet Header
                buffer = (buffer + sizeof(struct EthernetHeader));
                
                socklen_t out_size = sizeof(out_addr);
                
                // Sending data
                if(sendto(sock_eth2_send, (const char *)buffer, n - sizeof(EthernetHeader), 0, (struct sockaddr *)&out_addr, out_size) < 0)
                {
                  perror("Error");
                  close(sock_eth2_send);
                  exit(-1);
                }
             }

           }
        }

        // Once any connection is ready on a socket, FD_ISSET(socket, file descriptor set) will become true
        else if(FD_ISSET(sock_eth2_recv, &fdset))
        {
          saddr_size = sizeof(saddr);

          // Receiving from eth2
          n = recvfrom(sock_eth2_recv, buffer, 65536, 0, (struct sockaddr *)&saddr, (socklen_t *)&saddr_size);
          unsigned char *ptr = buffer;
          
          if(n < 0)
          {
            perror("Error: Receive Error");
            close(sock_eth2_recv);
            exit(-1);
          }

          // Filtering only received packets 
          if((int)saddr.sll_pkttype != PACKET_OUTGOING)
          {
             char *dest_ip = processPacket(buffer);
             string destination_ip(dest_ip);

             // Filterting packets not sent to r1
             if(destination_ip != eth2_ip)
             {
                // Defining destination address
                struct sockaddr_in out_addr;
                out_addr.sin_family = AF_INET; 
                out_addr.sin_addr.s_addr = inet_addr(dest_ip);
                
                // Discarding Ethernet Header
                buffer = (buffer + sizeof(struct EthernetHeader));
                
                socklen_t out_size = sizeof(out_addr);
                
                // Sending data
                if(sendto(sock_eth2_send, (const char *)buffer, n - sizeof(EthernetHeader), 0, (struct sockaddr *)&out_addr, out_size) < 0)
                {
                  perror("Error");
                  close(sock_eth2_send);
                  exit(-1);
                }
             }
          }
        }
      }
    }

    close(sock_eth1_recv);
    close(sock_eth2_recv);
    close(sock_eth1_send);
    close(sock_eth2_send);
    return 0;
}