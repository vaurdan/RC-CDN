#include "CServer.h"
#include <istream>


int main(int argc, char *argv[]){
	
	char *cs_port;

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
	
	
	
	

}

	
