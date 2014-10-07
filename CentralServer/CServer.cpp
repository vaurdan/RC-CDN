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
#include <time.h>


#include "CServer.h"
#include "../Client/Client.h"

void CServer::startListening() {
	std::cout << ":::: Central Server ::::" << std::endl;
	std::cout << "Listening on port " << cs_port << "..." << std::endl;

	//Retrieve the storages server list
	this->retrieveStorage();

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

void CServer::list_command() {
	std::srand(time(0));
	int random_server = std::rand() % storages.size();
	std::vector<std::string> files = this->retrieveFiles();
	int files_count = files.size();

	std::vector<std::string> server = storages[random_server];
	std::string command = "AWL " + server[0] + " " + server[1] + " " + std::to_string(files_count) + " ";
	for (std::vector<std::string>::iterator it = files.begin() ; it != files.end(); ++it)
		command += (std::string) *it;
	command += "\n\0";
	//TODO: Validar o pedido LST, verificar se EOF (sem ficheiros no servidor)
	std::cout << "UDP: List requested by " << inet_ntoa(addr_udp.sin_addr) << "..." << std::endl;
	bzero(buffer, 600);
	strncpy(buffer, command.c_str(),command.size());
}

char* CServer::UPR_command(char* filename) {
	char temp_buffer[300];
	std::cout << "FILENAME: " << filename << std::endl;
	std::cout << "TCP: UPR requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	// Percorrer os ficheiros e ver se existe.
	std::vector<std::string> files = this->retrieveFiles();
	for (std::vector<std::string>::iterator it = files.begin() ; it != files.end(); ++it) {
		std::cout << filename << " == " << ((std::string)*it).c_str() << std::endl;
		if( strcmp(filename, ((std::string)*it).c_str()) == 0 ) {
			std::string command = "AWR dup\n\0";
			strncpy(temp_buffer,command.c_str(), command.size());
			return temp_buffer;

		}
	}
	std::string command = "AWR new\n\0";
	strncpy(temp_buffer,command.c_str(), command.size());
	return temp_buffer;

}

void CServer::processTCP() {
	char tcp_buffer[600];
	bzero(tcp_buffer, 600);
	nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,4,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
	if(nread_tcp==-1) {
		std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
		return;
	}

	// Processamento dos comandos TCP
	if(strcmp(tcp_buffer, "UPR ") == 0){
		bzero(tcp_buffer,600);

		nread_tcp=recvfrom(accept_fd_tcp,tcp_buffer,30,0,(struct sockaddr*)&addr_tcp,&addrlen_tcp);
		std::cout << "buffer: " << tcp_buffer << std::endl;
		char* result = this->UPR_command(tcp_buffer);
		strncpy(tcp_buffer, result, 600);
	} else {
		strncpy(tcp_buffer, "ERR\n\0", 5);
	}

	ret_tcp=send(accept_fd_tcp,tcp_buffer,nread_tcp,0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response sent: " << tcp_buffer << std::endl;
}

void CServer::processUDP() {
	std::cout << "UDP: Waiting for connections..." << std::endl;
	addrlen_udp=sizeof(addr_udp);
	bzero(buffer, 600);
	nread_udp=recvfrom(fd_udp,buffer,600,0,(struct sockaddr*)&addr_udp,&addrlen_udp);
	if(nread_udp==-1) {
		std::cout << "UDP: recv error: " << strerror(errno) << std::endl;
		return;
	}

	// Get the IP address of the requestent 
	int peer_name = getpeername(fd_udp,(struct sockaddr*)&addr_udp,&addrlen_udp);

	if(strcmp(buffer, "LST\n") == 0){
		this->list_command();
	} else {
		strncpy(buffer, "ERR\n\0", 5);
	}

	ret_udp=sendto(fd_udp,buffer,600,0,(struct sockaddr*)&addr_udp, addrlen_udp);
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

		pid = fork();
		if( pid == -1 ) {
			exit(1);
		} else if( pid == 0 ) {
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

void CServer::retrieveStorage() {

	//Criar directório para o Central Server
	std::ifstream input( "serverlist.txt" );
	if( !input.good() ) {
		std::cerr << "ERRO: Impossível ler ficheiro com a lista de servidores." << std::endl;
		return;
	}
	std::string line;
	while( std::getline(input, line, '\n') ) {
		std::vector< std::string > server = split(line, ' ');
		storages.push_back(server);
	}

}

std::vector<std::string> CServer::retrieveFiles() {

	std::vector<std::string> files;

	std::ifstream input( "files.txt" );
	if( !input.good() ) {
		std::cerr << "ERRO: Impossível ler ficheiro com a lista de ficheiros." << std::endl;
		return std::vector<std::string>();
	}
	std::string line;
	while( std::getline(input, line, '\n') ) {
		files.push_back(line);
	}

	return files;
}

/*
 * Definição das funções de split
 */
std::vector<std::string> &CServer::split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> CServer::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void CServer::strip(char *s) {
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
