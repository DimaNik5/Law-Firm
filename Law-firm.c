#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "TCP/client.c"
#include "display.c"
#include <fcntl.h>

#define SPEC 48
#define initMes(M) M[0] = SPEC; M[1] = 0
#define countSimbol(C, M, S) for(C=0;M[C];M[C] == S ? C++ : *M++)

//#pragma comment(lib,"ws2_32.lib")

// gcc -o Law-firm Law-firm.c -lws2_32 

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
    FILE * file;
    file = fopen("data/fio.txt", "r");
    if (file){
        fgetws(fio, 30, file);
        fclose(file);
    }else{
        wprintf(L"Введите своё ФИО: ");
        wscanf("%s\n", fio);
        FILE * fp;
        if(fp){
            fp = fopen("data/fio.txt", "w+");
            fputws(fio, fp);
            fclose(fp);
        }
    }

    l = 1;
    while(l >= 1){
        client->message[0] = 0;
        wcscat(client->message, way);
        r = send_recv(client);
        if(r){
            printf("Error %d", r);
            getchar();
            exit(r);
        }
        if(wcsstr(client->server_reply, L"GET") != NULL){
            wcscat(client->message, way);
            wcscat(client->message, fio);
            continue;
        }

        _putws(client->server_reply);
        wcscat(client->server_reply, L"Назад.");
        wchar_t* s = client->server_reply;
        int count;
        countSimbol(count, s, L'.');
        int a = menu(client->server_reply);
        if(a >= count) continue;
        if(a == count - 1){
            l--;
            way[l] = 0;
            continue;
        }
        way[l++] = a - 1 + 48;
        way[l] = 0;
    }

    return 0;

}