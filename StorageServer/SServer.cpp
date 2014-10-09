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

std::vector<std::string> &SServer::split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> SServer::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
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


void SServer::ups_command(std::string fn, std::string fn_size){

	FILE *ficheiro_recebido;
	int remain_data = 0;
	ssize_t len;
	char ups_buffer[128];
	int result_tcp_fnsize;
	std::string ups_response;
	int read_amount = 128;

	if( ! (std::istringstream(fn_size) >> result_tcp_fnsize) ) result_tcp_fnsize = 0;
	
	fn = "SS" + std::string(ss_port) + std::string("/") + fn;
	std::cout << "Directoria: " << fn << std::endl;
	std::cout << "TCP: UPS requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;

	bzero(ups_buffer,100);

	char file_buffer[result_tcp_fnsize];
	std::cout << "File size: " << result_tcp_fnsize << std::endl;
	ficheiro_recebido = fopen(fn.c_str(), "w");
	std::cout << "Iniciei criação do ficheiro" << std::endl;
	if(ficheiro_recebido == NULL){
		fprintf(stderr, "Falha a abrir o ficheiro --> %s\n", strerror(errno));
		ups_response = "AWS nok\n";
		ret_tcp=send(accept_fd_tcp,ups_response.c_str(),ups_response.size(),0);
		return;
	}

	remain_data = result_tcp_fnsize;
	
	
	bzero(ups_buffer,128);
	do {
		read_amount = remain_data;
		if(read_amount > 128)
		read_amount = 128;

		len = recv(accept_fd_tcp,ups_buffer,read_amount,0);
		fwrite(ups_buffer, sizeof(char), len, ficheiro_recebido);
		remain_data -= len;

	}while(len > 0 && (remain_data > 0));
	
	fclose(ficheiro_recebido);

	ups_response = "AWS ok\n";
	ret_tcp=send(accept_fd_tcp,ups_response.c_str(),ups_response.size(),0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response sent." << std::endl;


}

void SServer::processTCP() {
	
	char tcp_buffer[128];
	bzero(tcp_buffer, 128);
	int read_amount;
	ssize_t len;
	char letra;
	char letra2;
	std::string nome_ficheiro = "";
	std::string tamanho_ficheiro = "";
	// Processamento dos comandos TCP
		
		nread_tcp=read(accept_fd_tcp,tcp_buffer,4);
		if(nread_tcp==-1) {
			std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
			return;
		}
		std::cout << "Comando : " << tcp_buffer << std::endl;
		if(strcmp(tcp_buffer, "REQ ") == 0){
			nread_tcp=recv(accept_fd_tcp,tcp_buffer,30,0);
			this->strip(tcp_buffer);
			std::cout << "Buffer com: " << tcp_buffer << std::endl;
			this->req_command(tcp_buffer);
	   } else if(strcmp(tcp_buffer, "UPS ") == 0){
		   
		   std::cout << "Entrei no processamento do comando UPS" << std::endl;

			
	
			int contador = 0;
			int contador_2 = 0;
			
			while (letra != ' '){
				if(contador>100) {
					std::cout << "Ocorreu um erro a transferir o ficheiro: Não foi possivel encontrar o nome do mesmo." << std::endl;
					return;
				}
	
				nread_tcp=recv(accept_fd_tcp,&letra,1,0);
				if(letra == ' ')
					break;
		
				nome_ficheiro += letra;
				contador++;

				

			}
			std::cout << "Nome ficheiro: " << nome_ficheiro << std::endl;
			
			while (letra2 != ' '){
				if(contador_2>100) {
					std::cout << "Ocorreu um erro a transferir o ficheiro: Não foi possivel encontrar o tamanho do mesmo." << std::endl;
					return;
				}
	
				nread_tcp=recv(accept_fd_tcp,&letra2,1,0);
				if(letra2 == ' ')
					break;
		
				tamanho_ficheiro += letra2;
				contador_2++;
			}
			std::cout << "Tamanho ficheiro: " << tamanho_ficheiro << std::endl;			

			this->ups_command(nome_ficheiro, tamanho_ficheiro);

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





