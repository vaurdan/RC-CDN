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
#include "Client.h"

inline void Client::loadbar(unsigned int x, unsigned int n, unsigned int w)
{
	if ( (x != n) && (x % (n/100+1) != 0) ) return;
	
	float ratio  =  x/(float)n;
	int   c      =  ratio * w;
	
	std::cout << std::setw(3) << (int)(ratio*100) << "% [";
	for (int x=0; x<c; x++) std::cout << "=";
	for (unsigned int x=c; x<w; x++) std::cout << " ";
	std::cout << "]\r" << std::flush;
}

/*
 * Definição das funções de split
 */
std::vector<std::string> &Client::split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> Client::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}


std::vector<std::string> Client::parse_files( std::vector<std::string> response ) {
	//Inverter a lista
	std::reverse(response.begin(),response.end());
	
	std::string command = response.back();
	response.pop_back();

	std::string ip = response.back();
	response.pop_back();
	this->ss_ip = new char[ip.length() + 1];
	std::strcpy (this->ss_ip, ip.c_str());
	
	std::string port = response.back();
	response.pop_back();
	this->ss_port = new char[port.length() + 1];
	std::strcpy (this->ss_port, port.c_str());
	
	
	unsigned int num_files = atoi(response.back().c_str());
	response.pop_back();
	
	if(num_files != response.size() || command != "AWL" ) {
		return std::vector<std::string>();
	}
	
	std::reverse(response.begin(),response.end());
	
	return response;
	
}

std::vector<std::string> Client::parse_response(char* buffer) {
	std::string content(buffer);
	std::vector<std::string> elements = split(content,' ');
	
	return parse_files(elements);  
}
	


void Client::list() {
	
	this->connectionCS(0);
	
	// Enviar a mensagem para o servidor
	connect_id=sendto(fd_udp_cs, "LST\n", 4, 0, (struct sockaddr*)&addr_udp_cs, sizeof(addr_udp_cs));
	if(connect_id==-1)
		exit(1);
	
	addrlen_udp_cs=sizeof(addr_udp_cs);
	recieve_id=recvfrom(fd_udp_cs,buffer,600,0,(struct sockaddr*)&addr_udp_cs,&addrlen_udp_cs);
	if(recieve_id==-1)
		exit(1);
	
	std::cout << buffer << std::endl;
	std::vector<std::string> resposta = parse_response(buffer);
	if(resposta.empty()) {
		std::cout << "Erro de protocolo" << std::endl;
		return;
	}
	
	int number = 1;
	std::cout << "NUMERO\tNOME DO FICHEIRO" << std::endl;
	for (std::vector<std::string>::iterator it = resposta.begin(); it != resposta.end(); ++it) {
		std::cout << number++ << "\t" << *it << std::endl;
	}
	
}

