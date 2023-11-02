#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../crypto.h"

int uuechar_index(int c){
    return c + 32;
}

int uueichar_index(int c){
    return c - 32;
}

int de_uu(char* in2, char* out){
    int len = 0;
    for(int i = 0; in2[i]!='\0'; i++) len++;
    char in[len*2];
    sprintf(in," %s",in2); //padding byte to make things cleaner

    //char out[len];
    int skipped = 0;
    for(int i = 0; i<len; i+=4){
        if((i - skipped*2)%60==0){
            i+=2;
            skipped+=1;
        };

        uint8_t u1 = i>len?0:in[i];
        uint8_t u2 = i+1>len?0:in[i+1];
        uint8_t u3 = i+2>len?0:in[i+2];
        uint8_t u4 = i+3>len?0:in[i+3];
        
        u1 = uueichar_index(u1);
        u2 = uueichar_index(u2);
        u3 = uueichar_index(u3);
        u4 = uueichar_index(u4);
        
        uint8_t left = u1 << 2 | u2 >> 4;
        uint8_t middle = u2 << 4 | u3 >> 2;
        uint8_t right = u3 << 6 | u4;
        if(u1 < 64 && u2 < 64){
            sprintf(out,"%s%c",out,left);
        }
        if(u2 < 64 && u3 < 64){
            sprintf(out,"%s%c",out,middle);
        }
        if(u3 < 64 && u4 < 64){
            sprintf(out,"%s%c",out,right);
        }
        
    }
    return 0;
}
int en_uu(char* in, char* out){
    int len = 0;
    for(int i = 0; in[i]!='\0'; i++) len++;

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
        if(i / 3 * 4 % 60 == 0){
            if(i==0) sprintf(out,"%c",(len - i >= 45) ? 'M':uuechar_index(len - i));
            else sprintf(out,"%s\n%c",out,(len - i >= 45) ? 'M':uuechar_index(len - i));
        }
        sprintf(out,"%s%c%c%c%c",out,uuechar_index(i1),uuechar_index(i2),
                uuechar_index(i3),uuechar_index(i4));
    }
    sprintf(out,"%s\n`",out);
    
    return 0;
}



int l_uuencode(lua_State* L){  
  size_t len;
  const char* _a = lua_tolstring(L, 1, &len);
  char *a = calloc(len, sizeof * a);
  memcpy(a, _a, len);
  
  char* encode = calloc(len * 3,sizeof * encode);
  en_uu(a, encode);
  lua_pushstring(L, encode);

  free(a);
  free(encode);
  return 1;
};

int l_uudecode(lua_State* L){
  size_t len;
  const char* _a = lua_tolstring(L, 1, &len);
  char *a = calloc(len, sizeof * a);
  memcpy(a, _a, len);
  
  char* encode = calloc(len,sizeof * encode);
  de_uu(a, encode);
  lua_pushstring(L, encode);

  free(a);
  free(encode);
  return 1; 
};
