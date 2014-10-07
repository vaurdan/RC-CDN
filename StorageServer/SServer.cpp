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


template <typename T>
std::string SServer::to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

void SServer::startListening() {
	std::cout << ":::: Storage Server ::::" << std::endl;
	std::cout << ":::: Creating Storage Server Dir ::::" << std::endl;
	std::string ss_dir = "SS" + std::string(ss_port);
	std::cout << "Listening on port " << ss_port << "..." << std::endl;
	mkdir(ss_dir.c_str(),0755);
	//Start the TCP connection.
	this->initTCP();
		
}


void SServer::req_command(std::string fn) {
	//TODO: Validar o pedido REQ, verificar se EOF (sem ficheiros no servidor)

	FILE *req_file;

	int fsize;
	char *buffer_retrieve;
	char test[21];
	size_t result;
	std::string req_response;
	std::string file_size_result;
	std::ostringstream convert;


	std::cout << "TCP: REQ requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	//std::cout << "Ficheiro pedido: " << fn << std::endl;
	std::string req_dir = "SS" + std::string(ss_port) + "/" + std::string(fn);
	req_file =fopen(req_dir.c_str(), "r");
	//std::cout << "Directoria onde vou verificar: " << req_dir << std::endl;

	if(req_file == NULL){
		req_response = "REP " + std::string("nok");
		ret_tcp=send(accept_fd_tcp,req_response.c_str(),req_response.size(),0);
		if(ret_tcp==-1) {
			std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
			return;
		}
	std::cout << "TCP: Response sent." << std::endl;

		exit(EXIT_FAILURE);		
	}

	fseek(req_file, 0, SEEK_END);
	fsize = ftell (req_file);
	int new_file_size = fsize;
	std::cout << "tem size: " << fsize << std::endl;
	rewind (req_file);

	buffer_retrieve = (char*)malloc (sizeof(char)*fsize);
	if(buffer_retrieve == NULL){

		fputs("Memory error", stderr);
		exit(1);
	}
	while(fsize != 0){

		result= fread (buffer_retrieve,1,fsize,req_file);
		fsize--;

	}


	convert << new_file_size;
	file_size_result = convert.str();
	//std::cout << "file size com: " << file_size_result << std::endl;
	fclose(req_file);
	
	req_response = "REP " + std::string("ok ") + file_size_result + std::string(" ") + this->to_string(buffer_retrieve);
	//std::cout << "response tem: " << req_response << std::endl;

	ret_tcp=send(accept_fd_tcp,req_response.c_str(),req_response.size(),0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response sent." << std::endl;

 
}


void SServer::ups_command(){
	std::cout << "TCP: UPS requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;


}

void SServer::processTCP() {
	
	char tcp_buffer[128];
	bzero(tcp_buffer, 128);
	// Processamento dos comandos TCP
		
		nread_tcp=read(accept_fd_tcp,tcp_buffer,4);
		if(nread_tcp==-1) {
			std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
			return;
		}
		std::cout << "Comando : " << tcp_buffer << std::endl;
		if(strcmp(tcp_buffer, "REQ ") == 0){
			nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,30,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
			this->strip(tcp_buffer);
			std::cout << "Buffer com: " << tcp_buffer << std::endl;
			this->req_command(tcp_buffer);
	   } else if(strcmp(tcp_buffer, "UPS ") == 0){
			//nread_tcp=recv(accept_fd_tcp,tcp_buffer,30,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
			char letra;
			int contador = 0;
			std::string size_buffer = "";

			while (letra != ' '){
				if(contador>100) {
					std::cout << "Ocorreu um erro a transferir o ficheiro: Não foi possivel encontrar o tamanho do mesmo." << std::endl;
					return;
				}
	
			ret_tcp=recv(accept_fd_tcp,&letra,1,0);
			if(letra == ' ')
				break;
			
			size_buffer += letra;
			contador++;
			std::cout << "buffer: " << letra << "      contador: " << contador << std::endl;

			}
			this->strip(tcp_buffer);
			std::cout << "Buffer com: " << tcp_buffer << std::endl;
			this->ups_command();
		}
		


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
		std::cout << "Vou criar um fork..." << std::endl;
		pid = fork();
		if( pid == -1 ) {
			exit(1);
		} else if( pid == 0 ) {
			std::cout << "Fork criado..." << std::endl;
			this->processTCP();
			_exit(0);
		}

		//Parent process

		do {
			ret_tcp = close(accept_fd_tcp);
		} while (ret_tcp == -1 && errno==EINTR);

		if(ret_tcp==-1)return;
	}
}

void SServer::strip(char *s) {
    char *p2 = s;
    while(*s != '\0') {
    	if(*s != '\t' && *s != '\n') {
    		*p2++ = *s++;
    	} else {
    		++s;
    	}
    }
    *p2 = '\0';
}





