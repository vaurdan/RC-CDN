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

#define PORT "59000"

class SServer {
	
	public:
	
	pid_t pid;
	
	char *ss_port;
    char buffer[128];
    
	// Ligação TCP
    int fd_tcp, accept_fd_tcp, ret_tcp, nread_tcp;

    socklen_t addrlen_tcp;
    struct sockaddr_in addr_tcp;
    
	
	
	public:
	
	SServer(char *port) : ss_port(port) {};
    
    
	void startListening();
	void list_command();
	void processTCP();
	void initTCP();
	void req_command(std::string fn);
	void ups_command();

	void strip(char *s);

	private:

	template <typename T>
    std::string to_string(T value);


};


#endif
