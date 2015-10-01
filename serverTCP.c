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
	char *tokens = strtok(requestCopy, " \n\r");
	//pares the message
	while(tokens != NULL){
		strArray[i] = malloc(strlen(tokens)+1);
		strcpy(strArray[i],tokens);
		//printf("\n%s",strArray[i]);
		i++;
		tokens = strtok(NULL, " \n\r");
	}
	int numElements =i;
	
	i =0;
	int check =0;
	char fileName[50] =".";
	while(i < numElements){		
		if(strstr(strArray[i],".html") !=NULL || strstr(strArray[i],".jpg") !=NULL || strstr(strArray[i],".gif") !=NULL){
			if(access("./kitten.jpg", F_OK ) != -1 && strcmp(strArray[i],"/kitten.jpg") ==0 ||
			 access("./index.html", F_OK ) != -1  && strcmp(strArray[i],"/index.html") ==0 ||
			 access("./lizards.gif",F_OK) != -1 && strcmp(strArray[i],"/lizards.gif") ==0)
			{
				strcat(fileName,strArray[i]);
    				strcat(responseHTTP," 200 OK");
				check =1;
			} else {
   				strcat(responseHTTP," 404 Not Found");
				check =1;
			}
		}		
		i++;
	}
	if(check == 0)
		strcat(responseHTTP, " 404 Not Found");
	strcat(responseHTTP,"\r\n");
	strcat(responseHTTP,"Date: ");
	
	
	time_t currentTime;
	time(&currentTime);
	char *time =ctime(&currentTime); //adds a newline
	strcat(responseHTTP,time);
	//strcat(responseHTTP,"\r\n");
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
		}else if(strstr(strArray[i],".gif")){
			strcat(responseHTTP,"Content-Type: image/gif");
			strcat(responseHTTP,"\r\n");
			break;	
		}
	i++;
	}
	//strcat(responseHTTP,"\n");
	strcat(responseHTTP,"\r\n");
	nBytes = write(newsock,responseHTTP,strlen(responseHTTP));
	printf("The HTTP response is as follows \n");
	printf("%s",responseHTTP);
	FILE *fname =fopen(fileName,"rb");
	char buffer[256];
	int sent = fread(buffer,1,256,fname);
	while(sent > 0){
		write(newsock,buffer,sent);
		sent = fread(buffer,1,256,fname);
	}
}
/*void respond(int sock, char* message) {
	char* tokens[100];
	char* s = " \r\n";
	char* token = strtok(message, s);
	int i;
	for(i = 0; token != NULL; i++) {
		tokens[i] = token;
		token = strtok(NULL, s);
	}
	
	if(strcmp("GET",(char*)tokens[0]) == -1) {
		char* errmess = "400 BAD REQUEST";
		write(sock, errmess, strlen(errmess)+1); 
	} else if (access(strcat(".",(char*)tokens[1]), F_OK) == -1) {
		char* errmsg = "404 NOT FOUND";
		write(sock, errmsg, strlen(errmsg)+1);
	} else {
		//get the date info
		time_t t;
		time(&t);
		char* time = ctime(&t);
		time = strtok(time, "\n");
		//get the content type
		char *fileInfo[2];
		fileInfo[0] = strtok(tokens[1], ".");
		fileInfo[1] = strtok(NULL, ".");
		//get a pointer to the file
		char fileName[50] = ".";
		strcat(fileName, (char*)tokens[1]);
		FILE* fname = fopen(fileName, "r");
		//get the server's name
		char hostname[128];
		gethostname(hostname, sizeof(hostname));
		//use strcat to make this one long string that we will use for the response
		char* response = (char*)tokens[2]; strcat(response, " 200 OK\r\nDate: ");
		strcat(response,time);
		strcat(response, "\r\nContent-Type: ");
		if (strcmp((char*)fileInfo[1], "html") == 0) {
			strcat(response, "text/html");	
		} else if (strcmp((char*)fileInfo[1], "jpeg") == 0) {
			strcat(response, "image/jpeg");
		} else if (strcmp((char*)fileInfo[1], "gif") == 0) {
			strcat(response, "image/gif");
		}
		strcat(response, "\r\nServer: ");
		strcat(response, hostname);
		strcat(response, "\r\n\r\n");
		//send the response message
		write(sock, response, strlen(response)+1);
		//Send the file
		
		fclose(fname);
	}
}*/
