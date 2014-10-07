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

#define PORT "58021"

class CServer {
	
	public:
    
    //definição das variaveis so para testar agora se esta a ser feita ligação
    pid_t pid,pid_udp;

    char *cs_port;
    char buffer[128];

    // Ligação UDP
    int fd_udp, accept_fd_udp, ret_udp, nread_udp;

    socklen_t addrlen_udp;
    struct sockaddr_in addr_udp;
    
	// Ligação TCP
    int fd_tcp, accept_fd_tcp, ret_tcp, nread_tcp;

    socklen_t addrlen_tcp;
    struct sockaddr_in addr_tcp;
    
	
	public:
	
	CServer(char *port) : cs_port(port) {};
   
    void startListening();

    void initUDP();
    void processUDP();

    void initTCP();
    void processTCP();

    void list_command();

	void testConnection();
	
	private:
	




};


#endif

