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
#define PORT "58021"


class Client {
    
public:
       
    //Ligações UDP com servidor central
    int fd_udp_cs;                   
    struct sockaddr_in addr_udp_cs;
    struct in_addr *a_udp_cs;
    socklen_t addrlen_udp_cs;
    //Ligações TCP com servidor central
    int fd_tcp_cs;
    struct sockaddr_in addr_tcp_cs;
    struct in_addr *a_tcp_cs;
    socklen_t addrlen_tcp_cs;
    //Ligações TCP com server storage
    int fd_tcp_ss;
    struct sockaddr_in addr_tcp_ss;
    struct in_addr *a_tcp_ss;
    socklen_t addrlen_tcp_ss;
    
    
    char *host_name;
    char *cs_port;
    int connect_id, recieve_id;
    char buffer[600];
    std::string input;
    int nbytes, nleft,nwritten, nread;
    char *ptr;
    
private:
    //Storage Server information
    char *ss_ip;
    char *ss_port;
    int loadbar_acc;
    
public:
    Client( char* hostname, char *port ) : host_name(hostname), cs_port(port), loadbar_acc(0) {};
    void list();
    void retrieve(std::string file_name);
    void upload(std::string up_file_name);
    int file_size(int fd);
    std::vector<std::string> split(const std::string &s, char delim);
    std::vector<std::string> parse_response(char* buffer);
    bool connectionCS(int type);
    bool connectionSS();
    inline void loadbar(unsigned int x, unsigned int n, unsigned int w = 50);
    
    //APAGAR DEPOIS
    void testConnection();
    void testREQ(std::string test_command_size);

    void close_all();
    
private:
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    std::vector<std::string> parse_files( std::vector<std::string> response );
    void strip(char *s);


};




#endif
