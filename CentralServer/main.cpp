#include "CServer.h"
#include <istream>


int main(int argc, char *argv[]){
	
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
	
	CServer *cserver = new CServer(cs_port);
	cserver->startListening();
	
	
	

}

	
