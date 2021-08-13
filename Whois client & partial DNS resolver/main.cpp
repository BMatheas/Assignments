#include <iostream>
#include <string>
#include "ipstack.hpp"
#include "dns.hpp"
#include "whois.hpp"

#define NI_MAXHOST      1025
#define NI_MAXSERV      32

/***************************************************************************************
*   Function: parse_arguments
*   ----------------------------
*	Function checks the arguments
*   Param1: argc
*	Param2: argv
*	Param3: Pointer used to return -w value
*	Param4: Pointer used to return -q value
*   Returns: void
****************************************************************************************
*    Title: GETOPT(3)
*    Date: 2019-03-06
*    Availability: http://man7.org/linux/man-pages/man3/getopt.3.html
*
***************************************************************************************/
void parse_arguments(int p_argc, char** p_argv, char** W_VAL, char** Q_VAL)
{
	int opt;
	while((opt = getopt(p_argc, p_argv, "q:w:")) != -1)
	{
		switch(opt)
		{
			case 'q':
				*Q_VAL = optarg;
				break;
			case 'w':
				*W_VAL = optarg;
				break;
			default:
				fprintf(stderr, "Usage: %s -q IP/hostname -w whois_server\n", p_argv[0]);
				exit(10);
				break;
		}
	}
}

/***************************************************************************************
*   Function: get_addr_hostname
*   ----------------------------
*	Function gets hostname of IPv4 address
*   Param1: IPv4 address
*   Returns: hostname of IPv4 address
****************************************************************************************
*    Title: getnameinfo() example problem
*    Date: 03-07-2016
*	 Author: algorism
*    Availability: https://cboard.cprogramming.com/c-programming/169902-getnameinfo-example-problem.html
*
***************************************************************************************/
char* get_addr_hostname(char* address)
{
	struct sockaddr_in sa;     /* input */
    char hbuf[NI_MAXHOST];
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET; 
    inet_pton(AF_INET, address, &sa.sin_addr);
    char* ret_hostname = NULL;

    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD))
        return NULL;
    else
    {
    	ret_hostname = (char*) malloc(sizeof(char)*(strlen(hbuf)+1));
    	if(!ret_hostname)
    		return NULL;
    	if(strstr(hbuf, "www."))
    		strcpy(ret_hostname, hbuf+4);
    	else
    		strncpy(ret_hostname, hbuf, strlen(hbuf));
    	return ret_hostname;
    }
}

/***************************************************************************************
*   Function: get_addr6_hostname
*   ----------------------------
*	Function gets hostname of IPv6 address
*   Param1: IPv6 address
*   Returns: hostname of IPv6 address
****************************************************************************************
*    Title: getnameinfo() example problem
*    Date: 03-07-2016
*	 Author: algorism
*    Availability: https://cboard.cprogramming.com/c-programming/169902-getnameinfo-example-problem.html
*	 Note: modified for IPv6
***************************************************************************************/
char* get_addr6_hostname(char* address)
{
	struct sockaddr_in6 sa;     /* input */
    char hbuf[NI_MAXHOST];
    memset(&sa, 0, sizeof(sa));
    sa.sin6_family = AF_INET6; 
    inet_pton(AF_INET6, address, &sa.sin6_addr);
    char* ret_hostname = NULL;

    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD))
        return NULL;
    else
    {
    	ret_hostname = (char*) malloc(sizeof(char)*(strlen(hbuf)+1));
    	if(!ret_hostname)
    		return NULL;
    	if(strstr(hbuf, "www."))
    		strcpy(ret_hostname, hbuf+4);
    	else
    		strncpy(ret_hostname, hbuf, strlen(hbuf));
    	return ret_hostname;
    }
}

