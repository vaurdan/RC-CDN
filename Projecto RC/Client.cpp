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
#include <stdlib.h>
#include <vector>
#include <arpa/inet.h>
#include <sstream>
#include <algorithm>
#include "Client.h"

/*
 * Definição das funções de split
 */
std::vector<std::string> &Client::split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> Client::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


/** AWL
 192.168.128.2 59100 10
 f1.txt f222222.txt f3.jpg A380.jpg Boeing_747.jpg aaaaaa.txt bbbbbb.txt cccccc.txt dddddd.txt eeeeeee.txt
 */
std::vector<std::string> Client::parse_files( std::vector<std::string> response ) {
    //Inverter a lista
    std::reverse(response.begin(),response.end());
    
    std::string command = response.back();
    response.pop_back();

    std::string ip = response.back();
    response.pop_back();
    this->ss_ip = ip;
    
    std::string port = response.back();
    response.pop_back();
    this->ss_port = atoi(port.c_str());
    
    unsigned int num_files = atoi(response.back().c_str());
    response.pop_back();
    
    if(num_files != response.size() || command != "AWL" ) {
        return std::vector<std::string>();
    }
    
    std::reverse(response.begin(),response.end());
    
    return response;
    
}

std::vector<std::string> Client::parse_response(char* buffer) {
    std::string content(buffer);
    std::vector<std::string> elements = split(content,' ');
    
    return parse_files(elements);
}


void Client::list() {
    
    // Enviar a mensagem para o servidor
    connect_id=sendto(fd_udp_cs, "LST\n", 4, 0, (struct sockaddr*)&addr_udp_cs, sizeof(addr_udp_cs));
    if(connect_id==-1)
        exit(1);
    
    addrlen_udp_cs=sizeof(addr_udp_cs);
    recieve_id=recvfrom(fd_udp_cs,buffer,600,0,(struct sockaddr*)&addr_udp_cs,&addrlen_udp_cs);
    if(recieve_id==-1)
        exit(1);
    
    std::vector<std::string> resposta = parse_response(buffer);
    if(resposta.empty()) {
        std::cout << "Erro de protocolo" << std::endl;
        return;
    }
    
    int number = 1;
    std::cout << "NUMERO\tNOME DO FICHEIRO" << std::endl;
    for (std::vector<std::string>::iterator it = resposta.begin(); it != resposta.end(); ++it) {
        std::cout << number++ << "\t" << *it << std::endl;
    }
    
    //Fechar as ligações
    close(recieve_id);
    close(connect_id);
    
    
}

void Client::connect() {
    
    // UDP connection to the Central Server
    fd_udp_cs=socket(AF_INET, SOCK_DGRAM, 0);//SOCKET DO UPD
    if(fd_udp_cs==-1)
        exit(1);
    
    memset((void*)&addr_udp_cs,(int)'\0',sizeof(&addr_udp_cs));
    addr_udp_cs.sin_family=AF_INET;
    a_udp_cs=(struct in_addr*)gethostbyname(host_name)->h_addr_list[0];
    addr_udp_cs.sin_addr.s_addr = a_udp_cs->s_addr;
    addr_udp_cs.sin_port=htons(cs_port);

    // TCP connection to the Central Server

    fd_tcp_cs=socket(AF_INET, SOCK_STREAM,0);//SOCKET do TCP
    if(fd_tcp_cs==-1)
        exit(1);
    
    memset((void*)&addr_tcp_cs,(int)'\0',sizeof(&addr_tcp_cs));
    addr_tcp_cs.sin_family=AF_INET;
    a_tcp_cs=(struct in_addr*)gethostbyname(host_name)->h_addr_list[0];
    addr_tcp_cs.sin_addr.s_addr = a_tcp_cs->s_addr;
    addr_tcp_cs.sin_port=htons(cs_port);
}
