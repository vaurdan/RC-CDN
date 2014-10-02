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
#include "Client.h"

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


/** AWL
 192.168.128.2 59100 10
 f1.txt f222222.txt f3.jpg A380.jpg Boeing_747.jpg aaaaaa.txt bbbbbb.txt cccccc.txt dddddd.txt eeeeeee.txt
 */
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
        std::cout << "Erro de protocolo" << std::endl;
        return;
    }
    
    int number = 1;
    std::cout << "NUMERO\tNOME DO FICHEIRO" << std::endl;
    for (std::vector<std::string>::iterator it = resposta.begin(); it != resposta.end(); ++it) {
        std::cout << number++ << "\t" << *it << std::endl;
    }
    this->connectionSS();
    //Fechar as ligações
    close(recieve_id);
    close(connect_id);
    
    
}

//bool para dizer ao utilizador que ficheiro foi transferido?
void Client::retrieve(std::string file_name){

	int tamanho_ficheiro;
	FILE *ficheiro_recebido;
	int remain_data = 0;
	ssize_t len;
	int size_buffer;
	
	

    std::string command = "REQ " + file_name + "\n";
	connect_id=sendto(fd_tcp_ss, command.c_str(), 20, 0, (struct sockaddr*)&addr_tcp_ss, sizeof(addr_tcp_ss));
	std::cout << "connect_id com " << connect_id << std::endl;	
	if(connect_id==-1)
		exit(1);

	//std::cout << "Buffer: " << buffer << std::endl;
	bzero(buffer,600);
	addrlen_tcp_ss=sizeof(addr_tcp_ss);
	recieve_id=recvfrom(fd_tcp_ss,buffer,600,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);
	std::cout << "recieve_id com " << recieve_id << std::endl;
	if(recieve_id ==-1)
		exit(1);
	
	std::cout << "Buffer: " << buffer << std::endl;
	
	if(strcmp (buffer, "REP nok\n") == 0){
		std::cerr << "Erro, ficheiro '" << file_name << "' não existe" << std::endl;
		exit(0);		
	}else {
		std::vector<std::string> size_buffer_response = split(buffer, ' ');
		size_buffer =  atoi(size_buffer_response.back().c_str());
	}
	
	//stackoverflow.com/question/11952898/c-send-and-receive-file
	char file_buffer[size_buffer];
	
	
	recv(fd_tcp_ss, file_buffer, 512, 0);
	tamanho_ficheiro = size_buffer;
	
	ficheiro_recebido = fopen(file_name.c_str(), "w");
	std::cout << "Iniciei criação do ficheiro" << std::endl;
	if(ficheiro_recebido == NULL){
		fprintf(stderr, "Falha a abrir o ficheiro --> %s\n", strerror(errno));
		exit(EXIT_FAILURE);		
		}
	remain_data = tamanho_ficheiro;
	
	while((len = recv(fd_tcp_ss, file_buffer, 300, 0)) > 0 && (remain_data > 0)){
		fwrite(file_buffer, sizeof(char), len, ficheiro_recebido);
		remain_data -= len;
		//fprintf(stdout, "Recebidos %l bytes e esperava :- %d bytes\n", len, remain_data);
		
		}
		fclose(ficheiro_recebido);
		
	/*std::cout << "Vou iniciar a criação do ficheiro" << std::endl;
	
	std::ofstream outFile(file_name.c_str(), std::ios::out|std::ios::binary|std::ios::app);
	int size = 600;
	int n;
	std::cout << "Vou iniciar a leitura do ficheiro" << std::endl;
	while(size > 0){
		
		bzero(buffer,600);
		std::cout << "entrei no ciclo e limpei buffer" << std::endl;
		if(size >= 600){
			
			std::cout << "entrei no if com size: " << size << std::endl;
			n = recv(fd_tcp_ss,buffer,600,0);
			std::cout << "tou aqui!!" << std::endl;
			std::cout << "n do if tem: " << n << std::endl;
			outFile.write(buffer,n);
			std::cout << "ja escrevi no if" << std::endl;
			
			}else{
				
				std::cout << "entrei no else com size: " << size << std::endl;
				n = recvfrom(fd_tcp_ss,buffer,size,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);
				std::cout << "n do else tem: " << n << std::endl;
				buffer[size]='\0';
				outFile.write(buffer,n);
				std::cout << "ja escrevi no else" << std::endl;
				}
				
				size -= 600;
				std::cout << "Decrementei size" << size << std::endl;
		
		
		}
		
		outFile.close();
		std::cout << "Ficheiro carregado" << std::endl;*/

	/*memset(buffer,0,600);
	FILE *fn = fopen(file_name.c_str(), "a");
	std::cout << "Abri o ficheiro: " << file_name.c_str() <<std::endl;
	if(fn == NULL){
		std::cerr << "Não é possivel abrir o ficheiro" std::cout;
	}else{
		
		
		}
	
	
	*/
	//std::cout << buffer << std::endl;
	/*if(buffer == 0){
		fputs("Error reading file", stderr);
		}else{
		int fn_block_sz = 0;
		while((fn_block_sz = recvfrom(fd_tcp_ss,buffer,600,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss))){
			int write_sz = fwrite(buffer, sizeof(char), fn_block_sz, fn);
			if(write_sz < fn_block_sz){
				std::cerr << "File write failed." << std::endl;
				}
				memset(buffer,0,600);
				if(fn_block_sz == 0 || fn_block_sz != 600)
					break;
				
			}
			fclose(fn);			
		}
		
		close(fd_tcp_ss);
				
	*/
		
			
	
	
	
	
	close(fd_tcp_ss);
	
	//return 0;
 
	}
	
