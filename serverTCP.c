#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>


/*Author: Roman Claprood
Project: Minilab
Due: 9/16/15
*/

void sendBack();

int main (int argc, char *argv[])
{
	int sock, newsock,portNum, pid;
	socklen_t clientLen;
	struct sockaddr_in server, client;

	if (argc < 2)
	{
		printf("Error, please give a port!\n");
		exit(1);
	}

	sock = socket(AF_INET,SOCK_STREAM, 0);
	
	if(sock < 0)
		printf("ERROR opening socket!\n");
	//set up for socket to bind
	portNum = atoi(argv[1]);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(portNum);

	if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0)
		printf("ERROR binding socket!\n");

	listen(sock,50);
	clientLen = sizeof(client);

	while(1){
		newsock = accept(sock, (struct sockaddr *) &client, &clientLen);
		if(newsock < 0)
			printf("ERROR accepting!");

		pid = fork();
		if(pid < 0)
			printf("Error forking");

		if(pid == 0){
			close(sock);
			sendBack(newsock);
			exit(0);
		}else{
			close(newsock);
		}
	}
	return 0;
}

//responds back to the client with either original message or special message.
void sendBack(int newsock){

	int nBytes = 0;
	char message[256];
	char special[] = "hello\n"; //Added a newline at the because fgets adds one in the client.
	bzero(message,256);
	nBytes = read(newsock,message,255);
   	if (nBytes < 0) 
		printf("ERROR reading from socket");
	
	int check = strcasecmp(special,message);//Eliminate concern for case.
	
	if (check == 0){
		printf("Here is the message: %s\n",message);
		nBytes = write(newsock,"Welcome!",8);
   	if (nBytes < 0) 
		printf("ERROR writing to socket");
	}else{
   		printf("Here is the message: %s\n",message);
   		nBytes = write(newsock,"Messaged received was: ",23);
		write(newsock,&message,255);
   		if (nBytes < 0) 
			printf("ERROR writing to socket");
	}
}