int main(int argc, char** argv)
{
	char* W_VAL = NULL, *Q_VAL = NULL, *dns_hostname = NULL;
	std::string ipver6, ipver4, whois_server;

	/* CHECKING AND GETTING ARGUMENT VALUES */
	parse_arguments(argc, argv, &W_VAL, &Q_VAL);
	
	/* CHECKING IF ADDRESS ASSIGN TO PTR WAS CORRECT */
	if(!Q_VAL || !W_VAL)
	{
		printf("ERROR ASSIGNING ADDRESS\n");
		return 20;
	}

	/* GET AND PRINT DNS STATS */
	print_DNS_stats(Q_VAL, &ipver4, &ipver6);

	/* GET AND PRINT IPSTACK STATS */
	/* IF -q argument value is IPv4 or IPv6 address */
	if(isValidIpAddress(Q_VAL) || isValidIp6Address(Q_VAL))
		print_ip_stack(Q_VAL);
	/*IF -q argument is hostname */
	else
	{
		if(!ipver4.empty())
			print_ip_stack((char*)ipver4.c_str());
		else if(!ipver6.empty())
			print_ip_stack((char*)ipver6.c_str());
		else
			std::cout << "\nNepodarilo sa ziskat zaznam IPSTACK\n";
	}

	/* IF hostname in -q argument contains "www.", we need to delete it*/
	if(strstr(Q_VAL, "www."))
		strcpy(Q_VAL, Q_VAL+4);

	/*GET AND PRINT WHOIS STATS */
	if(!isValidIpAddress(W_VAL) && !isValidIp6Address(W_VAL))
	{
		/* Translate whois server hostname to IP */
		whois_server= HostToIp(std::string(W_VAL));
		if(whois_server.empty())
		{
			printf("ENTER VALID WHOIS SERVER\n");
			return 0;
		}
		/* Check if whois server gets answer for -q argument */
		if(get_whois(Q_VAL, const_cast<char*>(whois_server.c_str())) == 3)
		{
			/* IF -q argument is IPv4, tazatel will query WHOIS using hostname*/
			if(isValidIpAddress(Q_VAL))
			{
				/* Get hostname of IPv4 address */
				dns_hostname = get_addr_hostname(Q_VAL);
				if(!dns_hostname)
				{
					printf("ERROR GETTING HOSTNAME OF \"%s\"\n", Q_VAL);
					return 0;
				}
				/* tazatel will query WHOIS using hostname*/
				if(get_whois(dns_hostname, const_cast<char*>(whois_server.c_str())) == 3)
				{
					printf("Skuste inu kombinaciu\n");
					return 0;
				}
			}
			/* IF -q argument is IPv6, tazatel will query WHOIS using hostname*/
			else if(isValidIp6Address(Q_VAL))
			{
				/* Get hostname of IPv6 address */
				dns_hostname = get_addr6_hostname(Q_VAL);
				if(!dns_hostname)
				{
					printf("ERROR GETTING HOSTNAME OF \"%s\"\n", Q_VAL);
					return 0;
				}
				/* tazatel will query WHOIS using hostname*/
				if(get_whois(dns_hostname, const_cast<char*>(whois_server.c_str())) == 3)
				{
					printf("Skuste inu kombinaciu\n");
					return 0;
				}
			}
			/* IF -q argument is hostname, tazatel will query WHOIS using IPv4*/
			else
			{
				if(get_whois(const_cast<char*>(ipver4.c_str()), const_cast<char*>(whois_server.c_str())) == 3)
				{
					printf("Skuste inu kombinaciu\n");
					return 0;
				}
			}
		}
	}
	else
	{
		/* Check if whois server gets answer for -q argument */
		if(get_whois(Q_VAL, W_VAL) == 3)
		{
			/* IF -q argument is IPv4, tazatel will query WHOIS using hostname*/
			if(isValidIpAddress(Q_VAL))
			{
				/* Get hostname of IPv4 address */
				dns_hostname = get_addr_hostname(Q_VAL);
				if(!dns_hostname)
				{
					printf("ERROR GETTING HOSTNAME OF \"%s\"\n", Q_VAL);
					return 0;
				}
				/* tazatel will query WHOIS using hostname*/
				if(get_whois(dns_hostname, W_VAL) == 3)
				{
					printf("Skuste inu kombinaciu\n");
					return 0;
				}
			}
			/* IF -q argument is IPv6, tazatel will query WHOIS using hostname*/
			else if(isValidIp6Address(Q_VAL))
			{
				/* Get hostname of IPv6 address */
				dns_hostname = get_addr6_hostname(Q_VAL);
				if(!dns_hostname)
				{
					printf("ERROR GETTING HOSTNAME OF \"%s\"\n", Q_VAL);
					return 0;
				}
				/* tazatel will query WHOIS using hostname*/
				if(get_whois(dns_hostname, W_VAL) == 3)
				{
					printf("Skuste inu kombinaciu\n");
					return 0;
				}
			}
			/* IF -q argument is hostname, tazatel will query WHOIS using IPv4*/
			else
			{
				if(get_whois(const_cast<char*>(ipver4.c_str()), W_VAL) == 3)
				{
					printf("Skuste inu kombinaciu\n");
					return 0;
				}
			}
		}
	}
	return 0;
}
