#include "mystring.h"
#include <string.h>       /* TO DO: initial cheat! Remove this line and all library dependency evetually */
#include <unistd.h>      /* for read() and write() */

/*
Your shell will require a small string library for performing a modest set of basic string operations,
such as comparing strings for equality, copying strings, etc. Identify and develop these as needed.
*/


/**
Function returns the length of a string.
**/
// still need to test
unsigned int mystrlen(const char *s)
{
  unsigned int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;  
}

/**
Function compares two strings for equality. 
**/
// still need to test
int mystrcmp(const char *s1, const char *s2)
{
  while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0') { // while characters are equal and not null terminator
    s1++;
    s2++;
  }
   return (unsigned char)*s1 - (unsigned char)*s2;
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
  //return strcpy(dest, src);
}
