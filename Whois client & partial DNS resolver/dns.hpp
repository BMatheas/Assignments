#ifndef DNS_HPP
#define DNS_HPP

#include <stdio.h>
#include <resolv.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/nameser.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <string>

#define N 4096

void print_DNS_stats(char* hostname, std::string* ipver4, std::string* ipver6);
int isValidIpAddress(char *ipAddress);
int isValidIp6Address(char *ipAddress);

#endif