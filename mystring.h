#ifndef MY_STRING_H
#define MY_STRING_H

/*
mystrlen: Function calculates the length of a string by counting the number
          of characters before the terminating null ('\0') character.

 @param str: Pointer to the input string whose length will be measured.

 @return The number of characters in the string (excluding the null terminator).
*/
int mystrlen(const char *str);
int mystrcmp(const char *s1, const char *s2);
char *mystrcpy(char *dest, const char *src);
char *mystrdup(const char *begin, const char *end);
char *mystrcat(char *dest, const char *src);

#endif
