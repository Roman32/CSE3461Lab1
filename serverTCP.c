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

void sendBack(int);
void parseAndSendResponse(int,char []);

int main (int argc, char *argv[])
{
	int sock, newsock,portNum, pid;
	socklen_t clientLen;
	struct sockaddr_in server, client;
	char request[256];

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

	if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		printf("ERROR binding socket!\n");
		exit(0);
	}

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
	close(newsock);
	return 0;
}

//responds back to the client with either original message or special message.
void sendBack(int newsock){

	int nBytes = 0;
	char message[256];
	bzero(message,256);
	nBytes = read(newsock,message,255);
   	if (nBytes < 0) 
		printf("ERROR reading from socket");	
	
   	printf("\nHere is the message: %s\n",message);
   	//nBytes = write(newsock,&message,1024);

	parseAndSendResponse(newsock,message);
	
   	if (nBytes < 0) 
		printf("ERROR writing to socket");
	bzero(message,256);
}

void parseAndSendResponse(int newsock,char request[]){
	char *reqType = "GET";
	int nBytes =0;
	int correct =0;
	int i =0;
	char requestCopy[256];
	char *strArray[10];
	//Copy of string, in case I need the original
	strcpy((char *)requestCopy,(char *)request);
	char *tokens = strtok(requestCopy, " \n \r");
	//pares the message
	while(tokens != NULL){
		strArray[i] = malloc(strlen(tokens)+1);
		strcpy(strArray[i],tokens);
		printf("\n%s",strArray[i]);
		i = i+1;
		tokens = strtok(NULL, " \n\r");
	}
	if(correct != 0){
		printf("Request type is not supported");
	}

}
