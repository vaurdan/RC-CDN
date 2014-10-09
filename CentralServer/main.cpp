#include "CServer.h"
#include <istream>
#include <signal.h>

CServer *cserver;

void my_handler(int s){
    cserver->close_all();
    if(cserver != NULL)
    	delete(cserver);
    exit(1); 

}

int main(int argc, char *argv[]){
	
	//Registo dos signals
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    signal (SIGINT,my_handler);

	char *cs_port;
	std::string input;
	cs_port = (char*) malloc( sizeof(char) * 10);
    cs_port = PORT;
	
	for(int i = 1; i < argc; i++){

		if(std::string(argv[i]) == "-p"){
			if(i+1 < argc){
				cs_port = argv[++i];
			}else{
				std::cerr << "erro" << std::endl;
				return 1;
			}
		}

	}
	
	cserver = new CServer(cs_port);
	cserver->startListening();
	
	
	

}

	