//bool para dizer ao utilizador que ficheiro foi transferido?
void Client::retrieve(std::string file_name){

	FILE *ficheiro_recebido;
	int remain_data = 0;
	ssize_t len;
	std::string size_buffer = "";
	int file_size;
	
	// Inicializar a connecção ao socket.
	this->connectionSS();
	
	std::string command = "REQ " + file_name + "\n";
	connect_id=sendto(fd_tcp_ss, command.c_str(), command.size(), 0, (struct sockaddr*)&addr_tcp_ss, sizeof(addr_tcp_ss));
	//std::cout << "connect_id com " << connect_id << std::endl;	
	if(connect_id==-1)
		exit(1);

	// Vamos ver se o comando está correcto
	bzero(buffer,600);
	addrlen_tcp_ss=sizeof(addr_tcp_ss);
	recieve_id=recvfrom(fd_tcp_ss,buffer,4,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);

	if(recieve_id ==-1)
		exit(1);
	
	//Leitura do comando do servidor
	if(strcmp (buffer, "ERR\n") == 0){
		std::cerr << "Erro, pedido mal formulado" << std::endl;
		return;
	}
	bzero(buffer,100);

	//Leitura de ok ou nok
	recieve_id=recvfrom(fd_tcp_ss,buffer,3,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);
	if(recieve_id ==-1)
		exit(1);
	
	if(strcmp (buffer, "nok") == 0){
		std::cerr << "Erro, ficheiro '" << file_name << "' não existe" << std::endl;
		return;		
	}
	
	bzero(buffer,100);
	char letra;
	
	int contador = 0;
	while (letra != ' '){
		if(contador>100) {
			std::cout << "Ocorreu um erro a transferir o ficheiro: Não foi possivel encontrar o tamanho do mesmo." << std::endl;
			return;
		}
	
		recieve_id=recvfrom(fd_tcp_ss,&letra,1,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);
		if(letra == ' ')
			break;
		
		size_buffer += letra;
		contador++;

	}
	
	//std::cout << "size_buffer: " << size_buffer << std::endl;
	std::cout << "buffer: " << buffer << std::endl;
	
	file_size = atoi(size_buffer.c_str());
	char file_buffer[file_size];
	
	ficheiro_recebido = fopen(file_name.c_str(), "w");
	//std::cout << "Iniciei criação do ficheiro" << std::endl;
	if(ficheiro_recebido == NULL){
		fprintf(stderr, "Falha a abrir o ficheiro --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);		
	}

	remain_data = file_size;
	setbuf(stdout, NULL);
	
	int i = 0;
	int read_amount;
	do {
		read_amount = remain_data;
		if(read_amount > 128)
			read_amount = 128;
	
		len = recvfrom(fd_tcp_ss,file_buffer,read_amount,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);
		fwrite(file_buffer, sizeof(char), len, ficheiro_recebido);
		i += len;
		remain_data -= len;
		loadbar(i, file_size);
	} while(len > 0 && (remain_data > 0));
	fclose(ficheiro_recebido);
	std::cout << std::endl << " Done! " << std::endl;
	
 
	}
	
void Client::upload(std::string up_file_name){
	
	this->connectionCS(1);
	FILE *up_file;
	int size;
	char *data[128];
	
	
	std::string command = "UPR " + up_file_name + "\n";
	connect_id=send(fd_tcp_cs, command.c_str(), command.size(), 0);
	std::cout << "commando enviado." << std::cout;
	if(connect_id ==-1) {
		std::cout << "Erro de envio." << std::endl;
		return;	
	}
	std::cout << "reconhecido UPR" << std::endl;
	bzero(buffer,600);
	recieve_id=recv(fd_tcp_cs,buffer,4,0);
	std::cout << "receive com: " << recieve_id << std::endl;
	if(recieve_id ==-1) {
		std::cout << "Erro de recepcao." << std::endl;
		return;	
	}

	std::cout << "buffer: " << buffer << std::endl;
	
	//Leitura do comando do servidor
	if(strcmp (buffer, "ERR\n") == 0){
		std::cerr << "Erro, pedido mal formulado" << std::endl;
		return;
	}

	bzero(buffer,100);
	//Leitura de dup ou new
	recieve_id=recvfrom(fd_tcp_cs,buffer,3,0,(struct sockaddr*)&addr_tcp_cs,&addrlen_tcp_cs);
	if(recieve_id ==-1) {
		std::cout << "Erro de recepcao." << std::endl;
		return;	
	}

	std::cout << "buffer: " << buffer << std::endl;
		
	if(strcmp(buffer, "dup") == 0){
		std::cerr << "Erro, ficheiro " << up_file_name << " já existe." << std::endl;
		return;
	} else{
		
		// TCP reconnection
//		this->connectionCS(1);

		up_file=fopen(up_file_name.c_str(), "r");
		if(up_file == NULL){
			fprintf(stderr, "Falha a abrir o ficheiro: %s\n", strerror(errno));
			return;		
		}

		size = this->file_size(fileno(up_file));
		std::cout << size << std::endl;
		std::ostringstream command_stream;
		command_stream << "UPC " << size << " ";
		std::string command = command_stream.str();
		connect_id=send(fd_tcp_cs, command.c_str(), command.size(), 0);
		if(connect_id ==-1) {
			std::cout << "Erro de envio." << std::endl;
			return;	
		}
		int tamanho_lido;

		while((tamanho_lido = fread(data,1,128, up_file)) > 0) {
			
			connect_id=send(fd_tcp_cs, data, tamanho_lido, 0);
			if(connect_id ==-1) {
				std::cout << "Erro de envio." << std::endl;
				return;	
			}
		}

		std::string barra_n = "\n";
		connect_id=send(fd_tcp_cs, barra_n.c_str(), barra_n.size(), 0);
		if(connect_id ==-1) {
			std::cout << "Erro de envio da barra n." << std::endl;
			return;	
		}
		fclose(up_file);
		
		std::cout << "Ficheiro enviado! " << std::endl;		
		//connect_id=sendto(fd_tcp_cs, command.c_str(), command.size(), 0, (struct sockaddr*)&addr_tcp_cs, sizeof(addr_tcp_cs));
		}
	}
	
int Client::file_size(int fd){
	struct stat stat_buf;
	int rc = fstat(fd, &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

bool Client::connectionCS(int type) {
	
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
}

bool Client::connectionSS() {
	
	if(this->ss_port == NULL || this->ss_ip == NULL)
		return false;
	
	struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
	struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.

	memset(&host_info, 0, sizeof host_info);
	
	host_info.ai_family = AF_INET;     // IP version not specified. Can be both.
	host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
	
	int status = getaddrinfo(this->ss_ip, this->ss_port, &host_info, &host_info_list);
	
	fd_tcp_ss=socket(host_info_list->ai_family, host_info_list->ai_socktype,
			   host_info_list->ai_protocol);//SOCKET do TCP
	
	if(fd_tcp_ss==-1)
		return false;
	
	status = connect(fd_tcp_ss, host_info_list->ai_addr, host_info_list->ai_addrlen);
	
	if(status == -1 )
		return false;
	
	return true;

}

void Client::close_all() {

	close(fd_tcp_ss);
	close(fd_udp_cs);
	close(fd_tcp_cs);

}
