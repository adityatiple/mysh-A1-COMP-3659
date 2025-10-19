#include "mystring.h"
#include "myheap.h"
#include <unistd.h>      


int mystrlen(const char *s)
{
    int len = 0;
    while (s[len] != '\0') {
        len++;                // increment till null is found
    }
    return len;               // return total 
}

int mystrcmp(const char *str1, const char *str2)
{
  while (*str1 == *str2 && *str1 != '\0' && *str2 != '\0') { // while characters are equal and not null terminator
    str1++;
    str2++;
  }   
   return (char)*str1 - (char)*str2; // if lexicographically 0 returned both are equal, else not.
}

char *mystrcpy(char *dest, const char *src)
{
   char *d = dest;        // keep pointer to start of dest
    while (*src) {         // copy until null terminator
        *d = *src;
        d++;
        src++;
    }
    *d = '\0';             // add the null terminator
    return dest;           // return original dest pointer
}

char *mystrdup(const char *begin, const char *end) {
    int len = (int)(end - begin);
    char *new_str = alloc(len + 1);   // dynamically allocate enough memory of the new_str len + '\0'

    if (!new_str) {
      return NULL;
    }
    for (int i = 0; i < len; i++) {   // duplicate chars from substring range
      new_str[i] = begin[i];      
    }
    new_str[len] = '\0';              // add the null terminator
    return new_str;                   // return new duplicated string
}

char *mystrcat(char *dest, const char *src) {
    int d_len = mystrlen(dest);     
    int i = 0;   

    for (i; src[i] != '\0'; i++){
      dest[d_len + i] = src[i];     // start appending src string on dest's '\0' index 
    }
    dest[d_len + i] = '\0';         // add the null terminator 
    return dest;                    // retrun concatenated string 
}
