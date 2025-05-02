#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

#define BUFSIZE 256

void zeroArray(char**, int);
int readActions(int*, char*, int);
void removeNewline(char* str);

//TODO: wont exicute multiple commands correctly
int main(){

//buffer
char input[256];
memset(input, 0, 256);
//list of commands "ls -la"
char commands[32][256];
memset(commands, 0, sizeof(commands));
//array of actions "pipe, redirect" represented in int
int actions[32];
memset(actions, 0, sizeof(actions)/sizeof(actions[0]));
//strtok pointer
char* tokPtr = NULL;
//argv for execvp. ran on each command
char* args[256];
zeroArray(args, sizeof(args)/sizeof(args[0]));
//num of ps to exicute - 1
int numActions = 0;
//num ps 
int numPs = 0;
//process id 
int pid = 0;



/*PROCESS INPUT*/
//get input
if(fgets(input, sizeof(input), stdin) == NULL)
{
   printf("fgets failed!");
   exit(1);
}
removeNewline(input);


/*EXTRACT COMMANDS AND ACTIONS*/
//extract actions 
numActions = readActions(actions, input, sizeof(actions)/sizeof(actions[0]));
numPs = numActions + 1;
printf("numPs: %d \nnumAction: %d\n\n", numPs, numActions);//debug

//split by command
tokPtr = strtok(input, "|<>");
for(int i = 0; i < 32 && tokPtr != NULL; i++)
{
   strcpy(commands[i], tokPtr);
  // printf("--%s--\n", commands[i]);//debug
   tokPtr = strtok(NULL, "|<>");
}

printf("DEBUG\n");
for(int i = 0;i<numPs; i++){
printf("%s\n", commands[i]);
}
printf("___________________________\n");

/*EXTRACT ARGV AND EXICUTE-----------------------------------------------------------------*/
//split by arg and execute
for(int curPs = 0; curPs < numPs; curPs++)
{
   //copy command to temp, then strtok it
   char temp[BUFSIZE]; memset(temp, 0, BUFSIZE);
   strcpy(temp, commands[curPs]);

   //split temp command by space into argv
   tokPtr = strtok(temp, " ");
   for(int word; tokPtr != NULL && word < BUFSIZE; word++)
   {
      args[word] = tokPtr;//TODO  args[word] = tokPtr or address of tokPtr
      tokPtr = strtok(NULL, " ");
printf("(%s)\n", args[word]);
   }
printf(">NEXT COMMAND>>>\n");

  //clear temp and argv 
  zeroArray(args, sizeof(args) / sizeof(args[0]));
  memset(temp, 0, BUFSIZE);

}

for(int i = 0; i < numPs; i++)
{
   wait(NULL);
}
return 0;
}



//functions 

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
