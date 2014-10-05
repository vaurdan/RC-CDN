#ifndef Projecto_RC_SServer_h
#define Projecto_RC_SServer_h

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
#include <cstring>
#include <fstream>
#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <sys/stat.h>
#include <iostream>

#define PORT 59000

class SServer {
	
	public:
	
	/*//Ligações UDP com servidor central
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
    char *ptr;*/
    
    //definição das variaveis so para testar agora se esta a ser feita ligação
    
    
    char *ss_port;
    int fd, newfd;
    socklen_t addrlen;
    struct sockaddr_in addr;
    int n, nw;
    char *ptr, buffer[128];
    
	
	
	public:
	
	SServer(char *port) : ss_port(port) {};
    void start();
    
	void TCP_Connection_test();	
	
	private:
	




};


#endif
