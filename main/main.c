#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

#define BUFSIZE 256
#define NUMCOMMANDS 32

void zeroArray(char**, int);
int readActions(int*, char*, int);
void removeNewline(char* str);
int initPipes(int[][2], int);
void fillArgv(char *, int, char**);

//TODO handle quotations in args like grep "hi"

int main(){

//buffer
char input[BUFSIZE];
//list of commands "ls -la"
char commands[NUMCOMMANDS][BUFSIZE];
//array of actions "pipe, redirect" represented in int
int actions[NUMCOMMANDS - 1];
//strtok pointer
char* tokPtr = NULL;
//argv for execvp. ran on each command
char* args[BUFSIZE];
//num of ps to exicute - 1
int numActions = 0;
//num ps 
int numPs = 0;
//process id 
int pid = 0;
//pipes 
int pipes[NUMCOMMANDS][2];

while(1)
{

//clear buffers
memset(input, 0, sizeof(input));
memset(commands, 0, sizeof(commands));
memset(actions, 0, sizeof(actions));
zeroArray(args, sizeof(args)/sizeof(args[0]));

/*PROCESS INPUT*/
//get input
printf("\n>> ");

if(fgets(input, sizeof(input), stdin) == NULL)
{
   printf("fgets failed!");
   exit(1);
}

removeNewline(input);
printf("\n");


/* EXTRACT COMMANDS AND ACTIONS */
//extract actions 
numActions = readActions(actions, input, sizeof(actions)/sizeof(actions[0]));

//create pipes 
if(initPipes(pipes, numActions) == -1)
{
  printf("pipes failed!");
  exit(1);
}
numPs = numActions + 1;



//split by command
tokPtr = strtok(input, "|<>");

for(int i = 0; i < NUMCOMMANDS && tokPtr != NULL; i++)
{
   strcpy(commands[i], tokPtr);
   tokPtr = strtok(NULL, "|<>");
}

   //DEBUG
    //printf("numPs: %d\n", numPs);
    //printf("num actions: %d\n\n\n", numActions);
    
/* LOOP THROUGH COMMANDS AND SPAWN PS FOR EACH. */
for(int curPs = 0; curPs < numPs; curPs++)
{
  //fork each child, each child gets zero'ed buffer and argv. no need to clean up
  pid = fork(); 

  if(pid < 0)
  {
    printf("failed fork\n");
    exit(0);
  }

  //************************CHILD PS***********************
  if(pid == 0)
  {
 
    //copy command to temp, then strtok it
    char temp[BUFSIZE]; memset(temp, 0, sizeof(temp));
    strcpy(temp, commands[curPs]);

    
    //split temp command by space or quotation into argv- COULD CAUSE ISSUES with quotation!
    fillArgv(temp, sizeof(temp)/sizeof(temp[0]), args);
    //printf("\n");//DEBUG
    
      //SETUP PIPES
      //if more than 1 ps then pipes needed
    if(numPs > 1)
    {
      //redirect stdout to pipe1 write
       if(curPs == 0)
       {
          dup2(pipes[curPs][1], STDOUT_FILENO);
       }
       
       //last ps stdin to last pipe read
       else if(curPs == numPs - 1)
       {
          dup2(pipes[curPs - 1][0], STDIN_FILENO);
       }

       //middle pipes
       else
       {
        //stdin to pipe before read
        dup2(pipes[curPs - 1][0], STDIN_FILENO);
        //stdout to pipe write
        dup2(pipes[curPs][1], STDOUT_FILENO);
       }
    }

    //close all pipe, heck why not?
    //parent close pipes
    for(int i = 0; i < numActions; i++)
    {
      close(pipes[i][0]);
      close(pipes[i][1]);
    }

   //execvp 
   execvp(args[0], args);
   //exit child when done
   exit(0);
  }//**********************************END CHILD **********************/

}

//parent close pipes
for(int i = 0; i < numActions; i++)
{
  close(pipes[i][0]);
  close(pipes[i][1]);
}

//parent wait
for(int i = 0; i < numPs; i++)
{
   wait(NULL);
}
}
return 0;
}



/* FUNCTIONS */

void zeroArray(char** ar, int size){
    for(int i=0; i<size; i++){
      ar[i] = NULL;
   }
   return;
}

//loops through a string and fills ar with actions, then returns number of actions (num execvp = num actions + 1)
int readActions(int* ar, char* str, int size){
   char* ptr = NULL;
   char buf[BUFSIZE]; memset(buf, 0, sizeof(buf)/sizeof(buf[0]));
   strcpy(buf, str);

   //split buffer
   int arIndex = 0;
   ptr = strtok(buf, " ");
   while(ptr != NULL){
    
      if(!strcmp(ptr, "|"))
        {
          ar[arIndex] = 1;
          arIndex++;
        }

      if(!strcmp(ptr, "<"))
        {
          ar[arIndex] = 2;
          arIndex++;
        }

      if(!strcmp(ptr, ">"))
        {
          ar[arIndex] = 3;
          arIndex++;
        }

      if(!strcmp(ptr, "<<"))
        {
          ar[arIndex] = 4;
          arIndex++;
        }

      if(!strcmp(ptr, ">>"))
        {
          ar[arIndex] = 5;
          arIndex++;
        }

      if(!strcmp(ptr, "&&"))
        {
          ar[arIndex] = 6;
          arIndex++;
        }

      ptr = strtok(NULL, " ");
   }

return arIndex;
}

//removes newline 
void removeNewline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

//initializes pipes 
int initPipes(int pipes[][2], int numPipes){
  
  for(int i = 0; i < numPipes && i < NUMCOMMANDS; i++)
  {
    if(pipe(pipes[i]) == -1)
    {
      return -1;
    }
  }
  return 0;
}


//custom string parser to include strings (replace with official library of some sort)
//0 success, -1 fail 
void fillArgv(char * ar, int arSize, char** argv){
    
    int argvIndex = 0;
    int parsingCommand = 0;

    for(int arIndex = 0; arIndex < arSize && ar[arIndex] != 0; arIndex++)
    {
        if(ar[arIndex] == ' ')
        {
            ar[arIndex] = 0;
            parsingCommand = 0;
        }
        else if((ar[arIndex] == '\"' || ar[arIndex] == '\'') && parsingCommand == 0)
        {
            parsingCommand = 1;//signal parsing for safety
            ar[arIndex] = 0;//set quotation to null
            arIndex++;//move cursor to right of quotation
            argv[argvIndex] = &ar[arIndex];//set argv to start of substring
            argvIndex++;//set argv index for next command

            for(; ar[arIndex] != 0; arIndex++)
            {
                if(ar[arIndex] == '\'' || ar[arIndex] == '\"')
                {
                    ar[arIndex] = 0;//nullify end quotation
                    break;
                }
            }
            parsingCommand = 0;//done parsing
            //printf("break--\n");//debug
        }
        else if(parsingCommand == 0)
        {
            argv[argvIndex] = &ar[arIndex];
            
            argvIndex++;
            parsingCommand = 1;//signal that we are looking for end of word
        }
    }

}