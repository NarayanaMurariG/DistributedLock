#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <semaphore.h>
#include <strings.h>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>

using namespace std;


bool isOccupied = false;
pthread_mutex_t mutex;
pthread_cond_t cond;
const int PORT = 8081;
const int BUFFER_SIZE = 50;
int whichProcessIsUsing;
const char *acquire = "A";
const char *release = "R";
void *deviceAllocation(void *args);
time_t currentTime;
int clientCounter = 0;

int main(){
	//Mediator (Server)
	printf("Server Coming up shortly....\n");

	if(pthread_mutex_init(&mutex,NULL) != 0){
		printf("Mutex Lock Failed \n");
		exit(1);
	}

	struct sockaddr_in serverAddress;
	int addresslen = sizeof(serverAddress);
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	int valueRead, opt = 1;
	char buffer[BUFFER_SIZE] = {0};

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if( bind(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == 0){ // returns -1 if failed
		printf("Binded Socket Successfully to the port number\n");
	} else{
		printf("Binding Socket Failed...\n Try After 10-15s \n");
		printf("Sometimes this happens when we stop server and start it immediately\n");
		exit(0);
	}

	if(listen(socketfd,100) < 0){
		printf("Listener failed");
		exit(1);
	}

	/*
		Mediator gets requests from multiple processes and each process is handled
		in a new thread.
	*/
	while(true){
		struct sockaddr_in clientAddress;
		int newSocketClient;
		if( (newSocketClient = accept(socketfd,(struct sockaddr *)&clientAddress,(socklen_t*)&addresslen)) < 0){
			printf("Error while accepting connection");
			exit(1);
		}
		else{
			pthread_t threadId;

			pthread_create(&threadId,NULL,&deviceAllocation,&newSocketClient);
		}
	}


	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	close(socketfd);
	return 0;
}

/*
	If a release request is received, the file is unlocked , conditional signal
	is sent and a new process in conditioanl wait can use it.

	If its a acquire request and it file is being locked by another
	process, the requested process goes into conditional wait queue until
	the process which uses it releases it and sends a signal.
*/
void *deviceAllocation(void *args){
	int threadNumber = ++clientCounter;
	printf("Connected to Client %d\n",threadNumber);

	int commfd = *(int *)args;
	char buffer[BUFFER_SIZE] = {0};
	char *requestFrom;
	char *requestType;
	while(true){
		read(commfd,buffer,BUFFER_SIZE);
		requestFrom = strtok(buffer,"#");
		requestType = strtok(NULL,"#");
		printf("T%d - Request From Process : %s, Request Type : %s \n",threadNumber,requestFrom,requestType);
		//A = Access, R = Release

		pthread_mutex_lock(&mutex);
		if(strcmp(requestType,release) == 0){
			printf("T%d - Releasing Device for %s \n",threadNumber,requestFrom);
			isOccupied = false;
			whichProcessIsUsing = -1;
			printf("T%d - Sending Cond Signal\n",threadNumber);
			pthread_cond_signal(&cond);
			string reply = "Device Access Released";
			write(commfd,reply.c_str(),BUFFER_SIZE);
		}
		else{
			if(isOccupied){
					printf("T%d - Process %d is using it now\n",threadNumber,whichProcessIsUsing);
					pthread_cond_wait(&cond,&mutex);
					printf("T%d - Ending Cond Wait\n",threadNumber);
			}

			isOccupied = true;
			whichProcessIsUsing = std::stoi(requestFrom);
			string reply = "Device Access Granted";
			printf("T%d - Device Access Granted for : %s\n",threadNumber,requestFrom);
			write(commfd,reply.c_str(),BUFFER_SIZE);
		}
		pthread_mutex_unlock(&mutex);
		bzero(buffer,BUFFER_SIZE);
	}
}
