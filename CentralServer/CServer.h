#ifndef Projecto_RC_CServer_h
#define Projecto_RC_CServer_h

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

#define PORT "58021"

class CServer {
	
	private:
    
    //definição das variaveis so para testar agora se esta a ser feita ligação
    pid_t pid,pid_udp;

    char *cs_port;
    char buffer[600];

    // Ligação UDP
    int fd_udp, accept_fd_udp, ret_udp, nread_udp;

    socklen_t addrlen_udp;
    struct sockaddr_in addr_udp;
    
	// Ligação TCP
    int fd_tcp, accept_fd_tcp, ret_tcp, nread_tcp;

    socklen_t addrlen_tcp;
    struct sockaddr_in addr_tcp;


    int *fd_tcp_ss;
    socklen_t *addrlen_tcp_ss;
    struct sockaddr_in *addr_tcp_ss;

    public:
    //Servidores de Storage
    std::vector< std::vector<std::string> > storages;
    std::vector< std::string > *file_list;
	
	public:
	
	CServer(char *port) : cs_port(port) {
        file_list = new std::vector<std::string>();
    };
   
    void startListening();

	void testConnection();

    void retrieveStorage();
    void connectSS();
    void disconnectSS();

    void close_all();

    void retrieveFiles();
    void updateFiles();
    void addFileToList(std::string filename);
	
	private:

    void initUDP();
    void processUDP();

    void initTCP();
    void processTCP();

    void list_command();
    char* UPR_command( const char* filename );
    char* UPC_command( char* buffer, const char* new_filename);

    bool connectTCP(int i, std::string server, std::string port);

    void strip(char *s);

    std::vector<std::string> split(const std::string &s, char delim);
    std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
    template <typename T>
    std::string to_string(T value);


};


#endif

