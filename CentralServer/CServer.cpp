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
#include <sys/wait.h>

#include "CServer.h"

void CServer::startListening() {
	std::cout << ":::: Central Server ::::" << std::endl;
	std::cout << "Listening on port " << cs_port << "..." << std::endl;
	//Start the UDP connection
	pid_udp = fork();

	if( pid_udp == -1 ) {
		std::cerr << "Error initializing UDP child process.";
		exit(1);
	} else if(pid_udp == 0) {
		this->initUDP();
		_exit(0);
	} else {
		//Start the TCP connection.
		this->initTCP();
		int status;
		// Apenas terminar o programa quando já não houver nenhum filho (UDP a funcionar);
		(void)waitpid(pid, &status, 0);
	}
}


void CServer::processTCP() {
	char tcp_buffer[128];
	bzero(tcp_buffer, 128);
	nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,128,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
	if(nread_tcp==-1) {
		std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
		return;
	}

	if(strcmp(tcp_buffer, "test\n") == 0){
		std::cout << "TCP: Test command recieved." << std::endl;
	}

	ret_tcp=send(accept_fd_tcp,tcp_buffer,nread_tcp,0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response sent." << std::endl;
}

void CServer::processUDP() {
	std::cout << "UDP: Waiting for connections..." << std::endl;
	addrlen_udp=sizeof(addr_udp);
	bzero(buffer, 128);
	nread_udp=recvfrom(fd_udp,buffer,128,0,(struct sockaddr*)&addr_udp,&addrlen_udp);
	if(nread_udp==-1) {
		std::cout << "UDP: recv error: " << strerror(errno) << std::endl;
		return;
	}

	if(strcmp(buffer, "test\n") == 0){
		std::cout << "UDP: Test command recieved." << std::endl;
	}

	ret_udp=sendto(fd_udp,buffer,nread_udp,0,(struct sockaddr*)&addr_udp, addrlen_udp);
	if(ret_udp==-1) {
		std::cout << "UDP: error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "UDP: Response sent." << std::endl;
}


void CServer::initUDP() { 
	std::cout << "UDP: Initialization..." << std::endl;
	if((fd_udp=socket(AF_INET,SOCK_DGRAM,0))==-1) {
		std::cout << "UDP: Error initializing the socket";
		return;
	}
	
	memset((void*)&addr_udp,(int)'\0',sizeof(addr_udp));
	addr_udp.sin_family=AF_INET;
	addr_udp.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_udp.sin_port=htons(atoi(cs_port));

	ret_udp=bind(fd_udp,(struct sockaddr*)&addr_udp,sizeof(addr_udp));
	if(ret_udp == -1) {
		std::cout << "UDP: Binding error: " << strerror(errno) << std::endl;
		return;
	}

	while(1){
		
		this->processUDP();
		
		
		}
		
	std::cout << "vou desligar" << std::endl;
		
	 //close(fd);
	 //exit(0);
}

void CServer::initTCP() {

	std::cout << "TCP: Initialization..." << std::endl;

	if((fd_tcp=socket(AF_INET,SOCK_STREAM,0))==-1)
		std::cout << "TCP: Error initializing the socket";
		
	memset((void*)&addr_tcp,(int)'\0',sizeof(addr_tcp));
	addr_tcp.sin_family=AF_INET;
	addr_tcp.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_tcp.sin_port=htons(atoi(cs_port));
	
	ret_tcp=bind(fd_tcp,(struct sockaddr*)&addr_tcp,sizeof(addr_tcp));
	if(ret_tcp == -1) {
		std::cout << "TCP: Binding error: " << strerror(errno) << std::endl;
		return;
	}

	if( listen(fd_tcp,2) == -1)
		return;


	while(true) {
		//Waiting for new connections.
		std::cout << "TCP: Waiting for connections..." << std::endl;
		addrlen_tcp = sizeof(addr_tcp);

		do {
			accept_fd_tcp = accept(fd_tcp,(struct sockaddr*)&addr_tcp, &addrlen_tcp);
		} while ( accept_fd_tcp == -1 && errno == EINTR );

		if(accept_fd_tcp == -1) {
			std::cout << "TCP: Accept erro: " << strerror(errno) << std::endl;
			return;
		}

		if( (pid == fork()) == -1 ) {
			exit(1);
		} else if( pid == 0 ) {
			this->processTCP();
		}
		//Parent process

		do {
			ret_tcp = close(accept_fd_tcp);
		} while (ret_tcp == -1 && errno==EINTR);

		if(ret_tcp==-1)return;
	}
}

