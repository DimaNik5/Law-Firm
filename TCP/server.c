#include <io.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <winsock2.h>
#include <wchar.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"pthread.lib")

#define PORT 8080
#define MAX_LENGTH 512

// gcc server.c -o server -lws2_32 -lpthread

pthread_mutex_t m;

int init(SOCKET* s, WSADATA* wsa, struct sockaddr_in* server);

void* working(void* arg);

int saveNote(wchar_t* m);

int* getFreeLawyer(int* count, int setDate);

void getContent(wchar_t* filename, wchar_t* buff);

int main(int argc , char *argv[])
{
	SOCKET s;
	struct sockaddr_in server;
	WSADATA wsa;
	if(init(&s, &wsa, &server)){
		return 1;
	}
	// Переменная для потоков работы с "клиентами"
	pthread_t t;
    pthread_mutex_init(&m, NULL); // инициализация мьютекса
	//Listen to incoming connections
	listen(s , 3);
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	
	int c = sizeof(struct sockaddr_in);
	struct sockaddr_in client;
	SOCKET new_socket;
	while(1)
	{
		new_socket = accept(s , (struct sockaddr *)&client, &c);
		if (new_socket == INVALID_SOCKET)
		{
			printf("accept failed with error code : %d" , WSAGetLastError());
			break;
		}
		puts("Connection accepted");
    	pthread_create(&t, NULL, working, &new_socket);
	}

	closesocket(s);
	WSACleanup();
	
	return 0;
}

enum TypesOfUsers {
	CLIENT,
	LAWYER,
	ADMIN
};

enum MenusForClient {
	QUESTIONS = 1,
	DATENOTE,
	FREE_LAWYER
};

void* working(void* arg){
	wchar_t message[MAX_LENGTH], temp[MAX_LENGTH], client_reply[MAX_LENGTH], file[30];
	int recv_size;
	while(1){
		if((recv_size = recv(*(SOCKET*)arg, client_reply, MAX_LENGTH , 0)) == SOCKET_ERROR)
		{
			break;
		}
		client_reply[recv_size] = L'\0';

		int type, isDate = 0, isLawyer = 0, isNewNote = 0;
		message[0] = 0;
		if(recv_size == 1){
			type = client_reply[0] - 48;
			continue;
		}
		else{
			switch (type)
			{
			case CLIENT:{
				switch(recv_size - 1){
					case QUESTIONS:
						file[0] = 0;
						wcscat(file, L"data/question.txt");
						break;
					case DATENOTE:
						isDate = 1;
						file[0] = 0;
						break;
					case FREE_LAWYER:
						isLawyer = 1;
						file[0] = 0;
						wcscat(file, L"data/lawyer.txt");
						break;
					default:
						file[0] = 0;
						if(recv_size - 1 < 4) break;
						if(client_reply[QUESTIONS] > 47 && client_reply[QUESTIONS] < 58 && 
								client_reply[DATENOTE] > 47 && client_reply[DATENOTE] < 58 && 
								client_reply[FREE_LAWYER] > 47 && client_reply[FREE_LAWYER] < 58){
							isNewNote = 1;
						}
						break;
				}
				break;
			}
			case LAWYER:
				break;
			case ADMIN:
				break;
			default:
				break;
			}
		}

		if(file[0] != 0){ 
			getContent(file, message);
		}

		if(isDate){
			wcscat(message, L"Дата.");
			wchar_t t[7];
			time_t mytime = time(NULL);
    		struct tm *now = localtime(&mytime);
			int d = now->tm_mday;
			int m = now->tm_mon;
			int max_d = (m > 6 ? (m % 2 ? 31 : 30) : (m % 2 ? (m == 1 ? 28 : 30) : 31));
			for(int i = 0 ; i < 30; i++){
				d++;
				if(d > max_d){
					d = 1;
					m++;
					if(m > 11) m = 0;
					max_d = (m > 6 ? (m % 2 ? 31 : 30) : (m % 2 ? (m == 1 ? 28 : 30) : 31));
				}
				swprintf(t, L"%d/%d.\0", d, m + 1);
				wcscat(message, t);
				
			}
		}
		else if(isLawyer){
			int count = 0;
			wchar_t* s[MAX_LENGTH];
			wcscat(s, message);
			for (int i = 0; s[i]; i++){
				if(s[i] == L'.') count++;
			}
			int* freeLawyer = getFreeLawyer(&count, client_reply[DATENOTE] - 48);
			if(freeLawyer != NULL){
				int cur = 0;
				int real = 0;
				int point = 0;
				for(int k = 0; k < count; k++){
					if(freeLawyer[k] == point){
						s[cur++] = s[real++];
						if(s[real] == L'.'){
							point++;
							continue;
						}
					}
					while (freeLawyer[k] != point)
					{
						if(s[real++] == L'.'){
							point++;
						}
					}
					
				}
				free(freeLawyer);
				message[0] = 0;
				wcscat(message, L"Адвокаты.");
				wcscat(message, s);
			}
			else message[0] = 0;
		}
		else if(isNewNote){
			if(saveNote(client_reply) == 0){
				wcscat(message, L"Успешно.");
			}
			else{
				wcscat(message, L"Неудачно.");
			}
		}

		if( send(*(SOCKET*)arg, message, MAX_LENGTH, 0) < 0)
		{
			break;
		}
	}

	return NULL;
}

