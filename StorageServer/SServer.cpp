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

//Definição da função to_string devido a problema de compatibilidade com o compilador
template <typename T>
std::string SServer::to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

//recebe uma string e faz o split dado um certo delimitador returnando os vários elementos
std::vector<std::string> &SServer::split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

//recebe uma string e faz o split dado um certo delimitador returnando os vários elementos
std::vector<std::string> SServer::split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

//inicialização dos storage server com a criação das pastas de cada SS e respectivas portas. Iniciação da ligação TCP.
bool SServer::startListening() {
	std::cout << ":::: Storage Server ::::" << std::endl;
	std::cout << ":::: Creating Storage Server Dir ::::" << std::endl;

	std::cout << "Trying to listen on port " << ss_port << "..." << std::endl;
	
	//Start the TCP connection.
	this->initTCP();
		
}

// testa a porta e se nao estiver disponivel, muda
bool SServer::testConnection() {

	int valid_port = atoi(this->ss_port);


	if((fd_tcp=socket(AF_INET,SOCK_STREAM,0))==-1) {
		std::cout << "TCP: Error initializing the socket";
		exit(EXIT_FAILURE);
	}

	do {
		//Actualizamos a porta que vamos testar
		std::string string_port = this->to_string(valid_port);
		char temp_port[10];
		bzero(temp_port,10);
		strncpy(temp_port, string_port.c_str(), 5);
		std::cout << "porta tmp:" << temp_port << std::endl;

		//Iniciamos a ligação
		memset((void*)&addr_tcp,(int)'\0',sizeof(addr_tcp));
		addr_tcp.sin_family=AF_INET;
		addr_tcp.sin_addr.s_addr=htonl(INADDR_ANY);
		addr_tcp.sin_port=htons(valid_port);
		
		//Testamos o bind
		ret_tcp=bind(fd_tcp,(struct sockaddr*)&addr_tcp,sizeof(addr_tcp));
		if(ret_tcp == -1) {
			std::cout << "Binding error on port " << temp_port << ": " << strerror(errno) << std::endl;
			valid_port++;
			continue;
		}

		//Já temos ligação.
		if( listen(fd_tcp,2) == -1) {
			std::cout << "TCP: Error initializing the listen";
			exit(EXIT_FAILURE);
		}

		this->ss_port = (char*)malloc(sizeof(char)*10);
		strncpy(this->ss_port, temp_port, 10);

		std::cout << "Listening on port " << this->ss_port << "..." << std::endl;
		return true;

	} while(ret_tcp == -1);

	return true;
}

//processamento do comando REQ por consequência do pedido retrieve do user.
void SServer::REQ_command(std::string fn) {

	FILE *req_file;

	int fsize;
	char *buffer_retrieve;
	char data[128];
	size_t result;
	std::string req_response;
	std::string file_size_result;
	std::ostringstream convert;


	std::cout << "TCP: REQ requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	std::string req_dir = "SS" + std::string(ss_port) + "/" + std::string(fn);
	req_file =fopen(req_dir.c_str(), "r");

	if(req_file == NULL){
		req_response = "REP " + std::string("nok");
		ret_tcp=send(accept_fd_tcp,req_response.c_str(),req_response.size(),0);
		if(ret_tcp==-1) {
			std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
			return;
		}
		std::cout << "TCP: No file provided." << std::endl;

		return;		
	}

	//Primeiro enviamos a primeira parte da mensagem "REP ok SIZE"
	fseek(req_file, 0, SEEK_END);
	fsize = ftell (req_file);
	rewind (req_file); //volta ao inicio para fazer a leitura

	std::stringstream size_string;
	size_string << fsize;

	req_response = "REP " + std::string("ok ") + size_string.str() + std::string(" ");

	std::cout << "TCP: Sending " << req_response << std::endl;
	ret_tcp=send(accept_fd_tcp,req_response.c_str(),req_response.size(),0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response sent." << std::endl;

	int tamanho_lido;
	do{	
		
		tamanho_lido = fread(data, 1, 128, req_file);
		if(tamanho_lido == -1){
			std::cerr << "Não foi possivel ler o tamanho do ficheiro: " << strerror(errno) << std::endl;
		}
		ret_tcp=send(accept_fd_tcp, data, tamanho_lido, 0);
		if(ret_tcp ==-1) {
			std::cout << "Erro de envio ciclo: " << strerror(errno) << std::endl;
			return;	
		}

	} while(tamanho_lido > 0);
		
	std::cout << tamanho_lido << std::endl;
	std::string barra_n = "\n";
	ret_tcp=send(accept_fd_tcp, barra_n.c_str(), barra_n.size(), 0);
	if(ret_tcp ==-1) {
		std::cout << "Erro de envio da barra n." << std::endl;
		return;	
	}
	fclose(req_file);

	std::cout << "TCP: Retrieving of file " << fn << " by " << inet_ntoa(addr_tcp.sin_addr) << " done!" << std::endl;

 
}

//processamento do comando UPS por consequência do pedido upload do user.
void SServer::UPS_command(std::string fn, std::string fn_size){

	FILE *ficheiro_recebido;
	int remain_data = 0;
	ssize_t len;
	char ups_buffer[128];
	int result_tcp_fnsize;
	std::string ups_response;
	int read_amount = 128;

	if( ! (std::istringstream(fn_size) >> result_tcp_fnsize) ) result_tcp_fnsize = 0;
	
	fn = "SS" + std::string(ss_port) + std::string("/") + fn;
	std::cout << "TCP: UPS requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;

	bzero(ups_buffer,100);

	char file_buffer[result_tcp_fnsize];
	ficheiro_recebido = fopen(fn.c_str(), "w");
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
		remain_data -= len;

		fwrite(ups_buffer, sizeof(char), len, ficheiro_recebido);

	} while(len > 0);
	
	fclose(ficheiro_recebido);

	ups_response = "AWS ok\n";
	ret_tcp=send(accept_fd_tcp,ups_response.c_str(),ups_response.size(),0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}
	std::cout << "TCP: Response " << ups_response << " sent." << std::endl;
	std::cout << "TCP: Uploading of file " << fn << " by " << inet_ntoa(addr_tcp.sin_addr) << " done!" << std::endl;

}

//processa as ligações TCP com os pedidos do central server
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
		
		//Lẽ os primeiros 4 bytes que pode ser "UPS " ou "REQ "
		nread_tcp=read(accept_fd_tcp,tcp_buffer,4);
		if(nread_tcp==-1) {
			std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
			return;
		}
		if(strcmp(tcp_buffer, "REQ ") == 0){
			nread_tcp=recv(accept_fd_tcp,tcp_buffer,30,0);
			this->strip(tcp_buffer);
			this->REQ_command(tcp_buffer);
	   } else if(strcmp(tcp_buffer, "UPS ") == 0){
		   	
			int contador = 0;
			int contador_2 = 0;
			
			//leitura do nome do ficheiro, lê x bytes até encontrar um espaço
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
			//leitura do tamanho do ficheiro, lê x bytes até encontrar um espaço			
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
			//pedido de processamento do comando enviando o nome e tamanho do ficheiro sobrando no buffer os dados que vai escrever.
			this->UPS_command(nome_ficheiro, tamanho_ficheiro);

			}

}

//inicializações TCP com Servidor Central
void SServer::initTCP() {

	std::cout << "TCP: Initialization..." << std::endl;

	//Testa e inicializa a ligação
	this->testConnection();
	//criação das directorias dos vários SS
	std::string ss_dir = "SS" + std::string(ss_port);
	mkdir(ss_dir.c_str(),0755);

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





