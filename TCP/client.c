#pragma one

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define PORT 8080
#define MAX_LENGTH 512

struct Client{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	wchar_t message[MAX_LENGTH] , server_reply[MAX_LENGTH];
	int recv_size;
};

typedef struct Client CLIENT;

enum Errors {
	OK,
	ERROR_WSA,
	ERROR_SOCKET,
	ERROR_CONNECT,
	ERROR_SEND,
	ERROR_RECV
};

int init(CLIENT* client, char* IP){
	if (WSAStartup(MAKEWORD(2,2),&(client->wsa)) != 0)
	{
		return ERROR_WSA;
	}
	
	//Create a socket
	if((client->s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		return ERROR_SOCKET;
	}
	
	client->server.sin_addr.s_addr = inet_addr(IP);
	client->server.sin_family = AF_INET;
	client->server.sin_port = htons(PORT);

	//Connect to remote server
	if (connect(client->s , (struct sockaddr *)&(client->server) , sizeof(client->server)) < 0)
	{
		return ERROR_CONNECT;
	}
	return OK;
}

int send_recv(CLIENT* client){
	//Send some data
	if(send(client->s , client->message , MAX_LENGTH , 0) < 0)
	{
		return ERROR_SEND;
	}
	
	//Receive a reply from the server
	if((client->recv_size = recv(client->s , client->server_reply , MAX_LENGTH , 0)) == SOCKET_ERROR)
	{
		return ERROR_RECV;
	}
	return OK;
}
