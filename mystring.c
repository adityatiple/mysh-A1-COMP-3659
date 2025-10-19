#include "mystring.h"
#include "myheap.h"
#include <unistd.h>      


int mystrlen(const char *s)
{
  int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;  
}

int mystrcmp(const char *str1, const char *str2)
{
  while (*str1 == *str2 && *str1 != '\0' && *str2 != '\0') { // while characters are equal and not null terminator
    str1++;
    str2++;
  }   
   return (char)*str1 - (char)*str2; // if 0 returned both are equal, else not.
}

char *mystrcpy(char *dest, const char *src)
{
   char *d = dest;        // keep pointer to start of dest
    while (*src) {         // copy until null terminator
        *d = *src;
        d++;
        src++;
    }
    *d = '\0';             // add the final null terminator
    return dest;           // return original dest pointer
}

char *mystrdup(const char *begin, const char *end) {
    int len = (int)(end - begin);
    char *new_str = alloc(len + 1);

    if (!new_str) {
      return NULL;
    }
    for (int i = 0; i < len; i++) {
      new_str[i] = begin[i];      
    }
    new_str[len] = '\0';    
    return new_str;
}

char *mystrcat(char *dest, const char *src) {
    int dlen = mystrlen(dest);
    int i = 0;   

   for (i; src[i] != '\0'; i++){
    dest[dlen + i] = src[i];
   }
    dest[dlen + i] = '\0';
    return dest;
}
