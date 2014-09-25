//
//  Client.h
//  Projecto RC
//
//  Created by Henrique Mouta on 25/09/14.
//  Copyright (c) 2014 Grupo 21. All rights reserved.
//

#ifndef Projecto_RC_Client_h
#define Projecto_RC_Client_h

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <arpa/inet.h>
#include <sstream>
#include <algorithm>
#define PORT 58021


class Client {
    
public:
    char *host_name;
    int cs_port;
    int fd_udp, fd_tcp;
    int connect_id, recieve_id;
    struct sockaddr_in addr;
    struct in_addr *a;
    socklen_t addrlen;
    
    char buffer[600];
    std::string input;
    
private:
    //Storage Server information
    std::string ss_ip;
    int ss_port;
    
public:
    Client( char* hostname, int port ) : host_name(hostname), cs_port(port) {};
    void list();
    std::vector<std::string> split(const std::string &s, char delim);
    std::vector<std::string> parse_response(char* buffer);
    void connect();
    
private:
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    std::vector<std::string> parse_files( std::vector<std::string> response );

};




#endif
