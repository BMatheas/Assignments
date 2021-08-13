#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include <pcap.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <algorithm>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <csignal>

#define DATA "datastring"

using namespace std;

struct pseudoTCPPacket {
  uint32_t srcAddr;
  uint32_t dstAddr;
  uint8_t zero;
  uint8_t protocol;
  uint16_t TCP_len;
};

unsigned short csum(unsigned short *ptr,int nbytes) {
  long sum;
  unsigned short oddbyte;
  short answer;

  sum=0;
  while(nbytes>1) {
    sum+=*ptr++;
    nbytes-=2;
  }
  if(nbytes==1) {
    oddbyte=0;
    *((u_char*)&oddbyte)=*(u_char*)ptr;
    sum+=oddbyte;
  }

  sum = (sum>>16)+(sum & 0xffff);
  sum = sum + (sum>>16);
  answer=(short)~sum;

  return(answer);
}

void parse_arg(int p_argc, char** p_argv, string *int_name, string *UDP_ports, string *TCP_ports, string *arg_host)
{
    for(int i = 1; i < p_argc; i++)
    {
        if(!strcmp(p_argv[i], "-i"))
        {
            *int_name = string(p_argv[i+1]);
            i++;
        }
        else if(!strcmp(p_argv[i], "-pu"))
        {
            *UDP_ports = string(p_argv[i+1]);
            i++;

        }
        else if(!strcmp(p_argv[i], "-pt"))
        {
            *TCP_ports = string(p_argv[i+1]);
            i++;
        }
        else
            *arg_host = p_argv[i];
    }
    if(arg_host->empty())
    {
        cerr << "Nebol zadany host" << endl;
        exit(10);
    }
}

int *parse_TCP_ports(string TCP_ports, int* tag)
{
    size_t found = TCP_ports.find(',');
    int *token;
    if(found != string::npos)
    {
        size_t char_count = count(TCP_ports.begin(), TCP_ports.end(),',');
        token = (int*)malloc(sizeof(int)*(char_count+1));
        /***************************************************************************************
        *    Title: Parse (split) a string in C++ using string delimiter (standard C++)
        *    Author: Vincenzo Pii
        *    Date: 10. Januar 2013
        *    Availability: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
        ***************************************************************************************/
        size_t pos = 0;
        int i = 0;
        while((pos = TCP_ports.find(',')) != string::npos)
        {
            token[i] = stoi(TCP_ports.substr(0,pos));
            TCP_ports.erase(0, pos+1);
            i++;
        }
        token[i] = stoi(TCP_ports);
        for(int a = 0; a <= i; a++)
            if(token[a] > 65535 || token[a] < 0)
                exit(10);
        *tag = 1;
    }
    else
    {
        found = TCP_ports.find('-');
        if(found != string::npos)
        {
            size_t pos = 0;
            token = (int*)malloc(sizeof(int)*2);
            pos = TCP_ports.find('-');
            token[0] = stoi(TCP_ports.substr(0,pos));
            TCP_ports.erase(0, pos+1);
            token[1] = stoi(TCP_ports);
            if(token[0] > 65535 || token[0] < 0)
                exit(10);
            if(token[1] > 65535 || token[1] < 0)
                exit(10);
            *tag = 2;
        }
        else
        {
            token = (int*)malloc(sizeof(int));
            *token = stoi(TCP_ports);
            if(token[0] > 65535 || token[0] < 0)
                exit(10);
            *tag = 3;
        }
    }
    return token;
}
/***************************************************************************************
*    Title: VALIDATE IPv4 Address
*    Author: Raymond Martineau
*    Date: 25. November 2008
*    Availability: https://stackoverflow.com/questions/318236/how-do-you-validate-that-a-string-is-a-valid-ipv4-address-in-c 
***************************************************************************************/
bool validateIpAddress(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}
/***************************************************************************************
*    Title: GET IP FROM HOSTNAME
*    Author: Neel Patel
*    Date: 22. Februar 2012
*    Availability: https://stackoverflow.com/questions/9400756/ip-address-from-host-name-in-windows-socket-programming
***************************************************************************************/
/* GET IP FROM HOSTNAME */
string HostToIp(const std::string& host) {
    hostent* hostname = gethostbyname(host.c_str());
    if(hostname)
        return std::string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
    return {};
}
/***************************************************************************************
*    Title: <Get the IP address of a network interface in C using SIOCGIFADDR>
*    Author:  Graham Shaw
*    Availability: http://www.microhowto.info/howto/get_the_ip_address_of_a_network_interface_in_c_using_siocgifaddr.html
***************************************************************************************/
string GetIPFromInt(string if_name)
{
    struct ifreq ifr;
    size_t if_name_len=if_name.length();
    if (if_name_len<sizeof(ifr.ifr_name)) 
    {
        memcpy(ifr.ifr_name, if_name.c_str(),if_name_len);
        ifr.ifr_name[if_name_len]=0;
    } 
    else 
        exit(10);
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if (fd==-1)
    {
        cerr << "SOCKET NOK" << endl;
        exit(20);
    }
    if (ioctl(fd,SIOCGIFADDR,&ifr)==-1)
    {
        close(fd);
        cerr << "IOCTL NOK" << strerror(errno) << endl;
        exit(20);
    }
    close(fd);
    struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    return inet_ntoa(ipaddr->sin_addr);
}

