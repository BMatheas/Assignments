#include "dns.hpp"
#include <arpa/inet.h>
#include <algorithm>

/***************************************************************************************
*   Function: find_digit
*   ----------------------------
*	Function finds first digit in string 
*   Param1: string 
*   Returns: index of first digit in string
****************************************************************************************/
std::size_t find_digit(std::string s)
{
	std::size_t idx = 0;
	for(unsigned int i = 0; i < s.length(); i++)
		if(isdigit(s[i]))
		{
			idx = i;
			break;
		}
	return idx;
}

/***************************************************************************************
*   Function: find_char
*   ----------------------------
*	Function finds first char in string 
*   Param1: string 
*   Returns: index of first char in string
****************************************************************************************/
std::size_t find_char(std::string s)
{
	std::size_t idx = 0;
	for(unsigned int i = 0; i < s.length(); i++)
		if(isalpha(s[i]))
		{
			idx = i;
			break;
		}
	return idx;
}


/***************************************************************************************
*   Function: isValidIpAddress
*   ----------------------------
*	Function determines if a string is valid IPv4 address
*   Param1: IPv4 address
*   Returns: <0,1>
****************************************************************************************
*   Title: Determine if a string is a valid IPv4 address in C
*   Author: Bill the Lizard
*   Date: Apr 27 '09
*   Availability: https://stackoverflow.com/questions/791982/determine-if-a-string-is-a-valid-ipv4-address-in-c 
*
***************************************************************************************/
int isValidIpAddress(char *ipAddress)
{
	struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

/***************************************************************************************
*   Function: isValidIp6Address
*   ----------------------------
*	Function determines if a string is valid IPv6 address
*   Param1: IPv6 address
*   Returns: <0,1>
*
*	Modification of function "isValidIpAddress" for IPv6
***************************************************************************************/
int isValidIp6Address(char *ipAddress)
{
	struct sockaddr_in6 sa;
    int result = inet_pton(AF_INET6, ipAddress, &(sa.sin6_addr));
    return result != 0;
}


/***************************************************************************************
*   Function: Format_DNS_response
*   ----------------------------
*	Function formats DNS response(MX, CNAME, NS, PTR)
*   Param1:	response from dns server
*	Param2:	key which will be used as delimiter
*	Param3: length of key
*   Returns: formated response
***************************************************************************************/
std::string Format_DNS_response(std::string s, std::string key, std::size_t indent)
{
	std::size_t found_key = s.rfind(key);
	s.erase(0, (found_key+indent));
	std::size_t found_char = find_char(s);
	return s.erase(0, found_char).substr(0, s.find_last_of("."));
}


/***************************************************************************************
*   Function: parse_answer
*   ----------------------------
*	Function parses response from DNS server and print it to STDOUT
*   Param1:	response from dns server
*	Param2:	return value from DNS query function indicating message length 
*	Param3: type of DNS Record requested
*	Param4: Pointer used to return IPv4 for later  use
*	Param5: Pointer used to return IPv6 for later  use
*   Returns: void
****************************************************************************************
*   Title: How to query a server and get the MX, A, NS records
*   Author: Dima00782
*   Date: Mar 18 '13
*   Availability: https://stackoverflow.com/questions/15476717/how-to-query-a-server-and-get-the-mx-a-ns-records
*	Part: How to get the MX, A, NS records
***************************************************************************************/
void parse_answer(u_char* ans, int len, int type, std::string *ipv4, std::string *ipv6)
{
	ns_msg handle;
	ns_rr rr;
	char buf[4096];
	ns_initparse(ans,len,&handle);
	int msg_len;
	/* Check IF there is any message */
	if((msg_len = ns_msg_count(handle, ns_s_an)) == 0)
		return;
	/* Check if response is parse-able */
	if((ns_parserr(&handle, ns_s_an, 0, &rr)>=0))
	{
		std::string s;
		/* Different type of response requires different type of formatting */
		switch(type)
		{
			case ns_t_a:
			{
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					/* Convert response to human readable format and store it in buf variable */
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					/* Format response */
					if(s.rfind("IN A")!= std::string::npos)
					{
						std::string key = "IN A";
						std::size_t found_key = s.rfind(key);
						s.erase(0, found_key+4);
						std::size_t found_num = find_digit(s);
						s.erase(0, found_num);
						std::cout << "A:\t" << s << std::endl;
						*ipv4 = s;
					}
				}
				break;
			}
			case ns_t_aaaa:
			{
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					if(s.rfind("IN AAAA")!= std::string::npos)
					{
						std::string key = "IN AAAA";
						std::size_t found_key = s.rfind(key);
						s.erase(0, found_key+7);
						std::size_t found_num = find_digit(s);
						std::size_t found_char = find_char(s);
						if(found_num < found_char)
							s.erase(0, found_num);
						else
							s.erase(0, found_char);
						std::cout << "AAAA:\t" << s << std::endl;
						*ipv6 = s;
					}
				}
				break;
			}
			case ns_t_soa:
			{	
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					if(s.rfind("IN SOA")!= std::string::npos)
					{
						std::string key = "IN SOA";
						std::size_t found_key = s.rfind(key);
						s.erase(0, found_key+6);
						std::size_t found_char = find_char(s);
						s.erase(0, found_char);
						std::size_t found_space = s.find(" ");
						std::string t_soa = s.substr(0, found_space-1);
						std::cout << "SOA:\t" << t_soa << std::endl;
						s.erase(0, found_space);
						s = s.substr(1, s.find("(")-1);
						s[s.find(".")] = '@';
						std::cout << "ADMIN:\t" << s.substr(0, s.find_last_of(".")) << std::endl; 
					}
				}
				break;
			}
			case ns_t_mx:
			{
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					if(s.rfind("IN MX")!= std::string::npos)
						std::cout << "MX:\t" << Format_DNS_response(s, "IN MX", 5) << std::endl;
				}
				break;
			}
			case ns_t_cname:
			{
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					if(s.rfind("IN CNAME")!= std::string::npos)
						std::cout << "CNAME:\t" << Format_DNS_response(s, "IN CNAME", 8) << std::endl;
				}
				break;
			}
			case ns_t_ns:
			{
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					if(s.rfind("IN NS")!= std::string::npos)
						std::cout << "NS:\t" << Format_DNS_response(s, "IN NS", 5) << std::endl;
				}
				break;
			}
			case ns_t_ptr:
			{
				/* Need to use a loop, to get all possible answers */
				for(int i = 0; i < msg_len; i++)
				{
					ns_parserr(&handle, ns_s_an, i, &rr);
					ns_sprintrr(&handle, &rr, NULL, NULL, buf, sizeof(buf));
					s = buf;
					if(s.rfind("IN PTR")!= std::string::npos)
						std::cout << "PTR:\t" << Format_DNS_response(s, "IN PTR", 6) << std::endl;
				}
				break;
			}
			default:
				break;
		}
	}
}

