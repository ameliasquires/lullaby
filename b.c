#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int char_index(int c){
    if(c <= 25) return 65 + c;
    if(c <= 51) return 71 + c;
    if(c <= 61) return (c - 52)+48;
    if(c == 62) return 43;
    if(c == 63) return 47;
    return 61;
}

int ichar_index(int c){
    if(65 <= c && c <= 90) return c - 65;
    if(97 <= c && c <= 122) return c - 97 + 26;
    if(48 <= c && c <= 57) return c - 48 + 52;
    if(c == 47) return 63;
    return 0;
}

int de_base64(char* in, char* out){
    int len = 0;
    for(int i = 0; in[i]!='\0'; i++) len++;
    
    //char out[len];
    for(int i = 0; i < len; i+=4){
        uint8_t u1 = i>len?0:in[i];
        uint8_t u2 = i+1>len?0:in[i+1];
        uint8_t u3 = i+2>len?0:in[i+2];
        uint8_t u4 = i+3>len?0:in[i+3];
        
        u1 = ichar_index(u1);
        u2 = ichar_index(u2);
        u3 = ichar_index(u3);
        u4 = ichar_index(u4);
        
        uint8_t left = u1 << 2 | u2 >> 4;
        uint8_t middle = u2 << 4 | u3 >> 2;
        uint8_t right = u3 << 6 | u4;
        
        if(u4==0) sprintf(out,"%s%c%c",out,left,middle);
        else if(u3==0) sprintf(out,"%s%c",out,left);
        else sprintf(out,"%s%c%c%c",out,left,middle,right);
        
    }
    return 0;
}
int en_base64(char* in, char* out){
    int len = 0;
    for(int i = 0; in[i]!='\0'; i++) len++;

    //char out[(len+1)*3];
    for(int i = 0; i < len; i+=3){
        uint8_t f = i>len?0:in[i];
        uint8_t s = i+1>len?0:in[i+1];
        uint8_t t = i+2>len?0:in[i+2];
        
        uint8_t i1 = f>>2;
        uint8_t i2 = (uint8_t)(f<<6)>>2 | (s>>4);
        uint8_t i3 = (uint8_t)(s<<4)>>2 | (t>>6);
        uint8_t i4 = t & 0x3f;
        
        if(t==0)i4 = 64;
        if(s==0)i3 = 64;
        sprintf(out,"%s%c%c%c%c",out,char_index(i1),char_index(i2),
                char_index(i3),char_index(i4));
    }
    return 0;
}

int main(){
    char* uwu = "Many hands make light work.AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    char uwue[20*8000];
    char uwud[20*8000];
    
    en_base64(uwu, uwue);
    de_base64(uwue, uwud);
    
    printf("%s\n%s\n%s",uwu,uwue,uwud);
}


