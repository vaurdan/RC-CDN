#include "CSInstance.h"

void CSInstance::test() {
	std::cout << getTypeName() << ": Waiting for connections..." << std::endl;
	addrlen=sizeof(addr);
	nread=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
	if(nread==-1){
		std::cout << getTypeName() << ": Error - " << strerror(errno) << std::endl;
		return;
	}
	if(buffer == "test\n"){
		std::cout << getTypeName() << ": Test command recieved." << std::endl;
	}

	ret=sendto(fd,buffer,nread,0,(struct sockaddr*)&addr, addrlen);
	if(ret==-1){
		std::cout << getTypeName() << ": Error - " << strerror(errno) << std::endl;
		return;
	}
	std::cout << getTypeName() << ": Response sent." << std::endl;
		
}

std::string CSInstance::getTypeName() {
	if(type==0)
		return std::string("UDP");
	return std::string("TCP");
}