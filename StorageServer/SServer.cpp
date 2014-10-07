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

#include "SServer.h"




void SServer::startListening() {
	std::cout << ":::: Storage Server ::::" << std::endl;
	std::cout << "Listening on port " << ss_port << "..." << std::endl;
	
	//Start the TCP connection.
	this->initTCP();
		
}


void SServer::req_command(std::string fn) {
	//TODO: Validar o pedido REQ, verificar se EOF (sem ficheiros no servidor)
	std::cout << "TCP: REQ requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	/*responder com REP status size data, status = ok se enviou o ficheiro ou nok 
	 * caso contrario, size = tamanho dos dados e data = aos proprios dados ou ERR*/

}

void SServer::ups_command(){
	std::cout << "TCP: UPS requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	/*responder com AWS status, status = ok se gravou o ficheiro ou nok 
	 * caso contrario ou ERR*/	
	}

void SServer::processTCP() {
	
	char tcp_buffer[128];
	bzero(tcp_buffer, 128); 
	// Processamento dos comandos TCP
		
		nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,4,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
		if(nread_tcp==-1) {
			std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
			return;
		}
		std::cout << "Comando : " << tcp_buffer << std::endl;
		if(strcmp(tcp_buffer, "REQ ") == 0){
			nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,3,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
			std::cout << "Buffer com: " << tcp_buffer << std::endl;
			this->req_command(tcp_buffer);
		   }else if(strcmp(tcp_buffer, "UPS ") == 0){
				nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,30,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
				std::cout << "Buffer com: " << tcp_buffer << std::endl;
				//this->ups_command();
				}
		


	ret_tcp=send(accept_fd_tcp,tcp_buffer,nread_tcp,0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response sent." << std::endl;

}

void SServer::initTCP() {

	std::cout << "TCP: Initialization..." << std::endl;

	if((fd_tcp=socket(AF_INET,SOCK_STREAM,0))==-1)
		std::cout << "TCP: Error initializing the socket";
		
	memset((void*)&addr_tcp,(int)'\0',sizeof(addr_tcp));
	addr_tcp.sin_family=AF_INET;
	addr_tcp.sin_addr.s_addr=htonl(INADDR_ANY);
	addr_tcp.sin_port=htons(atoi(ss_port));
	
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