/***************************************************************************************
*   Function: reverse_ip4_lookup
*   ----------------------------
*	Function prepares IPv4 address for reverse lookup by reversing IPv4 address and concatenate it with ".in-addr.arpa." 
*   Param1:	IPv4 address
*   Returns: Reversed IPv4 address
****************************************************************************************
*   Title: convert IP for reverse ip lookup
*   Author: MOHAMED
*   Date: May 4 '13
*   Availability: https://stackoverflow.com/questions/16373248/convert-ip-for-reverse-ip-lookup
***************************************************************************************/
char* reverse_ip4_lookup(char* ip4)
{
	int a, b, c, d;
	sscanf(ip4, "%d.%d.%d.%d", &a, &b, &c, &d);
	char* reversed = (char*) malloc(sizeof(char)*(strlen(ip4)+14));
	if(!reversed)
		return NULL;
	sprintf(reversed, "%d.%d.%d.%d.in-addr.arpa.", d, c, b, a);
	return reversed;
}

/***************************************************************************************
*   Function: invertirv6
*   ----------------------------
*	Function prepares IPv6 address for reverse lookup by reversing IPv6 address and concatenate it with ".ip6.arpa." 
*   Param1:	IPv6 address
*   Returns: Reversed IPv6 address
****************************************************************************************
*   Title: make a reverse ipv6 for DNSBL in c++
*   Author: Michael Hampton
*   Date: Mar 14 '17
*   Availability: https://stackoverflow.com/questions/42774904/make-a-reverse-ipv6-for-dnsbl-in-c
***************************************************************************************/
char* invertirv6(char* ip6) 
{
    struct in6_addr addr;
    inet_pton(AF_INET6,ip6,&addr);
    char str2[48];
    sprintf(str2,"%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x",
    addr.s6_addr[0], addr.s6_addr[1],
    addr.s6_addr[2], addr.s6_addr[3],
    addr.s6_addr[4], addr.s6_addr[5],
    addr.s6_addr[6], addr.s6_addr[7],
    addr.s6_addr[8], addr.s6_addr[9],
    addr.s6_addr[10], addr.s6_addr[11],
    addr.s6_addr[12], addr.s6_addr[13],
    addr.s6_addr[14], addr.s6_addr[15]);
    /* pripravenie ip6 arpa */
    std::string str = str2;
   	str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
   	std::string output = "";
   	reverse(str.begin(), str.end());
   	for(unsigned int i = 0; i < str.length(); i++)
   	{
   		output += str[i];
   		output += ".";
   	}
   	output += "ip6.arpa.";
    char* str_ret = (char*) malloc(sizeof(char)*(output.length()+1));
    if(!str_ret)
    	return NULL;
    strcpy(str_ret, (char*) output.c_str());
    return str_ret;
}

