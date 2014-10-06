#ifndef Projecto_RC_CSInstance_h
#define Projecto_RC_CSInstance_h

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

class CSInstance {
	
	public:
    
    //definição das variaveis so para testar agora se esta a ser feita ligação
    
    char *cs_port;
    char buffer[128];

    // Ligação
    int fd, ret, nread;
    socklen_t addrlen;
    struct sockaddr_in addr;
    

	int type; //UDP = 0, TCP = 1;
	
	public:
	
	CSInstance(int type, int fd, struct sockaddr_in addr) : type(type), addr(addr) {};
   
    void startListening();

	bool connectionCS(int type);
	
	void test();

	std::string getTypeName(); 
	
	private:
	




};


#endif

