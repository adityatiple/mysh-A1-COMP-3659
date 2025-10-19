#ifndef MY_STRING_H
#define MY_STRING_H

/*
mystrlen: Function calculates the length of a string by counting the number
          of characters before the terminating null ('\0') character.

 @param str: Pointer to the input string whose length will be measured.

 @return The number of characters in the string (excluding the null terminator).
*/
int mystrlen(const char *str);

/*
mystrcmp: Function compares two strings lexicographically (character by character). 
          It iterates through both strings simultaneously until a differing character 
          is found or until the end of either string ('\0') is reached. 

          The comparison is based on the ASCII values of the characters at the point 
          of difference. This behavior matches that of the standard strcmp() function.

 @param str1: Pointer to the first null-terminated string.
 @param str2: Pointer to the second null-terminated string.

 @return  0  if both strings are identical,
         <0 if str1 is lexicographically less than str2,
         >0 if str1 is lexicographically greater than str2.
*/
int mystrcmp(const char *str1, const char *str2);

/*
mystrcpy: Function copies the contents of a source string into a destination buffer.
          Each character from 'src' is copied sequentially into 'dest' until the
          null terminator ('\0') is reached. The destination string is null-terminated
          after copying is complete.

 @param dest: Pointer to the destination buffer where the string will be copied.
 @param src:  Pointer to the source null-terminated string to be copied.

 @return A pointer to the destination buffer (dest).
*/
char *mystrcpy(char *dest, const char *src);

/*
mystrdup: Function duplicates a substring from a given range of memory defined by
          'begin' and 'end'. It dynamically allocates a new buffer large enough
          to hold the copied characters plus a null terminator ('\0').

          The characters between 'begin' (inclusive) and 'end' (exclusive)
          are copied into the newly allocated buffer, and a null terminator
          is appended at the end to form a valid C-string.

 @param begin: Pointer to the start of the substring to duplicate.
 @param end:   Pointer to one position past the last character to copy.

 @return Pointer to the newly allocated null-terminated string,
         or NULL if memory allocation fails.
*/
char *mystrdup(const char *begin, const char *end);

/*
mystrcat: Function appends the contents of the source string ('src') to the end of 
          the destination string ('dest'). It first finds the length of 'dest' using 
          mystrlen(), then copies each character from 'src' starting at that position. 
          A null terminator ('\0') is added at the end to complete the new string.

 @param dest: Pointer to the destination string where 'src' will be appended.
 @param src:  Pointer to the source null-terminated string to append.

 @return Pointer to the destination string containing the concatenated result.
*/
char *mystrcat(char *dest, const char *src);

#endif
