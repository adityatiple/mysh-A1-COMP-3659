#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 16     /* TO DO */
#define MAX_CH 256     /* TO DO */

struct Command
{
  char *argv[MAX_ARGS+1]; // argument vector - array of argument strings
  unsigned int argc;      // argument count - number of arguments
  int background;          // '&' flag : background = 1 (no wait-time prompt user instantly) 
};

#endif
