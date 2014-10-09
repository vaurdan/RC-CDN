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

 
template <typename T>
std::string CServer::to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

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

void CServer::connectSS() {
	int storages_amount = storages.size();
	std::cout << storages_amount << " storage servers found. Initializing the connection..." << std::endl;

	this->fd_tcp_ss = (int*) malloc(sizeof(int)*storages_amount);
	this->addrlen_tcp_ss = (socklen_t*) malloc(sizeof(socklen_t)*storages_amount);
	this->addr_tcp_ss = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in)*storages_amount);

	int i = 0;

	for (std::vector< std::vector<std::string> >::iterator it = storages.begin() ; it != storages.end(); ++it) {
		std::vector<std::string> server = *it;
		std::cout << "Connecting to SS #" << i << ": " << server[0] << ":" << server[1] << "..."; 
		if( this->connectTCP( i++, server[0], server[1] ) )
			std::cout << " done!" << std::endl;
		else {
			std::cout << " error!" << std::endl;
		}

	}
}

void CServer::disconnectSS() {
	int storages_amount = storages.size();
	int i = 0;

	for (std::vector< std::vector<std::string> >::iterator it = storages.begin() ; it != storages.end(); ++it) {
		close(fd_tcp_ss[i++]);
	}
}

void CServer::list_command() {
	std::srand(time(0));
	int random_server = std::rand() % storages.size();
	std::vector<std::string> files = this->retrieveFiles();
	int files_count = files.size();

	std::vector<std::string> server = storages[random_server];
	std::string command = "AWL " + server[0] + " " + server[1] + " " + this->to_string(files_count);
	for (std::vector<std::string>::iterator it = files.begin() ; it != files.end(); ++it)
		command += " " + (std::string) *it;
	command += "\n\0";
	//TODO: Validar o pedido LST, verificar se EOF (sem ficheiros no servidor)
	std::cout << "UDP: List requested by " << inet_ntoa(addr_udp.sin_addr) << "..." << std::endl;
	bzero(buffer, 600);
	strncpy(buffer, command.c_str(),command.size());
}



bool CServer::connectTCP(int i, std::string server, std::string port) {
	
	
	struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
	struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.

	memset(&host_info, 0, sizeof host_info);
	
	host_info.ai_family = AF_INET;     // IP version not specified. Can be both.
	host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
	
	int status = getaddrinfo(server.c_str(), port.c_str(), &host_info, &host_info_list);
	
	fd_tcp_ss[i]=socket(host_info_list->ai_family, host_info_list->ai_socktype,
			   host_info_list->ai_protocol);//SOCKET do TCP
	
	if(fd_tcp_ss[i]==-1)
		return false;
	
	status = connect(fd_tcp_ss[i], host_info_list->ai_addr, host_info_list->ai_addrlen);
	
	if(status == -1 )
		return false;
	
	return true;
}

char* CServer::UPR_command(const char* filename) {
	char temp_buffer[300];
	bzero(temp_buffer, 300);
	std::cout << "TCP: ::: Starting upload " << filename << " ::: " << std::endl;
	std::cout << "TCP: UPR requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	// Percorrer os ficheiros e ver se existe.
	std::vector<std::string> files = this->retrieveFiles();
	for (std::vector<std::string>::iterator it = files.begin() ; it != files.end(); ++it) {
		if( strcmp(filename, ((std::string)*it).c_str()) == 0 ) {
			std::string command = "AWR dup\n";
			strncpy(temp_buffer,command.c_str(), command.size());
			return temp_buffer;

		}
	}
	std::string command = "AWR new\n";
	strncpy(temp_buffer,command.c_str(), command.size());
	return temp_buffer;
}

