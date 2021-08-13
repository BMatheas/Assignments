#include "whois.hpp"
#include <ctype.h>


/***************************************************************************************
*   Function: HostToIp
*   ----------------------------
*	Translate hostname to IP
*   Param1: Hostname
*   Returns: IP address on success
****************************************************************************************
*    Title: GET IP FROM HOSTNAME
*    Author: Neel Patel
*    Date: 22. Februar 2012
*    Availability: https://stackoverflow.com/questions/9400756/ip-address-from-host-name-in-windows-socket-programming
***************************************************************************************/
std::string HostToIp(const std::string& host) {
    hostent* hostname = gethostbyname(host.c_str());
    if(hostname)
        return std::string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
    return {};
}

/***************************************************************************************
*   Function: whois_query
*   ----------------------------
*	Function queries Whois server 
*   Param1: Whois server
*	Param2: message
*	Param3: Pointer used to return response for later use
*   Returns: 0 on success
****************************************************************************************
*   Title: C code to perform IP whois
*   Author: Silver Moon
*   Date: September 21, 2012
*   Availability: https://www.binarytides.com/c-code-to-perform-ip-whois/
***************************************************************************************/
int whois_query(char *server , char *query , char **response)
{
	char message[100] , buffer[1500];
	int sock , read_size , total_size = 0;
	struct sockaddr_in dest;
	struct sockaddr_in6 dest6;
	
	if(strstr(server, ":") != NULL)
	{
		sock = socket(AF_INET6, SOCK_STREAM , IPPROTO_TCP);
    	//Prepare connection structures :)
    	memset(&dest6 , 0 , sizeof(dest6));
    	dest6.sin6_family = AF_INET6;
    	dest6.sin6_port = htons(43) ;
		inet_pton(AF_INET6, server, &(dest6.sin6_addr));
		//Now connect to remote server
		if(connect(sock , (const struct sockaddr*) &dest6 , sizeof(dest6))  == -1)
		{
			printf("Connect failed\n");
			return 10;
		}
	}
	else
	{
		sock = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP);
     
    	//Prepare connection structures :)
    	memset(&dest, 0 , sizeof(dest));
    	dest.sin_family = AF_INET;

		//printf("%s\n", server);    
		dest.sin_addr.s_addr = inet_addr(server);
		dest.sin_port = htons(43) ;

		//Now connect to remote server
		if(connect(sock , (const struct sockaddr*) &dest , sizeof(dest)) > 0)
		{
			printf("Connect failed\n");
			return 20;
		}
	}
	sprintf(message , "%s\r\n" , query);
	if(send(sock , message , strlen(message) , 0) < 0)
	{
		//std::cout << strerror(errno) << std::endl;
		printf("Send failed\n");
		return 30;
	}
	
	//Now receive the response
	while((read_size = recv(sock , buffer , sizeof(buffer) , 0)))
	{
		*response = (char*) realloc(*response , read_size + total_size);
		if(*response == NULL)
		{
			printf("realloc failed\n");
			return 40;
		}
		memcpy(*response + total_size , buffer , read_size);
		total_size += read_size;
	}
	fflush(stdout);
	
	*response = (char*) realloc(*response , total_size + 1);
	*(*response + total_size) = '\0';
	close(sock);
	return 0;
}

/***************************************************************************************
*   Function: print_response
*   ----------------------------
*	Function prints answer
*   Param1: response from whois server
*	Param2: IP/Hostname which we want info about from whois server(used for error stdout)
*   Returns: int 
***************************************************************************************/
int print_answer(char* response, char* ip)
{
	std::cout << "\n=== WHOIS ===\n";
	std::stringstream ss(response);
  	std::string to;
  	std::string test = response;
  	/* Checks if whois server doesn't contain information about IP/Hostname */
  	if(test.rfind("No match found") != std::string::npos || 
  	   test.rfind("ERROR") != std::string::npos 		 || 
  	   test.rfind("No Data") != std::string::npos 		 ||
  	   test.rfind("No Object") != std::string::npos 	 ||
  	   test.rfind("NOT FOUND") != std::string::npos 	 ||
  	   test.rfind("not supported") != std::string::npos 	 ||
  	   test.rfind("Wrong") != std::string::npos	  	||
  	   test.rfind("Invalid") != std::string::npos		||
	   test.rfind("not found") != std::string::npos)
  	{
  		std::cout << "\nNenasiel zaznam pre: " << ip << std::endl;
  		free(response);
  		return 50;
  	}
  	int flag = 0, is = 0;
  	std::string tmp;
  	/* Format repsonse and print to stdout */
    while(std::getline(ss,to, '\n'))
    {
    	is = 0;
    	if(to[0] == '%')
    		continue;
    	if(to[0] == '>')
    		break;
    	else if(isalpha(to[0]) || isdigit(to[0]))
    	{
    		std::size_t found = to.rfind(":");
    		for(std::size_t i = found+1; i < to.length(); i++)
    			if(!isspace(to[i]))
    				is = 1;
    		if(is)
    		{
    			std::cout << to << std::endl;
    		}
    		flag = 0;
    	}
    	else if(!flag)
    	{
    		std::cout << to << std::endl;
    		flag = 1;
    	}
    }
    if(response)
  		free(response);
  	return 0;
}

/***************************************************************************************
*   Function: get_whois
*   ----------------------------
*	Function queries Whois server 
*   Param1: IP/Hostname which we want info about from whois server
*	Param2: Whois server 
*   Returns: int 
****************************************************************************************
*   Title: C code to perform IP whois
*   Author: Silver Moon
*   Date: September 21, 2012
*   Availability: https://www.binarytides.com/c-code-to-perform-ip-whois/
***************************************************************************************/
int get_whois(char *ip , char *server)
{
	char *response = NULL;
	
	/* Call whois_query function and get response */
	if(whois_query(server, ip , &response))
	{
		printf("WHOIS:\t whois_query() failed\n");
		return 1;
	}
	if(response == NULL)
	{
		printf("WHOIS:\t RESPONSE == NULL\n");
		return 2;
	}
	if(print_answer(response, ip) == 50)
	{
		return 3;
	}
	return 0;
}
