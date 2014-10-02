#include "Client.h"
#include <istream>



int main(int argc, char *argv[]){
    
    char *host_name;
    int cs_port;
    std::string input;
   
    if(argc == 1){
        host_name = (char*) malloc(sizeof(char) * 128);
        gethostname(host_name, 128);
        //std::cout << (host_name);
        
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
                    cs_port = atoi(argv[++i]);
                }else{
                    std::cerr << "erro" << std::endl;
                    return 1;
                }
                
            }
            
        }

    }
    
    //std::cout << host_name << ":"  << csport << std::endl;
    
    //Fazer a ligação ao servidor
    Client *client = new Client(host_name, cs_port);
    
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
			}
			else if(in.front() == "upload") {
				if(in.size() == 1){
					std::cerr << "File name missing" << std::endl;
					return 1;
				}
				std::cout << "Uploading...." << std::endl;
				//std::cout << "File name found: " << in.back() << std::endl;
				client->upload(in.back());
				}
				else if(in.front() == "exit") {
					std::cout << "Bye!" << std::endl;
					exit(1);
        }
        std::cout << "bwsh > ";
    }
    
}
