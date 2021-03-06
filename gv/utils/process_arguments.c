#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "lsd.h"
#include "process_arguments.h"

#ifndef FALSE
#define FALSE 0
#endif /* !FALSE */

#ifndef TRUE
#define TRUE 1
#endif /* !TRUE */

void error(char * msg);

/*----------------------------------------------------------------------------*/
/*--------------------- Command Line interface handling ----------------------*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/** Free an 'arguments' structure.
 */
static void free_arguments(struct arguments * arg)
{
  if( arg == NULL || arg->args == NULL )
    error("Error: NULL pointer at 'free_arguments'.");
  free( (void *) arg->args );
  free( (void *) arg );
}

/*----------------------------------------------------------------------------*/
/** Accepted characters in field identifier: numbers, letters and '_'.
 */
static int is_id_char(int c)
{
  return c=='_' || isalpha(c) || isdigit(c);
}

/*----------------------------------------------------------------------------*/
/** Read next field definition in an argument definition.
 */
static char * get_next_field(char * p, char * id, char * value)
{
  int n;

  /* check input */
  if( p == NULL || id == NULL || value == NULL )
    error("Error: invalid input to 'get_next_field'.");

  /* search for field id */
  while( isspace(*p) ) ++p; /* skip spaces */
  if( *p != '#' ) error("Error: missing '#' in 'use description'.");
  ++p;
  for( n=0; is_id_char(*p) && n<FIELD_LENGTH; n++ ) id[n] = *(p++);
  if( n >= FIELD_LENGTH ) error("Error: field too long in 'use description'.");
  id[n] = '\0';
  if( *(p++) != ':' ) error("Error: missing ':' in 'use description'.");

  /* search for field value */
  while( isspace(*p) ) ++p; /* skip spaces */
  for( n=0; *p != '#' && *p != '\0' && n<FIELD_LENGTH; n++ ) value[n] = *(p++);
  if( n >= FIELD_LENGTH ) error("Error: field too long in 'use description'.");
  value[n] = '\0';

  /* remove spaces at the end of the field */
  while( --n >= 0 && isspace(value[n]) ) value[n] = '\0';

  return p;
}

/*----------------------------------------------------------------------------*/
/** Read next token in an argument definition.
 */
static char * get_next_token(char * p, char div, char * value)
{
  int n;

  /* check input */
  if( p == NULL || value == NULL )
    error("Error: invalid input to 'get_next_token'.");

  if( *p == '\0' )
    error("Error: argument token expected in 'use description'.");

  while( isspace(*p) ) ++p; /* skip spaces */
  for( n=0; *p!=div && *p!='\0' && n<FIELD_LENGTH; n++) value[n] = *(p++);
  if( n >= FIELD_LENGTH ) error("Error: field too long in 'use description'.");
  value[n] = '\0';
  while( --n >= 0 && isspace(value[n]) ) value[n] = '\0';

  /* remove 'div' at the end of the token, if present */
  if( *p == div ) ++p;

  return p;
}

/*----------------------------------------------------------------------------*/
/** Process one argument description.
 */
