#pragma one

#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <wchar.h>

int menu(wchar_t* m){
    wchar_t t;
    int c = -1;
    do{
        int i = 0;
        int l = 0;
        while(m[i] != L'\0'){
            int j = 0;
            if(l == 0){
                while(m[i + j] != L'.'){
                    wprintf(L"%c", m[i + j++]);
                }
                wprintf(L"%c\n", m[i + j]);
                i += j + 1;
                l++;
                continue;
            }
            if(j = 0){
                wprintf(L"%d. ", l);
            }
            while(m[i + j] != L'.'){
                wprintf(L"%c", m[i + j++]);
            }
            wprintf(L"%c\n", m[i + j]);
            i += j + 1;
            l++;
        }
        wprintf(L"Введите пункт: ");
        while(1){
            if(_kbhit()){
                t = _getwch();
                wprintf(L"%c ", t);
                wprintf(L"%d", t);
                if(t == 13){
                    break;
                }
                if(t > 48 && t < 48 + l){
                    c *= 10;
                    c += t + 48;
                }
                else{
                    c = -1;
                }
            }
        }
        system("cls");
    }while(c <= 0);
    return c;
}