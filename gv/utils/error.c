#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "lsd.h"
#include "process_arguments.h"


/*----------------------------------------------------------------------------*/
/** Fatal error, print a message to standard-error output and exit.
 */
void error(char * msg)
{
  fprintf(stderr,"%s\n",msg);
  exit(EXIT_FAILURE);
}