static void process_new_argument(char * id, char * value,struct arguments * arg)
{
  char token[FIELD_LENGTH];
  char * p;
  int i;

  /* check input */
  if( id == NULL || value == NULL || arg == NULL )
    error("Error: invalid input to 'process_new_argument'.");

  /* allocate memory if needed */
  if( arg->arg_num >= arg->arg_allocated )
    {
      arg->arg_allocated *= 2;
      arg->args = (struct argument *) realloc( (void *) arg->args,
                                 arg->arg_allocated * sizeof(struct argument) );
      if( arg->args == NULL ) error("Error: not enough memory.");
    }

  /* argument name */
  p = get_next_token(value,'|',arg->args[arg->arg_num].name);
  for( i=0; i<arg->arg_num; i++ )
    if( strcmp(arg->args[i].name,arg->args[arg->arg_num].name) == 0 )
      error("Error: argument name used twice in 'use description'.");

  /* 'option' letter - to be used with '-' to identify option */
  p = get_next_token(p,'|',token);
  if( strcmp(id,"opt") == 0 )
    {
      arg->args[arg->arg_num].required = FALSE;
      if( strlen(token) != 1 )
        error("Error: invalid option letter in 'use description'.");
      arg->args[arg->arg_num].id = token[0];
      if( !isalpha(arg->args[arg->arg_num].id) )
        error("Error: option id must be a letter in 'use description'.");
      for( i=0; i<arg->arg_num; i++ )
        if( !(arg->args[i].required) &&
            arg->args[i].id == arg->args[arg->arg_num].id )
          error("Error: option letter used twice in 'use description'.");
    }
  else /* must be 'req' - required argument */
    {
      arg->args[arg->arg_num].required = TRUE;
      if( strlen(token) > 0 )
        error("Error: unused option letter in 'use description'.");
      arg->args[arg->arg_num].id = 0;
    }

  /* argument type */
  p = get_next_token(p,'|',token);
  if( strcmp(token,"int") == 0 )         arg->args[arg->arg_num].type = 'i';
  else if( strcmp(token,"double") == 0 ) arg->args[arg->arg_num].type = 'd';
  else if( strcmp(token,"str") == 0 )    arg->args[arg->arg_num].type = 's';
  else if( strcmp(token,"bool") == 0 )   arg->args[arg->arg_num].type = 'b';
  else error("Error: unknown argument type in 'use description'.");

  /* required arguments can't be boolean */
  if( arg->args[arg->arg_num].required && arg->args[arg->arg_num].type == 'b' )
    error("Error: required arguments can't be boolean in 'use description'.");

  /* default value */
  p = get_next_token(p,'|',token);
  if( strlen(token) > 0 )
    {
      if( arg->args[arg->arg_num].required )
       error("Error: default value in required argument in 'use description'.");
      arg->args[arg->arg_num].def_value = TRUE;
      arg->args[arg->arg_num].assigned = TRUE;
      strcpy(arg->args[arg->arg_num].d_value,token);
      strcpy(arg->args[arg->arg_num].s_value,token);
      if( arg->args[arg->arg_num].type == 'i' )
        arg->args[arg->arg_num].i_value = atoi(token);
      if( arg->args[arg->arg_num].type == 'd' )
        arg->args[arg->arg_num].f_value = atof(token);
    }
  else
    {
      arg->args[arg->arg_num].def_value = FALSE;
      arg->args[arg->arg_num].s_value[0] = '\0';
      arg->args[arg->arg_num].assigned = FALSE;
    }

  /* required arguments can't have default value */
  if( arg->args[arg->arg_num].required && arg->args[arg->arg_num].def_value )
   error("Error: required args can't have default value in 'use description'.");

  /* min value */
  p = get_next_token(p,'|',token);
  if( strlen(token) > 0 )
    {
      arg->args[arg->arg_num].min_set = TRUE;
      arg->args[arg->arg_num].min = atof(token);
    }
  else
    {
      arg->args[arg->arg_num].min_set = FALSE;
    }

  /* max value */
  p = get_next_token(p,'|',token);
  if( strlen(token) > 0 )
    {
      arg->args[arg->arg_num].max_set = TRUE;
      arg->args[arg->arg_num].max = atof(token);
    }
  else
    {
      arg->args[arg->arg_num].max_set = FALSE;
    }

  /* argument description */
  p = get_next_token(p,'|',arg->args[arg->arg_num].desc);

  /* the field should end there */
  if( *p != '\0' )
    error("Error: too many tokens in one argument in 'use description'.");

  arg->arg_num++;
}

/*----------------------------------------------------------------------------*/
/** Process an argument definition.
 */
static void process_argument_description( char * desc, struct arguments * arg )
{
  char id[FIELD_LENGTH];
  char value[FIELD_LENGTH];

  /* check input */
  if( desc == NULL || arg == NULL )
    error("Error: invalid input to 'process_argument_description'.");

  /* initialize 'arg' */
  arg->name[0] = '\0';
  arg->author[0] = '\0';
  arg->version[0] = '\0';
  arg->year[0] = '\0';
  arg->desc[0] = '\0';
  arg->compiled[0] = '\0';
  arg->arg_num = 0;
  arg->arg_allocated = 2;
  arg->args = (struct argument *)
                      malloc( arg->arg_allocated * sizeof(struct argument) );
  if( arg->args == NULL ) error("Error: not enough memory.");

  /* assign compilation date and time */
  strcat(arg->compiled,__DATE__);
  strcat(arg->compiled," ");
  strcat(arg->compiled,__TIME__);

  /* process description */
  while( *desc != '\0' )
    {
      desc = get_next_field(desc,id,value);

      if( strcmp(id,"name") == 0 )
        {
          if( arg->name[0] != '\0' )
            error("Error: multiple 'name' fields in 'use description'.");
          strcpy(arg->name,value);
        }
      else if( strcmp(id,"author") == 0 )
        {
          if( arg->author[0] != '\0' )
            error("Error: multiple 'author' fields in 'use description'.");
          strcpy(arg->author,value);
        }
      else if( strcmp(id,"version") == 0 )
        {
          if( arg->version[0] != '\0' )
            error("Error: multiple 'version' fields in 'use description'.");
          strcpy(arg->version,value);
        }
      else if( strcmp(id,"year") == 0 )
        {
          if( arg->year[0] != '\0' )
            error("Error: multiple 'year' fields in 'use description'.");
          strcpy(arg->year,value);
        }
      else if( strcmp(id,"desc") == 0 )
        {
          if( arg->desc[0] != '\0' )
            error("Error: multiple 'desc' fields in 'use description'.");
          strcpy(arg->desc,value);
        }
      else if( strcmp(id,"opt") == 0 || strcmp(id,"req") == 0 )
        {
          process_new_argument(id,value,arg);
        }
      else
        {
          error("Error: unknown token in 'use description'.");
        }
    }

  /* verify required arguments */
  if( arg->name[0] == '\0' )
    error("Error: program name is required in 'use description'.");
  if( arg->author[0] == '\0' )
    error("Error: author name is required in 'use description'.");
  if( arg->version[0] == '\0' )
    error("Error: version is required in 'use description'.");
  if( arg->desc[0] == '\0' )
    error("Error: program description is required in 'use description'.");
  if( arg->year[0] == '\0' )
    error("Error: year is required in 'use description'.");
}

