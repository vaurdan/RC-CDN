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
#include <math.h>
#include "Client.h"

inline void Client::loadbar(unsigned int x, unsigned int n, unsigned int w)
{
	if( x != n) {
		if ( (x > n) || (loadbar_acc < ceil(n/w)) ) { 
			loadbar_acc += 128; 
			return; 
		}
	}

	loadbar_acc = 0;

	float ratio  =  x/(float)n;
	int   c      =  ratio * w;

	printf("\e[?25l"); 
	std::cout << std::setw(3) << (int)(ratio*100) << "% [" << std::flush;
	for (int x=0; x<c; x++) std::cout << "=" << std::flush;
	for (unsigned int x=c; x<w; x++) std::cout << " " << std::flush;
	std::cout << "]\r" << std::flush;
	printf("\e[?25h"); 

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
	

//processamento do pedido list pelo cliente
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
	
	std::vector<std::string> resposta = parse_response(buffer);

	if(resposta.empty()) {
		std::cout << "Nenhum ficheiro foi encontrado." << std::endl;
		return;
	}
	
	int number = 1;
	std::cout << "NUMERO\tNOME DO FICHEIRO" << std::endl;
	for (std::vector<std::string>::iterator it = resposta.begin(); it != resposta.end(); ++it) {
		std::cout << number++ << "\t" << *it << std::endl;
	}
	
}

//processamento do pedido retrieve pelo cliente
void Client::retrieve(std::string file_name){

	FILE *ficheiro_recebido;
	int remain_data = 0;
	ssize_t len;
	std::string size_buffer = "";
	int file_size;
	
	// Inicializar a connecção ao socket.
	this->connectionSS();
	
	std::string command = "REQ " + file_name + "\n";
	connect_id=send(fd_tcp_ss, command.c_str(), command.size(), 0);
	if(connect_id ==-1) {
		std::cout << "Ocorreu um erro:  " << strerror(errno) << std::endl;
		return;	
	}

	// Vamos ver se o comando está correcto
	bzero(buffer,600);
	addrlen_tcp_ss=sizeof(addr_tcp_ss);
	recieve_id=recv(fd_tcp_ss,buffer,4,0);

	if(recieve_id ==-1) {
		std::cout << "Ocorreu um erro:  " << strerror(errno) << std::endl;
		return;	
	}
	
	//Leitura do comando do servidor
	if(strcmp (buffer, "ERR\n") == 0){
		std::cerr << "Erro, pedido mal formulado" << std::endl;
		return;
	}
	bzero(buffer,100);

	//Leitura de ok ou nok
	recieve_id=recv(fd_tcp_ss,buffer,3,0);
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
	
	file_size = atoi(size_buffer.c_str());
	char file_buffer[128];
	
	ficheiro_recebido = fopen(file_name.c_str(), "w");

	if(ficheiro_recebido == NULL){
		fprintf(stderr, "Falha a abrir o ficheiro: %s\n", strerror(errno));
		return;		
	}

	remain_data = file_size;
	
	int i = 0;
	int read_amount;
	do {
		read_amount = remain_data;
		if(read_amount > 128)
			read_amount = 128;

		len = recv(fd_tcp_ss,file_buffer,read_amount,0);
		if( len == -1) {
			std::cout << "Ocorreu um erro a receber o ficheiro:  " << strerror(errno) << std::endl;
			return;	
		}

		fwrite(file_buffer, sizeof(char), len, ficheiro_recebido);
		i+=len;
		remain_data -= len;

		loadbar(i, file_size);

	} while(len > 0 && (remain_data > 0));

	fclose(ficheiro_recebido);
	std::cout << std::endl << "Done! " << std::endl;
	
 
	}

