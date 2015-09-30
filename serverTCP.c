#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


/*Author: Roman Claprood and Eric Olson
Project: Lab1
Due: 10/02/15
*/

void sendBack(int);
void parseAndSendResponse(int,char []);

int main (int argc, char *argv[])
{
	int sock, newsock,portNum, pid;
	socklen_t clientLen;
	struct sockaddr_in server, client;
	char request[1024];

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
	close(sock);
	return 0;
}

//responds back to the client with either original message or special message.
void sendBack(int newsock){

	int nBytes = 0;
	char message[1024];
	bzero(message,1024);
	nBytes = read(newsock,message,1023);
   	if (nBytes < 0) 
		printf("ERROR reading from socket");	
	
   	printf("\nHere is the message: %s\n",message);
   	//nBytes = write(newsock,&message,1024);

	parseAndSendResponse(newsock,message);
	
   	if (nBytes < 0) 
		printf("ERROR writing to socket");
	bzero(message,1023);
	close(newsock);
}

void parseAndSendResponse(int newsock,char request[]){
	int nBytes =0;
	int correct =0;
	int i =0;
	char requestCopy[1024];
	char *strArray[100];
	char responseHTTP[5000] ="HTTP/1.1";
	
	//FILE *fname = "/index.html";
	//Copy of string, in case I need the original
	strcpy((char *)requestCopy,(char *)request);
	char *tokens = strtok(requestCopy, " ,;\n\r");
	//pares the message
	while(tokens != NULL){
		strArray[i] = malloc(strlen(tokens)+1);
		strcpy(strArray[i],tokens);
		//printf("\n%s",strArray[i]);
		i++;
		tokens = strtok(NULL, " ,;\n\r");
	}
	int numElements =i;
	
	i =0;
	int check =0;
	while(i < numElements){		
		if(strstr(strArray[i],".html") !=NULL || strstr(strArray[i],".jpg") !=NULL || strstr(strArray[i],".gif") !=NULL){
			if(access("./kitten.jpg", F_OK ) != -1 && strcmp(strArray[i],"/kitten.jpg") ==0 ||
			 access("./index.html", F_OK ) != -1  && strcmp(strArray[i],"/index.html") ==0)
			{
    				strcat(responseHTTP," 200 OK");
				check =1;
			} else {
   				strcat(responseHTTP," 404 NOT FOUND");
				check =1;
			}
		}		
		i++;
	}
	if(check == 0)
		strcat(responseHTTP, " 404 NOT FOUND");
	strcat(responseHTTP,"\r\n");
	strcat(responseHTTP,"Date: ");
	
	//strcat(responseHTTP,"\r\n");
	time_t currentTime;
	time(&currentTime);
	char *time =ctime(&currentTime); //adds a newline
	strcat(responseHTTP,time);
	i =0;
	while(i < numElements){
		if(strstr(strArray[i],".html") !=NULL){
			strcat(responseHTTP,"Content-Type: text/html");
			strcat(responseHTTP,"\r\n");
			break;
		}else if(strstr(strArray[i],".jpg")){
			strcat(responseHTTP,"Content-Type: image/jpeg");
			strcat(responseHTTP,"\r\n");
			break;
		}else if(strstr(strArray[i], ".gif")){
			strcat(responseHTTP,"Content-Type: image/gif");
			strcat(responseHTTP,"\r\n");
			break;	
		}
	i++;
	}
	//strcat(responseHTTP,"\n");
	strcat(responseHTTP,"\r\n");
	//nBytes = write(newsock,&responseHTTP,5000);
	
	printf("%s",responseHTTP);
	printf("data should be here");
}