/***************************************************************************************
*   Function: reverse_ptr
*   ----------------------------
*	Function retrieve PTR record from DNS server
*   Param1:	hostname which will be resolved
*   Returns: void
****************************************************************************************
*   Title: linker error for ns_initparse
*   Author: Bruce
*   Date: Nov 6 '09
*   Availability: https://stackoverflow.com/questions/1688895/linker-error-for-ns-initparse
***************************************************************************************/
void reverse_ptr(char* hostname)
{
	u_char nsbuf[4096];
    int l;
    char *ht = NULL;
    std::string ipver4, ipver6;
    if(isValidIpAddress(hostname))
    {
		ht = reverse_ip4_lookup(hostname);
		if(!ht)
		{
			printf("DNS:\tIPv4 reverse malloc() failed\n");
			return;
		}
    }
	else if(isValidIp6Address(hostname))
	{
		ht = invertirv6(hostname);
		if(!ht)
		{
			printf("DNS:\tIPv6 reverse malloc() failed\n");
			return;
		}
	}
	l = res_query (ht, ns_c_in, ns_t_ptr, nsbuf, sizeof (nsbuf));
    if (l < 0) 
    {
        perror (ht);
        parse_answer(nsbuf, l, ns_t_ptr, &ipver4, &ipver6);
    } 
    else 
    {
		#ifdef USE_PQUERY
           	res_pquery (&_res, nsbuf, l, stdout);
		#else
           	parse_answer(nsbuf, l, ns_t_ptr, &ipver4, &ipver6);
		#endif
	}
	if(ht)
		free(ht);
	return;
}

/***************************************************************************************
*   Function: parse_answer
*   ----------------------------
*	Function parses response from DNS server and print it to STDOUT
*   Param1:	hostname which will be resolved
*	Param2: Pointer used to return IPv4 for later  use
*	Param3: Pointer used to return IPv6 for later  use
*   Returns: void
****************************************************************************************
*   Title: How to query a server and get the MX, A, NS records
*   Author: Dima00782
*   Date: Mar 18 '13
*   Availability: https://stackoverflow.com/questions/15476717/how-to-query-a-server-and-get-the-mx-a-ns-records
*	Part: How to query a Server
***************************************************************************************/
void print_DNS_stats(char* hostname, std::string* ip4, std::string* ip6)
{
	int len;
	u_char ans[4096];
	res_init();
	std::string ipver4, ipver6;
	std::cout << "=== DNS ===" << std::endl;
	/* IF hostname contains IP address, it will do PTR request */
	if(isValidIpAddress(hostname) || isValidIp6Address(hostname))
	{
		reverse_ptr(hostname);
		return;
	}
	// A 	 RECORD
	len = res_query(hostname, ns_c_in, ns_t_a, ans, sizeof(ans));  //mozno treba osetrit error code	
	parse_answer(ans, len, ns_t_a, &ipver4, &ipver6);
	// AAAA	 RECORD
	len = res_query(hostname, ns_c_in, ns_t_aaaa, ans, sizeof(ans));  //mozno treba osetrit error code
	parse_answer(ans, len, ns_t_aaaa, &ipver4, &ipver6);
	// MX	 RECORD
	len = res_query(hostname, ns_c_in, ns_t_mx, ans, sizeof(ans));  //mozno treba osetrit error code
	parse_answer(ans, len, ns_t_mx, &ipver4, &ipver6);
	// SOA	 RECORD
	len = res_query(hostname, ns_c_in, ns_t_soa, ans, sizeof(ans));  //mozno treba osetrit error code
	parse_answer(ans, len, ns_t_soa ,&ipver4, &ipver6);
	// NS	 RECORD
	len = res_query(hostname, ns_c_in, ns_t_ns, ans, sizeof(ans));  //mozno treba osetrit error code
	parse_answer(ans, len, ns_t_ns, &ipver4, &ipver6);
	// CNAME RECORD
	len = res_query(hostname, ns_c_in, ns_t_cname, ans, sizeof(ans));  //mozno treba osetrit error code
	parse_answer(ans, len, ns_t_cname, &ipver4, &ipver6);
	// PTR	 RECORD
	len = res_query(hostname, ns_c_in, ns_t_ptr, ans, sizeof(ans));  //mozno treba osetrit error code
	parse_answer(ans, len, ns_t_ptr, &ipver4, &ipver6);
	*ip4 = ipver4;
	*ip6 = ipver6;
}