/*----------------------------------------------------------------------------*/
/** Print version.
 */
static void print_version(struct arguments * arg, FILE * f)
{
  if( arg == NULL || f == NULL )
    error("Error: invalid input to 'print_version'.");
  fprintf(f,"Version %s, compiled %s\n",arg->version,arg->compiled);
}

/*----------------------------------------------------------------------------*/
/** Print command line interface help and exit.
 */
static void use(struct arguments * arg)
{
  int i;

  if( arg == NULL ) error("Error: invalid input to 'use'.");

  fprintf(stderr,"----------------------------------------");
  fprintf(stderr,"----------------------------------------\n");
  fprintf(stderr,"This is %s, ",arg->name);
  print_version(arg,stderr);
  fprintf(stderr,"%s\n",arg->desc);
  fprintf(stderr,"Copyright (c) %s %s\n",arg->year,arg->author);
  fprintf(stderr,"----------------------------------------");
  fprintf(stderr,"----------------------------------------\n");
  fprintf(stderr,"\nUsage: %s",arg->name);

  /* always present 'help' option */
  fprintf(stderr," [%s]",HELP_OPTION);

  /* always present version option */
  fprintf(stderr," [%s]",VERSION_OPTION);

  for(i=0;i<arg->arg_num;i++)
    if( !(arg->args[i].required) )
      {
        fprintf(stderr," [-%c",arg->args[i].id);
        if( arg->args[i].type != 'b' ) fprintf(stderr," %s",arg->args[i].name);
        fprintf(stderr,"]");
      }

  for(i=0;i<arg->arg_num;i++)
    if( arg->args[i].required )
      fprintf(stderr," %s",arg->args[i].name);

  fprintf(stderr,"\n\n");

  /* option description */
  fprintf(stderr,"  %s\tPrint this help message and exit.\n",
          HELP_OPTION);
  fprintf(stderr,"  %s\tPrint version and compilation date/time and exit.\n",
          VERSION_OPTION);
  for(i=0;i<arg->arg_num;i++)
    if( !(arg->args[i].required) )
      {
        fprintf(stderr,"  -%c",arg->args[i].id);
        if( arg->args[i].type != 'b' )
          {
            fprintf(stderr," %s",arg->args[i].name);
          }
        fprintf(stderr,"\t%s\n",arg->args[i].desc);
        if( arg->args[i].type == 'i' )
          {
            fprintf(stderr,"\t\t'%s' is integer",arg->args[i].name);
            fprintf(stderr,", range [");
            if( arg->args[i].min_set )
              fprintf(stderr,"%d,",(int)arg->args[i].min);
            else fprintf(stderr,"-inf,");
            if( arg->args[i].max_set )
              fprintf(stderr,"%d]",(int)arg->args[i].max);
            else fprintf(stderr,"inf]");
            if( arg->args[i].def_value )
              fprintf(stderr,", default value %d",atoi(arg->args[i].d_value));
            fprintf(stderr,"\n");
          }
        if( arg->args[i].type == 'd' )
          {
            fprintf(stderr,"\t\t'%s' is double",arg->args[i].name);
            fprintf(stderr,", range [");
            if( arg->args[i].min_set ) fprintf(stderr,"%g,",arg->args[i].min);
            else fprintf(stderr,"-inf,");
            if( arg->args[i].max_set ) fprintf(stderr,"%g]",arg->args[i].max);
            else fprintf(stderr,"inf]");
            if( arg->args[i].def_value )
              fprintf(stderr,", default value %g",atof(arg->args[i].d_value));
            fprintf(stderr,"\n");
          }
      }

  for(i=0;i<arg->arg_num;i++)
    if( arg->args[i].required )
      {
        fprintf(stderr,"  %s",arg->args[i].name);
        fprintf(stderr,"\t%s\n",arg->args[i].desc);
        if( arg->args[i].type == 'i' )
          {
            fprintf(stderr,"\t\t'%s' is integer",arg->args[i].name);
            fprintf(stderr,", range [");
            if( arg->args[i].min_set )
              fprintf(stderr,"%d,",(int)arg->args[i].min);
            else fprintf(stderr,"-inf,");
            if( arg->args[i].max_set )
              fprintf(stderr,"%d]",(int)arg->args[i].max);
            else fprintf(stderr,"inf]");
            fprintf(stderr,"\n");
          }
        if( arg->args[i].type == 'd' )
          {
            fprintf(stderr,"\t\t'%s' is double",arg->args[i].name);
            fprintf(stderr,", range [");
            if( arg->args[i].min_set ) fprintf(stderr,"%f,",arg->args[i].min);
            else fprintf(stderr,"-inf,");
            if( arg->args[i].max_set ) fprintf(stderr,"%f]",arg->args[i].max);
            else fprintf(stderr,"inf]");
            fprintf(stderr,"\n");
          }
      }

  fprintf(stderr,"\n");

  free_arguments(arg);

  exit(EXIT_FAILURE);
}

