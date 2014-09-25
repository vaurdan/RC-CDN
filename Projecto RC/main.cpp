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
    client->connect();
    
    std::cout << "bwsh > ";
    while(std::getline(std::cin, input)){
        if(input == "list") {
            client->list();
        } else if(input == "exit") {
            std::cout << "Bye!" << std::endl;
            exit(1);
        }
        std::cout << "bwsh > ";
    }
    
}