void Client::upload(std::string up_file_name){
	
	memset( buffer, 0, 600);	
	std::string command = "UPR " + up_file_name + "\n";
	connect_id=sendto(fd_tcp_ss, command.c_str(), 20, 0, (struct sockaddr*)&addr_tcp_ss, sizeof(addr_tcp_ss));
	std::cout << "connect_id com " << connect_id << std::endl;	
	if(connect_id==-1)
		exit(1);

	//std::cout << "Buffer: " << buffer << std::endl;
	memset( buffer, 0, 600);
	addrlen_tcp_ss=sizeof(addr_tcp_ss);
	recieve_id=recvfrom(fd_tcp_ss,buffer,600,0,(struct sockaddr*)&addr_tcp_ss,&addrlen_tcp_ss);
	std::cout << "recieve_id com " << recieve_id << std::endl;
	if(recieve_id ==-1)
		exit(1);
	
	std::cout << "Buffer: " << buffer << std::endl;
	
	}

void Client::connection() {
    
    // UDP connection to the Central Server
    fd_udp_cs=socket(AF_INET, SOCK_DGRAM, 0);//SOCKET DO UPD
    if(fd_udp_cs==-1)
        exit(1);
    
    memset((void*)&addr_udp_cs,(int)'\0',sizeof(&addr_udp_cs));
    addr_udp_cs.sin_family=AF_INET;
    a_udp_cs=(struct in_addr*)gethostbyname(host_name)->h_addr_list[0];
    addr_udp_cs.sin_addr.s_addr = a_udp_cs->s_addr;
    addr_udp_cs.sin_port=htons(cs_port);

    // TCP connection to the Central Server

    fd_tcp_cs=socket(AF_INET, SOCK_STREAM,0);//SOCKET do TCP
    if(fd_tcp_cs==-1)
        exit(1);
    
    memset((void*)&addr_tcp_cs,(int)'\0',sizeof(&addr_tcp_cs));
    addr_tcp_cs.sin_family=AF_INET;
    a_tcp_cs=(struct in_addr*)gethostbyname(host_name)->h_addr_list[0];
    addr_tcp_cs.sin_addr.s_addr = a_tcp_cs->s_addr;
    addr_tcp_cs.sin_port=htons(cs_port);
}

void Client::connectionSS() {
    
    struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
    struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.

    memset(&host_info, 0, sizeof host_info);
    
    host_info.ai_family = AF_INET;     // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
    
    int status = getaddrinfo(this->ss_ip, this->ss_port, &host_info, &host_info_list);
    
    fd_tcp_ss=socket(host_info_list->ai_family, host_info_list->ai_socktype,
               host_info_list->ai_protocol);//SOCKET do TCP
    
    if(fd_tcp_ss==-1)
        exit(1);
    
    status = connect(fd_tcp_ss, host_info_list->ai_addr, host_info_list->ai_addrlen);
    
    if(status == -1 ) exit(1);
	
	}