/*----------------------------------------------------------------------------*/
/** Evaluate arguments.
 */
static void evaluate_arguments(int argc, char ** argv, struct arguments * arg)
{
  int in_required_args = FALSE;
  int n,i;

  /* check input */
  if( argc <= 0 ) error("Error: unexpected command line: missing command.");
  if( argv == NULL || arg == NULL )
    error("Error: invalid input to 'evaluate_arguments'.");

  for( n=1; !in_required_args && n < argc; n++ )
    {
      /* when an argument do not start with "-" it is not optional.
         but, if the argument is just "-", then is a non optional
         argument with value "-", and will be analyzed later.  */
      if( argv[n][0] != '-' || (argv[n][0]=='-' && strlen(argv[n])== 1) )
        {
          in_required_args = TRUE;
          --n;
          continue;
        }

      if( strlen(argv[n]) != 2 )
        {
          /* check if it is the especial option 'help' */
          if( strcmp(argv[n],HELP_OPTION) == 0 ) use(arg);

          /* check if it is the special option 'version' */
          if( strcmp(argv[n],VERSION_OPTION) == 0 )
            {
              print_version(arg,stdout);
              free_arguments(arg);
              exit(EXIT_SUCCESS);
            }

          /* otherwise is a bad option */
          fprintf(stderr,"Error: %s ",argv[n]);
          error("unrecognized option.");
        }

      for( i=0; i<arg->arg_num; i++ )
        if( !(arg->args[i].required) && arg->args[i].id == argv[n][1] )
          {
            arg->args[i].assigned = TRUE;
            if( arg->args[i].type != 'b' )
              {
                /* go for the value */
                ++n;

                /* a value is expected */
                if( n >= argc )
                  {
                    fprintf(stderr,"Error: in '%s': ",argv[n-1]);
                    error("a value was expected.");
                  }
                if( strlen(argv[n]) > FIELD_LENGTH )
                  {
                    fprintf(stderr,"Error: in '%s': ",argv[n-1]);
                    error("value too long.");
                  }
                strcpy(arg->args[i].s_value,argv[n]);
                if( arg->args[i].type == 'i' )
                  {
                    arg->args[i].i_value = atoi(argv[n]);
                    if( arg->args[i].min_set &&
                        arg->args[i].i_value < (int) arg->args[i].min )
                      {
                        fprintf(stderr,"Error: in '%s': ",argv[n-1]);
                        error("value out of range.");
                      }
                    if( arg->args[i].max_set &&
                        arg->args[i].i_value > (int) arg->args[i].max )
                      {
                        fprintf(stderr,"Error: in '%s': ",argv[n-1]);
                        error("value out of range.");
                      }
                  }
                if( arg->args[i].type == 'd' )
                  {
                    arg->args[i].f_value = atof(argv[n]);
                    if( arg->args[i].min_set &&
                        arg->args[i].f_value < arg->args[i].min )
                      {
                        fprintf(stderr,"Error: in '%s': ",argv[n-1]);
                        error("value out of range.");
                      }
                    if( arg->args[i].max_set &&
                        arg->args[i].f_value > arg->args[i].max )
                      {
                        fprintf(stderr,"Error: in '%s': ",argv[n-1]);
                        error("value out of range.");
                      }
                  }
              }
            i = arg->arg_num; /* argument found, stop search */
          }
    }

  for( i=0; n<argc && i<arg->arg_num; i++ )
    if( arg->args[i].required )
      {
        arg->args[i].assigned = TRUE;
        strcpy(arg->args[i].s_value,argv[n]);
        if( arg->args[i].type == 'i' )
          {
            arg->args[i].i_value = atoi(argv[n]);
            if( arg->args[i].min_set &&
                arg->args[i].i_value < (int) arg->args[i].min )
              {
                fprintf(stderr,"Error: in '%s': ",arg->args[i].name);
                error("value out of range.");
              }
            if( arg->args[i].max_set &&
                arg->args[i].i_value > (int) arg->args[i].max )
              {
                fprintf(stderr,"Error: in '%s': ",arg->args[i].name);
                error("value out of range.");
              }
          }
        if( arg->args[i].type == 'd' )
          {
            arg->args[i].f_value = atof(argv[n]);
            if( arg->args[i].min_set &&
                arg->args[i].f_value < arg->args[i].min )
              {
                fprintf(stderr,"Error: in '%s': ",arg->args[i].name);
                error("value out of range.");
              }
            if( arg->args[i].max_set &&
                arg->args[i].f_value > arg->args[i].max )
              {
                fprintf(stderr,"Error: in '%s': ",arg->args[i].name);
                error("value out of range.");
              }
          }
        ++n;
      }
}

