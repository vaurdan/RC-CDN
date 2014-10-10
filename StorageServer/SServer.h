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
    
    
	bool startListening();
	bool testConnection();
	void list_command();
	void processTCP();
	void initTCP();
	void REQ_command(std::string fn);
	void UPS_command(std::string fn, std::string fn_size);
	std::vector<std::string> split(const std::string &s, char delim);

	void strip(char *s);

	private:

	template <typename T>
    std::string to_string(T value);
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);


};


#endif
