#include "SServer.h"
#include <istream>
#include <signal.h>

int main(int argc, char *argv[]){
	
	char *ss_port;
	std::string input;
	ss_port = (char*) malloc( sizeof(char) * 10);
    ss_port = PORT;

 	void(*old_handler)(int);
    if((old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR)
        exit(1);
	
	for(int i = 1; i < argc; i++){

		if(std::string(argv[i]) == "-p"){
			if(i+1 < argc){
				ss_port = argv[++i];
			}else{
				std::cerr << "erro" << std::endl;
				return 1;
			}
		}

	}
	SServer *sserver = new SServer(ss_port);
	sserver->startListening();
		
}