int saveNote(wchar_t* input){
	int count = 0;
	wchar_t* s[MAX_LENGTH];
	getContent(L"data/lawyer.txt", s);
	for (int i = 0; s[i]; i++){
		if(s[i] == L'.') count++;
	}
	int* freeLawyer = getFreeLawyer(&count, input[DATENOTE] - 48);
	if(count > input[FREE_LAWYER + 1] - 48){
		pthread_mutex_lock( &m );  
		FILE* fp = _wfopen("data/notes.txt", L"a+");
		fseek(fp, 0, SEEK_END);
		if(fp != NULL)
		{
			wchar_t t[7];
			time_t mytime = time(NULL);
			struct tm *now = localtime(&mytime);
			int d = now->tm_mday + 1;
			int m = now->tm_mon;
			int max_d = (m > 6 ? (m % 2 ? 31 : 30) : (m % 2 ? (m == 1 ? 28 : 30) : 31));
			m++;
			if(d > max_d){
				d = 1;
				m++;
				m = m % 13 + 1;
			}
			swprintf(t, L"%d/%d.\0", d, m + 1);
			fwprintf(fp, L"%s,%d,%s\n", t, &freeLawyer[input[FREE_LAWYER + 1] - 48], (input + FREE_LAWYER + 2));
			fclose(fp);
		}
		pthread_mutex_unlock( &m );
	}

}

int* getFreeLawyer(int* count, int setDate){
	int* res = (int*)malloc(sizeof(int) * *count);
	for(int i = 0; i < *count; i++) res[i] = i;
	wchar_t t[7];
	time_t mytime = time(NULL);
	struct tm *now = localtime(&mytime);
	int d = now->tm_mday + 1;
	int m = now->tm_mon;
	int max_d = (m > 6 ? (m % 2 ? 31 : 30) : (m % 2 ? (m == 1 ? 28 : 30) : 31));
	m++;
	if(d > max_d){
		d = 1;
		m++;
		m = m % 13 + 1;
	}
	swprintf(t, L"%d/%d.\0", d, m + 1);

	wchar_t buff[MAX_LENGTH];
	wchar_t temp[30];
	pthread_mutex_lock( &m );  
	FILE* fp = _wfopen(L"data/notes.txt", L"r");
    if(fp != NULL && buff != NULL)
    {
        while(fgetws(buff, MAX_LENGTH, fp) != NULL){
			if(wcsstr(buff, t)){
				int l;
				swscanf(buff, "%s,%d,%s\n", temp, &l, temp);
				for(int i = 0; i < *count; i++){
					if(res[i] == l){
						res[i] = res[(*count) - 1];
						(*count)--;
					}
				}
			}
		}
		if(*count > 1){
			for(int i = 0; i < *count; i++){
				for(int j = i + 1; j < *count; j++){
					if(res[i] > res[j]){
						int t = res[i];
						res[i] = res[j];
						res[j] = t;
					}
				}
			}
		}
		fclose(fp);
		pthread_mutex_unlock( &m );
		return res;
    }
	pthread_mutex_unlock( &m );
	return NULL;
}

void getContent(wchar_t* filename, wchar_t* buff){
	pthread_mutex_lock( &m );  
	FILE* fp = _wfopen(filename, L"r");
    if(fp != NULL && buff != NULL)
    {
        fgetws(buff, MAX_LENGTH, fp);
		fclose(fp);
    }
	pthread_mutex_unlock( &m );
}

int init(SOCKET* s, WSADATA* wsa, struct sockaddr_in* server){
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2), wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	printf("Initialised.\n");
	
	//Create a socket
	if((*s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
		return 1;
	}
	printf("Socket created.\n");
	
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_port = htons(PORT);
	
	//Bind
    if(bind(*s ,(struct sockaddr *)server , sizeof(*server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d" , WSAGetLastError());
		return 1;
	}
	puts("Bind done");
	return 0;
}
