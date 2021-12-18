#include<iostream>
#include<sys/socket.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include <time.h>
#include <signal.h>
#include<pthread.h>
#include<fstream>
using namespace std;

void signalHandler(int signalNumber);
void *terminateProgram(void *args);

const int BUFFER_SIZE = 50;
const int PORT_NUMBER = 8081;
int processNumber;
int socketfd;
int main(int argc,char* argv[]){

	if(argc < 1){
    printf("Please enter the process number and restart again\n");
    exit(1);
  }

	processNumber = atoi(argv[1]);
	char buffer[BUFFER_SIZE] = {0};

	struct sockaddr_in serverAddress;
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	int valueRead;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUMBER);
	inet_pton(AF_INET,"127.0.0.1",&serverAddress.sin_addr);
	//Connects to the server
	connect(socketfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));

	pthread_t terminateThread;
	pthread_create(&terminateThread,NULL,&terminateProgram,NULL);

	int count = 0;
	printf("Process %d - Process coming up...\n",processNumber);
	while(true){
		//write r to request access to device to mediator
		string c = "";
		c.append(std::to_string(processNumber));
		c.append("#A");
		send(socketfd,c.c_str(),c.size(),0);

		printf("Process %d - Sent message to mediator for device access \n",processNumber);
		//read message from mediator
		valueRead = read(socketfd,buffer,BUFFER_SIZE);
		printf("Process %d - Received from server %s \n",processNumber,buffer);
		bzero(buffer,BUFFER_SIZE);
		//use device for sometime
		time_t t;
		srand((unsigned) time(&t));
		int sleeptime = 0;
		while(sleeptime == 0){
			sleeptime = rand() % 5;
		}
		// printf("Process %d - Using Device for %d seconds\n",processNumber,sleeptime);
		/*
			Opening file, incrementing the counter, closing it
			and again reopening to see if any other process has changes the value.
		*/
		fstream fp("shared_file.txt");
		int fileVal;
		fp >> fileVal;
		int tmp;
		fp.clear();
		fp.seekg(0,ios::beg);
		fp << ++fileVal;
		fp.close();
		fstream fd("shared_file.txt");
		fd >> tmp;
		fd.close();
		printf("Process %d - Updated Value %d and  Final Read Value = %d\n", processNumber, fileVal, tmp);
		sleep(sleeptime);

		//write d to send message to mediator that use is over
		//Sending release request to mediator
		string d = "";
		d.append(std::to_string(processNumber));
		d.append("#R");
		send(socketfd,d.c_str(),d.size(),0);
		printf("Process %d - Sent message to mediator for device release \n",processNumber);
		read(socketfd,buffer,BUFFER_SIZE);
		printf("Process %d - Received from mediator :  %s \n",processNumber,buffer);
		bzero(buffer,BUFFER_SIZE);
		count++;

	}
	close(socketfd);

	return 0;
}

void signalHandler(int signalNumber){
  cout << "\n Terminating the program \n" << endl;
  //closeSocketDescriptor
  if(close(socketfd) != 0){
    printf("Socket could not be closed properly\n");
  }else{
    printf("Socket closed Successfully\n");
  }
  exit(signalNumber);
}
void *terminateProgram(void *args){
  signal(SIGINT, signalHandler);
  return NULL;
}
