#ifndef JOBS_H
#define JOBS_H

#define MAX_ARGS 2     /* TO DO */
#define MAX_CH 15     /* TO DO */

struct Command
{
  char *argv[MAX_ARGS+1]; // argument vector - array of argument strings
  unsigned int argc;      // argument count - number of arguments
  int background;          // '&' flag : 1 = background (no wait-time prompt user instantly), 
  /* later, you may need to record other information here */
};

#endif
