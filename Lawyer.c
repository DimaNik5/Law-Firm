#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "TCP/client.c"
#include <fcntl.h>

#define SPEC 49

// gcc -o Lawyer Lawyer.c -lws2_32 

int main(){
    _setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
    CLIENT* client = (CLIENT*)malloc(sizeof(CLIENT));
    int r = init(client, "127.0.0.1");
    if(r){
        printf("Error %d", r);
        getchar();
        exit(r);
    }
    wchar_t way[10];
    int l = 0;
    way[0] = SPEC;
    way[1] = 0;

    wchar_t fio[30];
    fio[0] = 0;
    wprintf(L"Введите своё ФИО: ");
    wscanf(L"%s", fio);

    client->message[0] = 0;
    wcscat(client->message, way);
    wcscat(client->message, fio);
    r = send_recv(client);
    if(r){
        printf("Error %d", r);
        getchar();
        exit(r);
    }
    _putws(client->server_reply);
    getwchar();
    getwchar();

    return 0;

}