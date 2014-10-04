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

#include "CServer.h"


//Ao ser chamado o CServer são criadas duas ligações, uma UPD outra TCP
/*bool CServer::connectionCS(int type) {
    
    if(type == 0) {
        // UDP connection to the Central Server
        fd_udp_cs=socket(AF_INET, SOCK_DGRAM, 0);//SOCKET DO UPD
        if(fd_udp_cs==-1)
            return false;
        
        memset((void*)&addr_udp_cs,(int)'\0',sizeof(&addr_udp_cs));
        addr_udp_cs.sin_family=AF_INET;
        a_udp_cs=(struct in_addr*)gethostbyname(host_name)->h_addr_list[0];
        addr_udp_cs.sin_addr.s_addr = a_udp_cs->s_addr;
        addr_udp_cs.sin_port=htons(atoi(cs_port));
        return true;
    } else if( type == 1) {
        // TCP connection to the Central Server
        
		struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
		struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.

		memset(&host_info, 0, sizeof host_info);

		host_info.ai_family = AF_INET;     // IP version not specified. Can be both.
		host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

		int status = getaddrinfo(host_name, cs_port, &host_info, &host_info_list);

		fd_tcp_cs=socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);//SOCKET do TCP

		if(fd_tcp_cs==-1)
			return false;

			status = connect(fd_tcp_cs, host_info_list->ai_addr, host_info_list->ai_addrlen);

			if(status == -1 )
			return false;

			return true;
		}

		return true;
}*/

void CServer::testConnection(){

	std::cout << "testando a conecçao" << std::endl;
	if((fd=socket(AF_INET,SOCK_DGRAM,0))==-1)
		exit(1);
		
	memset((void*)&addr,(int)'\0',sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(59021);
	
	ret=bind(fd,(struct sockaddr*)&addr,sizeof(addr));
	if(ret == -1)
		exit(1);
	std::cout << "vou entrar no while" << std::endl;
	while(1){
		
		std::cout << "tou no while" << std::endl;
		addrlen=sizeof(addr);
		nread=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
		if(nread==-1)
			exit(1);
		if(buffer == "test"){
			std::cout << "teste" << std::endl;
			}
		std::cout << "fiz receive" << std::endl;
		ret=sendto(fd,buffer,nread,0,(struct sockaddr*)&addr, addrlen);
		if(ret==-1)
			exit(1);
		std::cout << "fiz send" << std::endl;
		
		
		}
		
		std::cout << "vou desligar" << std::endl;
		
	 //close(fd);
	 //exit(0);
	}
