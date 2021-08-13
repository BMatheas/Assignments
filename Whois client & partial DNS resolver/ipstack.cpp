#include "ipstack.hpp"

#define MAX_REQUEST_LEN 1024


/***************************************************************************************
*   Function: ipstack_records
*   ----------------------------
*   Param1: IP address
*   Returns: response from website
***************************************************************************************
*   Title: POSIX 7 minimal runnable example
*   Author: Ciro Santilli 新疆改造中心法轮功六四事件
*   Date: Feb 28 '16
*   Availability: https://stackoverflow.com/questions/11208299/how-to-make-an-http-get-request-in-c-without-libcurl
*
***************************************************************************************/
char* ipstack_records(char* ip)
{
    char* buffer = NULL;
    buffer = (char*)malloc(sizeof(char)*4096);
    if(!buffer)
    {
        printf("\nIPSTACK:\t Malloc() failed\n");
        return NULL;
    }
    char request[MAX_REQUEST_LEN];
    char request_template[] = "GET /%s?access_key=aa9d0a07c7a2489664e19fb674b9003f HTTP/1.1\r\nHost: api.ipstack.com\r\n\r\n";
    struct protoent *protoent;
    char hostname[] = "api.ipstack.com";
    in_addr_t in_addr;
    int request_len;
    int socket_file_descriptor;
    struct hostent *hostent;
    struct sockaddr_in sockaddr_in;
    unsigned short server_port = 80;

    /* Build request message */
    request_len = snprintf(request, MAX_REQUEST_LEN, request_template, ip);
    if (request_len >= MAX_REQUEST_LEN) 
    {
        if(!buffer)
            free(buffer);
        return NULL;
    }

    /* Build the socket. */
    protoent = getprotobyname("tcp");
    if (protoent == NULL) 
    {
        printf("IPSTACK:\tgetprotobyname() failed\n");
        if(!buffer)
            free(buffer);
        return NULL;
    }
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
    if (socket_file_descriptor == -1) 
    {
        printf("IPSTACK:\tSocket() failed\n");
        if(!buffer)
            free(buffer);
        return NULL;
    }

    /* Build the address. */
    hostent = gethostbyname(hostname);
    if (hostent == NULL) 
    {
        printf("IPSTACK:\tgehostbyname() failed\n");
        if(!buffer)
            free(buffer);
       return NULL;
    }
    in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));
    if (in_addr == (in_addr_t)-1) 
    {
        printf("IPSTACK:\tinet_addr() failed\n");
        if(!buffer)
            free(buffer);
        return NULL;
    }
    sockaddr_in.sin_addr.s_addr = in_addr;
    sockaddr_in.sin_family = AF_INET;
    sockaddr_in.sin_port = htons(server_port);

    /* Actually connect. */
    if(connect(socket_file_descriptor, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) 
    {
        printf("IPSTACK:\tConnect() failed\n");
        if(!buffer)
            free(buffer);
        return NULL;
    }
    /* Send HTTP request. */
    if(send(socket_file_descriptor, request, strlen(request), 0) < 0)
    {
        printf("IPSTACK:\tSend() failed\n");
        if(!buffer)
            free(buffer);
        return NULL;
    } 
    /* Read HTTP response */
    if(read(socket_file_descriptor, buffer, 4096) < 0)
    {
        printf("IPSTACK:\tRead() failed\n");
        if(!buffer)
            free(buffer);
        return NULL;
    }
    close(socket_file_descriptor);
    return buffer;
}

/***************************************************************************************
 * Function: print_ip_stack
 * ----------------------------
 *   Parse and print to STDOUT answer retrieved from function ipstack_records.
 *   param1: IP address 
 *   returns: void
***************************************************************************************/
void print_ip_stack(char* ip)
{
    /* Retrieve response */
	char* response_ip_stack = NULL;
    response_ip_stack = ipstack_records(ip);
    if(!response_ip_stack)
        return;
    /* Parsing an response using strings and <algorithm> functions */
    std::string s(response_ip_stack);
    s.erase(0, s.find("{")+1);
    std::size_t pos = 0;
    std::string delimiter = ",";
    std::string token = "";
    std::cout << "\n=== IP_STACK ===" << std::endl;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
    	token = s.substr(0, pos);
    	token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());
    	if(token.rfind("type:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("continent_code:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("continent_name:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("region_code:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("region_name:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("city:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("zip:") != std::string::npos)
    	{
    		if(token.substr(token.find(":")).rfind("null"))
    		{
    			s.erase(0, pos + delimiter.length());
    			continue;
    		}
    		token = token.substr(0, token.find(":")) + ":\t\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("latitude:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("longitude:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("capital:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
    	else if(token.rfind("ip:") != std::string::npos)
    	{
    		token = token.substr(0, token.find(":")) + ":\t\t" + token.substr(token.find(":")+1, token.length());
    		std::cout << token << std::endl;
    	}
	   	s.erase(0, pos + delimiter.length());
	}
    try{
        if(response_ip_stack)
            free(response_ip_stack);    
    }
    catch(...)
    {
        printf("IP STACK:\t Nastala chyba pri uvolnovani\n");
    }
    return;
}