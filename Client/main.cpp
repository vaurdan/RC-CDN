#include "Client.h"
#include <istream>
#include <signal.h>

Client *client;

void my_handler(int s){
    std::cout << "\nCTRL+C detectado. Encerrando as ligações..." << std::endl;
    client->close_all();
    delete(client);
    exit(1); 

}

int main(int argc, char *argv[]){
    
    char *host_name = "localhost";
    char *cs_port;
    std::string input;
    
    //Registo dos signals
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    signal (SIGINT,my_handler);

    void(*old_handler)(int);
    if((old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR)
        exit(1);
    if(argc == 1){
        host_name = (char*) malloc(sizeof(char) * 128);
        gethostname(host_name, 128);
        cs_port = (char*) malloc( sizeof(char) * 10);
        cs_port = PORT;
        std::cout << (cs_port) << std::endl;
        
    }else{
        for(int i = 1; i < argc; i++){
            
            if(std::string(argv[i]) == "-n"){
                if(i+1 < argc){
                    host_name = argv[++i];
                }else{
                    std::cerr << "erro" << std::endl;
                    return 1;
                }
            }else if(std::string(argv[i]) == "-p"){
                if(i+1 < argc){
                    cs_port = argv[++i];
                }else{
                    std::cerr << "erro" << std::endl;
                    return 1;
                }
                
            }
            
        }

    }
    
    //std::cout << host_name << ":"  << csport << std::endl;
    
    //Fazer a ligação ao servidor pelo cliente
    client = new Client(host_name, cs_port);


    std::cout << "bwsh > ";
    while(std::getline(std::cin, input)){
		std::vector<std::string> in = client->split(input,' ');
        if(in.front() == "list") {
			std::cout << "Listing...." << std::endl;
            client->list();
        } else if(in.front() == "retrieve") {
			if(in.size() == 1){
				std::cerr << "File name missing" << std::endl;
				return 1;
			}
			std::cout << "Retrieving...." << std::endl;
			//std::cout << "File name found: " << in.back() << std::endl;
			client->retrieve(in.back());
		} else if(in.front() == "upload") {
			if(in.size() == 1){
				std::cerr << "File name missing" << std::endl;
				return 1;
			}
			std::cout << "Uploading...." << std::endl;
			//std::cout << "File name found: " << in.back() << std::endl;
			client->upload(in.back());
		} else if(in.front() == "exit") {
				std::cout << "Bye!" << std::endl;
				exit(1);
        	}else{
			std::cout << "Comando mal formulado, por favor tente de novo!" << std::endl;
			return 1;
        	}
       		 std::cout << "bwsh > ";
    	}




    std::cout << "Fim do programa. " << std::endl;

    return 0;
    
}