//processamento do pedido upload pelo cliente
void Client::upload(std::string up_file_name){
	
	this->connectionCS(1);
	FILE *up_file;
	int size;
	char *data[128];
	
	
	std::string command = "UPR " + up_file_name + "\n";
	connect_id=send(fd_tcp_cs, command.c_str(), command.size(), 0);

	if(connect_id ==-1) {
		std::cout << "Erro de envio: " << strerror(errno) << std::endl;
		return;	
	}

	bzero(buffer,600);
	recieve_id=recv(fd_tcp_cs,buffer,4,0);

	if(recieve_id ==-1) {
		std::cout << "Erro de recepcao." << std::endl;
		return;	
	}
	
	//Leitura do comando do servidor
	if(strcmp (buffer, "ERR\n") == 0){
		std::cerr << "Erro, pedido mal formulado" << std::endl;
		return;
	}

	bzero(buffer,100);
	//Leitura de dup ou new
	recieve_id=recvfrom(fd_tcp_cs,buffer,4,0,(struct sockaddr*)&addr_tcp_cs,&addrlen_tcp_cs);
	if(recieve_id ==-1) {
		std::cout << "Erro de recepcao." << std::endl;
		return;	
	}
		
	if(strcmp(buffer, "dup\n") == 0){
		std::cerr << "Erro, ficheiro " << up_file_name << " já existe." << std::endl;
		return;
	} else{

		up_file=fopen(up_file_name.c_str(), "r");
		if(up_file == NULL){
			fprintf(stderr, "Falha a abrir o ficheiro: %s\n", strerror(errno));
			return;		
		}

		size = this->file_size(fileno(up_file));

		std::ostringstream command_stream;
		command_stream << "UPC " << size << " ";
		std::string command = command_stream.str();
		connect_id=send(fd_tcp_cs, command.c_str(), command.size(), 0);
		if(connect_id ==-1) {
			std::cout << "Erro de envio 2: " << strerror(errno) << std::endl;
			return;	
		}

		int tamanho_lido;
		int i = 0;
		do{	
			
			tamanho_lido = fread(data, 1, 128, up_file);
			if(tamanho_lido == -1){
				std::cerr << "Ocorreu um erro: " << strerror(errno) << std::endl;
				return;
			}

			connect_id=send(fd_tcp_cs, data, tamanho_lido, 0);
			if(connect_id ==-1) {
				std::cout << "Ocorreu um erro a enviar o bloco: " << strerror(errno) << std::endl;
				return;	
			}

			i += tamanho_lido;
			loadbar(i, size);

		}while(tamanho_lido > 0);
		fclose(up_file);
			
		connect_id=send(fd_tcp_cs, "\n", 1, 0);
		if(connect_id ==- 1) {
			std::cout << "Erro de envio da barra n." << std::endl;
			return;	
		}

		//Verificar se o ficheiro foi enviado, e responder de acordo com isso
		bzero(buffer,100);
		recieve_id = recv(fd_tcp_cs,buffer,6,0);
		std::cout << std::endl;

		if(strcmp( buffer, "AWC ok" ) == 0) {
			std::cout << "Ficheiro " << up_file_name << " enviado com sucesso..." << std::endl;	
		} else {
			std::cout << "Ocorreu um erro a enviar o  " << up_file_name << "." << buffer << std::endl;	
		}
		
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
		fd_udp_cs=socket(AF_INET, SOCK_DGRAM, 0);
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
		
		struct addrinfo host_info;       
		struct addrinfo *host_info_list; 

		memset(&host_info, 0, sizeof host_info);

		host_info.ai_family = AF_INET;     
		host_info.ai_socktype = SOCK_STREAM;

		int status = getaddrinfo(host_name, cs_port, &host_info, &host_info_list);

		fd_tcp_cs=socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);

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
	
	struct addrinfo host_info;       
	struct addrinfo *host_info_list; 

	memset(&host_info, 0, sizeof host_info);
	
	host_info.ai_family = AF_INET;     
	host_info.ai_socktype = SOCK_STREAM; 
	
	int status = getaddrinfo(this->ss_ip, this->ss_port, &host_info, &host_info_list);
	
	fd_tcp_ss=socket(host_info_list->ai_family, host_info_list->ai_socktype,
			   host_info_list->ai_protocol);
	
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