int main(int argc, char **argv)
{
    string int_name, UDP_ports, TCP_ports, arg_host, HOST_IP, SOURCE_IP;
    /* PARSOVANIE ARGUMENTOV */
    parse_arg(argc, argv, &int_name, &UDP_ports, &TCP_ports, &arg_host);

    int *TP, TP_tag, UP_tag;
    /* KONTROLA TCP a UDP portov */
    TP = parse_TCP_ports(TCP_ports, &TP_tag);

    if(!validateIpAddress(arg_host))
        HOST_IP = HostToIp(arg_host);
    else
        HOST_IP = arg_host;

    /* PCAP BLBOSTI */
    pcap_t *handle; /* Session handle */
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    if(int_name.empty())
    {
        cout << "Nezadal som i" << endl;
        dev = pcap_lookupdev(errbuf);
        if (dev == NULL) {
            fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
            return(2);
        }
    }
    else
        dev = (char*)int_name.c_str(); /* The device to sniff on */

    /***************************************************************************************
    *    Title: Programming with pcap
    *    Author: Guy Harris
    *    Availability: https://www.tcpdump.org/pcap.html
    ***************************************************************************************/
    struct bpf_program fp;      /* The compiled filter */
    char filter_exp[] = "port 631";  /* The filter expression */
    bpf_u_int32 mask;       /* Our netmask */
    bpf_u_int32 net;        /* Our IP */
    struct pcap_pkthdr *header;  /* The header that pcap gives us */
    const u_char *packeti;       /* The actual packet */

    /* Find the properties for the device */
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
        net = 0;
        mask = 0;
    }
    /* Open the session in promiscuous mode */
    handle = pcap_open_live(dev, BUFSIZ, 1, 500, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return(2);
    }

    /***************************************************************************************
    *    Title: TCP/IP Header
    *    Author: rbaron
    *    Date: 20 Nov 2012
    *    Code version: 58abbc9
    *    Availability: https://github.com/rbaron/raw_tcp_socket/blob/master/raw_tcp_socket.c
    ***************************************************************************************/
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(HOST_IP.c_str());
    const int on = 1;
    int sd;

    struct iphdr *ipHdr;
    struct tcphdr *tcpHdr;
    char packet[512];
    char *data;
    struct pseudoTCPPacket pTCPPacket;
    char *pseudo_packet;
    memset(packet, 0, sizeof(packet));
    ipHdr = (struct iphdr *) packet;
    tcpHdr = (struct tcphdr *) (packet + sizeof(struct iphdr));
    data = (char *) (packet + sizeof(struct iphdr) + sizeof(struct tcphdr));
    strcpy(data, DATA);
    ipHdr->ihl = 5; //5 x 32-bit words in the header
    ipHdr->version = 4; // ipv4
    ipHdr->tos = 0;// //tos = [0:5] DSCP + [5:7] Not used, low delay
    ipHdr->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(data); //total lenght of packet. len(data) = 0
    ipHdr->id = htons(54321); // 0x00; //16 bit id
    ipHdr->frag_off = 0x00; //16 bit field = [0:2] flags + [3:15] offset = 0x0
    ipHdr->ttl = 0xFF; //16 bit time to live (or maximal number of hops)
    ipHdr->protocol = IPPROTO_TCP; //TCP protocol
    ipHdr->check = 0; //16 bit checksum of IP header. Can't calculate at this point
    ipHdr->saddr = inet_addr(GetIPFromInt(dev).c_str()); //32 bit format of source address
    ipHdr->daddr = inet_addr(HOST_IP.c_str()); //32 bit format of source address

    //Now we can calculate the check sum for the IP header check field
    ipHdr->check = csum((unsigned short *) packet, ipHdr->tot_len); 

    //Populate tcpHdr
    tcpHdr->source = htons(35000); //16 bit in nbp format of source port
    tcpHdr->seq = 0x0; //32 bit sequence number, initially set to zero
    tcpHdr->ack_seq = 0x0; //32 bit ack sequence number, depends whether ACK is set or not
    tcpHdr->doff = 5; //4 bits: 5 x 32-bit words on tcp header
    tcpHdr->res1 = 0; //4 bits: Not used
    tcpHdr->cwr = 0; //Congestion control mechanism
    tcpHdr->ece = 0; //Congestion control mechanism
    tcpHdr->urg = 0; //Urgent flag
    tcpHdr->ack = 0; //Acknownledge
    tcpHdr->psh = 0; //Push data immediately
    tcpHdr->rst = 0; //RST flag
    tcpHdr->syn = 1; //SYN flag
    tcpHdr->fin = 0; //Terminates the connection
    tcpHdr->window = htons(155);//0xFFFF; //16 bit max number of databytes 
    tcpHdr->check = 0; //16 bit check sum. Can't calculate at this point
    tcpHdr->urg_ptr = 0; //16 bit indicate the urgent data. Only if URG flag is set

    //Now we can calculate the checksum for the TCP header
    pTCPPacket.srcAddr = inet_addr(GetIPFromInt(dev).c_str()); //32 bit format of source address
    pTCPPacket.dstAddr = inet_addr(HOST_IP.c_str()); //32 bit format of source address
    pTCPPacket.zero = 0; //8 bit always zero
    pTCPPacket.protocol = IPPROTO_TCP; //8 bit TCP protocol

    if ((sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("raw socket");
        exit(1);
    }

    if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL,(char* )&on, sizeof(on)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    int length = sizeof(serv_addr);
    const u_char *pkt_data;
    int pkt_ret;

    /* PKT DATA RETRIEVE */
    struct ether_header *ethernet;
    struct iphdr *ip_ret;
    struct tcphdr *hdr_ret;
    cout << "Interesting TCP ports on " << HOST_IP << endl;
    cout << "PORT" << "\t" << "STATE" << endl;
    if(TP_tag == 2 || TP_tag == 3)
    {
    int i, end;
    if(TP_tag == 2)
    {  
       i = TP[0];
       end = TP[1];
    }
    else
    {
       i = TP[0];
       end = TP[0]; 
    }
    while(i <= end)
    {
        if (pcap_compile(handle, &fp, "dst port 35000" , 0, net) == -1) {
            fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
            return(2);
        }
        if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
        }
        tcpHdr->dest = htons(i); //16 bit in nbp format of destination port

        pTCPPacket.TCP_len = htons(sizeof(struct tcphdr) + strlen(data)); // 16 bit length of TCP header

        pseudo_packet = (char *) malloc((int) (sizeof(struct pseudoTCPPacket) + sizeof(struct tcphdr) + strlen(data)));
        memset(pseudo_packet, 0, sizeof(struct pseudoTCPPacket) + sizeof(struct tcphdr) + strlen(data));

        //Copy pseudo header
        memcpy(pseudo_packet, (char *) &pTCPPacket, sizeof(struct pseudoTCPPacket));

        tcpHdr->seq = htonl(i);
        tcpHdr->check = 0;
        memcpy(pseudo_packet + sizeof(struct pseudoTCPPacket), tcpHdr, sizeof(struct tcphdr) + strlen(data));
        tcpHdr->check = (csum((unsigned short *) pseudo_packet, (int) (sizeof(struct pseudoTCPPacket) + sizeof(struct tcphdr) +  strlen(data))));

        if(sendto(sd,packet,ipHdr->tot_len, 0, (struct sockaddr *) &serv_addr, length) < 0)
        {
            cerr << "SEND NOK: " << errno << endl;
            return 10;
        }
        pkt_ret = pcap_next_ex(handle, &header, &pkt_data);
        /***************************************************************************************
        *    Title: Reading Packet Data With libpcap
        *    Author: PherricOxide
        *    Date: 27. September 2012
        *    Availability: https://stackoverflow.com/questions/12629080/reading-packet-data-with-libpcap
        ***************************************************************************************/
        ethernet = (struct ether_header*) pkt_data;
        if(ntohs (ethernet->ether_type) == ETHERTYPE_IP)
        {
            ip_ret = (struct iphdr*)(pkt_data + sizeof(struct ether_header));
            if(ip_ret->protocol == 6)
            {
                hdr_ret = (struct tcphdr*)((char*)ip_ret + sizeof(struct iphdr));
                if(hdr_ret->syn == 1 && hdr_ret->ack == 1)
                    cout << i << ":" << '\t' << " OPEN" <<endl;
                else if(hdr_ret->ack == 1 && hdr_ret->rst == 1)
                    cout << i << ":" << '\t' << "CLOSED" <<endl;
                else
                    cout << i << ":" << '\t' << "FILTERED" <<endl; 
            }
        }
        i++;
    }
    }

    if(TP_tag == 1)
    {
    size_t char_count = count(TCP_ports.begin(), TCP_ports.end(),',');
    for(int i = 0; i < (char_count+1); i++)
    {
        if (pcap_compile(handle, &fp, "dst port 35000" , 0, net) == -1) {
            fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
            return(2);
        }
        if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
        }
        tcpHdr->dest = htons(TP[i]); //16 bit in nbp format of destination port

        pTCPPacket.TCP_len = htons(sizeof(struct tcphdr) + strlen(data)); // 16 bit length of TCP header

        pseudo_packet = (char *) malloc((int) (sizeof(struct pseudoTCPPacket) + sizeof(struct tcphdr) + strlen(data)));
        memset(pseudo_packet, 0, sizeof(struct pseudoTCPPacket) + sizeof(struct tcphdr) + strlen(data));

        //Copy pseudo header
        memcpy(pseudo_packet, (char *) &pTCPPacket, sizeof(struct pseudoTCPPacket));

        tcpHdr->seq = htonl(i);
        tcpHdr->check = 0;
        memcpy(pseudo_packet + sizeof(struct pseudoTCPPacket), tcpHdr, sizeof(struct tcphdr) + strlen(data));
        tcpHdr->check = (csum((unsigned short *) pseudo_packet, (int) (sizeof(struct pseudoTCPPacket) + sizeof(struct tcphdr) +  strlen(data))));

        if(sendto(sd,packet,ipHdr->tot_len, 0, (struct sockaddr *) &serv_addr, length) < 0)
        {
            cerr << "SEND NOK: " << errno << endl;
            return 10;
        }
        if((pkt_ret = pcap_next_ex(handle, &header, &pkt_data)) < 0)
            return 20;
        ethernet = (struct ether_header*) pkt_data;
        if(ntohs (ethernet->ether_type) == ETHERTYPE_IP)
        {
            ip_ret = (struct iphdr*)(pkt_data + sizeof(struct ether_header));
            if(ip_ret->protocol == 6)
            {
                hdr_ret = (struct tcphdr*)((char*)ip_ret + sizeof(struct iphdr));
                if(hdr_ret->syn == 1 && hdr_ret->ack == 1)
                    cout << TP[i] << ":" << '\t' << " OPEN" <<endl;
                else if(hdr_ret->ack == 1 && hdr_ret->rst == 1)
                    cout  << TP[i] << ":" << '\t' << "CLOSED" <<endl;
                else
                    cout  << TP[i] << ":" << '\t' << "FILTERED" <<endl;
            }
        }
    }
    }
    pcap_close(handle);
    free(TP);
    return 0;
}