char* CServer::UPC_command(char* buffer, const char* new_filename) {
	char letra = 'a';
	std::string size_buffer = "";
	std::string result;

	std::cout << "TCP: UPC (Upload) requested by " << inet_ntoa(addr_tcp.sin_addr) << "..." << std::endl;
	std::cout << "Buffer: " << buffer << std::endl;
	// Ler o tamanho do ficheiro
	int contador = 0;
	while (letra != ' '){
		if(contador>100) {
			std::cout << "TCP: File Upload: Error getting file size." << std::endl;
			result = "ERR\n";
			strncpy(buffer, result.c_str(), result.size());
			return buffer;
		}
	
		nread_tcp=recv(accept_fd_tcp,&letra,1,0);
		std::cout << letra << std::endl;
		if(letra == ' ')
			break;
		
		size_buffer += letra;
		contador++;
	}

	int file_size = atoi(size_buffer.c_str());
	char file_buffer[file_size];
	//Enviar a informação para os storages

	//Start the TCP connection with the Storage Servers
	this->connectSS();

	int i = 0; // possivel optimização: nao é necessario usar iterador aqui
	for (std::vector< std::vector<std::string> >::iterator it = storages.begin() ; it != storages.end(); ++it) {
		std::vector<std::string> server = *it;
		std::string command = "UPS " + std::string(new_filename) + std::string(" ") + std::string(size_buffer.c_str()) + std::string(" ");
		std::cout << "TCP: Processing Upload (UPS) to Storage Server #"<<i<<std::endl;
		std::cout << command << std::endl;
		send(fd_tcp_ss[i++], command.c_str(), command.size(), 0);
	}

	int remain_data = file_size;
	std::cout << "TCP: File has " <<  file_size << " bytes" << std::endl;
	ssize_t len;
	i = 0;

	int server_amount = storages.size();
	do {
		//Receber do cliente os os bytes necessarios
		len = recv(accept_fd_tcp,file_buffer,128,0);

		//Enviar para cada SS
		int server_id = 0;
		for (int j = 0 ; j < server_amount; j++) {
			// Send to the SS
			send(fd_tcp_ss[j], file_buffer, len, 0);
		}

		i += len;
		remain_data -= len;
		
	} while(len > 0 && (remain_data > 0));
	
	// Send the \n
	char buffers[600][storages.size()] ;
	for (int j = 0 ; j < server_amount; j++) {
		bzero(buffers[i], 600);
		std::string barra_n = "\n";
		send(fd_tcp_ss[j], barra_n.c_str(), barra_n.size(), 0);

		//Now store the response.
		recv(fd_tcp_ss[j], buffers[i], 50, 0);
	}
		

	this->disconnectSS();

	std::cout << "Upload Done!" << std::endl;

	result = "AWC nok";
	strncpy(buffer, result.c_str(), result.size());
	return buffer;
}

void CServer::processTCP() {
	char tcp_buffer[600];
	char *new_filename;
	bzero(tcp_buffer, 600);

	nread_tcp=recv(accept_fd_tcp,tcp_buffer,4,0);
	if(nread_tcp==-1) {
		std::cout << "TCP: recv error: " << strerror(errno) << std::endl;
		return;
	}

	// Processamento dos comandos TCP
	if(strcmp(tcp_buffer, "UPR ") == 0){
		std::cout << "Entrei no processamento de comandos" << std::endl;
		bzero(tcp_buffer,600);
		nread_tcp=recv(accept_fd_tcp,tcp_buffer,30,0);
		std::cout << "nread_tcp com: " << nread_tcp << std::endl;
		if(nread_tcp == -1){
			std::cerr << "Erro no nread_tcp: " << strerror(errno) << std::endl;
			exit(1);
		}
		strip(tcp_buffer);
		std::string filename(tcp_buffer);
		//Process the UPR command
		std::cout << "Vou entrar no UPR" << std::endl;
		char* result = this->UPR_command(filename.c_str());
		strncpy(tcp_buffer, result, 600);
		std::cout << tcp_buffer << std::endl;
		//Send back the answer
		send(accept_fd_tcp,tcp_buffer,sizeof(result),0);
		//if new, process the file upload.
		if(strcmp(tcp_buffer, "AWR new\n") == 0) {
			bzero(tcp_buffer,600);
			nread_tcp=recv(accept_fd_tcp,tcp_buffer,4,0);
			//Process the UPC
			result = this->UPC_command(tcp_buffer, filename.c_str());
			strncpy(tcp_buffer, result, 600); 
		}
	
	} else {
		strncpy(tcp_buffer, "ERR\n\0", 5);
	}

	ret_tcp=send(accept_fd_tcp,tcp_buffer,600,0);
	if(ret_tcp==-1) {
		std::cout << "TCP: sento error: " << strerror(errno) << std::endl;
		return;
	}

	std::cout << "TCP: Response sent: " << tcp_buffer << ";"<<std::endl;
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
		std::cout << "Processo criado" << std::endl;
		if( pid == -1 ) {
			exit(1);
		} else if( pid == 0 ) { 
			this->processTCP();
			std::cout << "TCP: Process done." << std::endl;
			_exit(0);
		}
		//Parent process

		do {
			ret_tcp = close(accept_fd_tcp);
		} while (ret_tcp == -1 && errno==EINTR);

		if(ret_tcp==-1) { std::cerr << "Erro no TCP: " << strerror(errno) << std::endl; return; }
	}

	std::cout << "Sai do ciclo, vai dar merda" << std::endl;
}

void CServer::retrieveStorage() {

	//Criar directório para o Central Server
	std::cout << "Abrimos o ficheiro" << std::endl;
	std::ifstream input( "serverlist.txt" );
	if( !input.good() ) {
		std::cerr << "ERRO: Impossível ler ficheiro com a lista de servidores." << std::endl;
		return;
	}
	std::string line;
	std::cout << "Abrimos o ficheiro" << std::endl;

	while( std::getline(input, line, '\n') ) {
		std::cout << "Abrimos o ficheiro" << std::endl;

		std::vector< std::string > server2 = split(line, ' ');
		storages.push_back(server2);
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

void CServer::close_all() {
	if(fd_tcp_ss != NULL)
		this->disconnectSS();
	close(fd_tcp);
	close(fd_udp);
}
