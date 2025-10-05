#include "mystring.h"
#include "myheap.h"
#include <string.h>       /* TO DO: initial cheat! Remove this line and all library dependency evetually */
#include <unistd.h>      /* for read() and write() */

/*
Your shell will require a small string library for performing a modest set of basic string operations,
such as comparing strings for equality, copying strings, etc. Identify and develop these as needed.
*/


/**
Function returns the length of a string.
**/
int mystrlen(const char *s)
{
  int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;  
}

/**
Function compares two strings for equality. 
**/
int mystrcmp(const char *s1, const char *s2)
{
  while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0') { // while characters are equal and not null terminator
    s1++;
    s2++;
  }
   //return (unsigned char)*s1 - (unsigned char)*s2;
   return (char)*s1 - (char)*s2;
}

/**
Function copies a string from a source to a destination.
**/
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


// duplicates bytes in [begin, end) into arena and NUL-terminates
char *mystrdup(const char *begin, const char *end) {
    int k = (int)(end - begin);
    char *new_str = alloc(k + 1);

    if (!new_str) {
      return NULL;
    }

    for (int i = 0; i < k; i++) {
      new_str[i] = begin[i];      
    }

    new_str[k] = '\0';    
    return new_str;
}