/*----------------------------------------------------------------------------*/
/** Process and evaluate a program arguments.
 */
struct arguments * process_arguments(char * desc, int argc, char ** argv)
{
  struct arguments * arg;
  int i;

  /* check input */
  if( desc == NULL || argv == NULL )
    error("Error: invalid input to 'process_arguments'.");

  /* get memory */
  arg = (struct arguments *) malloc(sizeof(struct arguments));
  if( arg == NULL ) error("Error: not enough memory.");

  process_argument_description(desc,arg);
  evaluate_arguments(argc,argv,arg);

  /* if there are missing arguments print the 'use' information */
  for(i=0; i<arg->arg_num; i++)
    if( arg->args[i].required && !(arg->args[i].assigned) ) use(arg);

  return arg;
}

/*----------------------------------------------------------------------------*/
/** Test if an argument has a defined value.
 */
int is_assigned(struct arguments * arg, char * name)
{
  int i;
  if( arg == NULL || name == NULL )
    error("Error: invalid input to 'is_assigned'.");
  for(i=0; i<arg->arg_num; i++)
    if( strcmp(name,arg->args[i].name) == 0 ) return arg->args[i].assigned;
  error("Error: is_assigned: unknown argument.");
  return -1; /* useless, just to prevent warning in strict compilers */
}

/*----------------------------------------------------------------------------*/
/** Get the value of a string argument.
 */
char * get_str(struct arguments * arg, char * name)
{
  int i;

  if( arg == NULL || name == NULL )
    error("Error: invalid input to 'get_str'.");

  for(i=0; i<arg->arg_num; i++)
    if( strcmp(name,arg->args[i].name) == 0 )
      {
        if( arg->args[i].type == 's' )
          {
            if( !(arg->args[i].assigned) ) return NULL;
            return arg->args[i].s_value;
          }
        else error("Error: get_str: the parameter is not a double.");
      }
  error("Error: get_str: unknown argument.");
  return NULL; /* useless, just to prevent warning in strict compilers */
}

/*----------------------------------------------------------------------------*/
/** Get the value of a double argument.
 */
double get_double(struct arguments * arg, char * name)
{
  int i;

  if( arg == NULL || name == NULL )
    error("Error: invalid input to 'get_double'.");

  for(i=0; i<arg->arg_num; i++)
    if( strcmp(name,arg->args[i].name) == 0 )
      {
        if( !(arg->args[i].assigned) )
          error("Error: get_double: parameter not assigned.");
        if( arg->args[i].type == 'd' ) return arg->args[i].f_value;
        else error("Error: get_double: the parameter is not a double.");
      }
  error("Error: get_double: unknown argument.");
  return -1.0; /* useless, just to prevent warning in strict compilers */
}



