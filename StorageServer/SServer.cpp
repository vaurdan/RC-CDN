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

#include "SServer.h"

//conexão TCP com user e CS
void SServer::TCP_Connection_test(){
	
		if((fd=socket(AF_INET,SOCK_STREAM,0))==-1)
			exit(1);
		
		memset((void*)&addr,(int)'\0', sizeof(addr));
		addr.sin_family=AF_INET;
		addr.sin_addr.s_addr=htonl(INADDR_ANY);
		addr.sin_port=htons(59000);
		if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1)
			exit(1);
		
		if(listen(fd,5)==-1)
			exit(1);
			
		while(1){
			
			addrlen=sizeof(addr);
			if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen))==-1)
				exit(1);
			while((n=read(newfd,buffer,128))!=0){
				if(n==-1)
					exit(1);
				ptr=&buffer[0];
					while(n>0){
						if((nw=write(newfd,ptr,n))<=0)
							exit(1);
						n -= nw;
						ptr += nw;
						
					}
			}
			
			close(newfd);
			
		}

		
	}





