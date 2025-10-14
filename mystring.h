#ifndef MY_STRING_H
#define MY_STRING_H

int mystrlen(const char *s);
int mystrcmp(const char *s1, const char *s2);
char *mystrcpy(char *dest, const char *src);
char *mystrdup(const char *begin, const char *end);
char *mystrcat(char *dest, const char *src);

#endif
