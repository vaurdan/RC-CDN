#ifndef Projecto_RC_CServer_h
#define Projecto_RC_CServer_h

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

#define PORT 58021

class CServer {
	
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
    
    char *cs_port;
    int fd, ret, nread;
    socklen_t addrlen;
    struct sockaddr_in addr;
    char buffer[128];
    
	
	
	public:
	
	CServer(char *port) : cs_port(port) {};
    void start();
    
	bool connectionCS(int type);
	
	void testConnection();
	
	private:
	




};


#endif

