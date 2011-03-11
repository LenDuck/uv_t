#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include "con.h"

/*Global variables, as only one connection is to be made*/
pthread_t user_handler;
pthread_mutex_t mutex_print;
con_t con = NULL;
int error = 0;
char *username = NULL;

/*The version our client uses */
#define SERVER_VERSION (1)
#define MINOR_VERSION (0)

/*Send a command, with an argument if given*/
int send_command(const char* cmd, const char *arg){
  char *msg = NULL;
  int rva;
  if (!cmd) return -1;
  msg = malloc(strlen(cmd) + (arg?strlen(arg):0) + 4); /*" " + "\r\n\0"*/
  if (!msg) return -2;

  if (arg){
    sprintf(msg, "%s %s\r\n",cmd,arg);
  } else {
    sprintf(msg, "%s\r\n",cmd);
  }
  rva = con_send_line(con, msg);
  free(msg);

  return rva; 
}

/*Say something, a shorter send_comman("SAY",...)*/
int say_user(const char *what){
  if (!what) return -1;

  return send_command("SAY",what); 
}

/*The character which defines "client commands"*/
#define COMMAND_CHAR '/'

/*This is the function called by the user thread*/
void *handle_user(void* data){
  /*anti warning*/ (void)data;

  /*While con is valid, and no errors are known loop*/
  while (con && (!error)){
    char userin[1024];
    int rva = 0;
    userin[0] = 0;

    /*Read a big chunk of text, but do limit at some point*/
    rva = scanf("%1023[^\n^\r]",userin);
    printf("Got%d: %s\n",rva,userin);
    if (rva == EOF){
      /*This should be called when no more data can be read*/
      error = 1;
      break;
    } else if (rva == 0){
      /*Remove empty lines, */
      rva = scanf("%1023[\r\n]",userin);
      continue;
    } else if (strlen(userin) < 1){
      /*If you manage to enter a empty string, refuse to parse it*/
      continue;
    }

    if (userin[0] == COMMAND_CHAR){
      /*Length of the command + args*/
      int len = strlen(userin) -1;

      /*look for a command*/
      if ((len >= 3 ) && strncasecmp(userin+1, "who",3) == 0){
        send_command("NAMES",NULL);

      } else if ((len >= 2 ) && strncasecmp(userin+1, "me",2) == 0){
        /*Print what the client thinks your name is*/
        pthread_mutex_lock(&mutex_print);
        printf("You are known as: %s\n",username);
        pthread_mutex_unlock(&mutex_print);

      } else if ((len >= 4 ) && strncasecmp(userin+1, "quit",4) == 0){
        error = 42;/*quit*/

      } else if ((len >= 5 ) && strncasecmp(userin+1, "user ",5) == 0){
        /*Send the USER command to change name*/
        send_command("USER",userin +6);
        strcpy(username, (userin+6));
        /*Does not receive feedback (feedback is printed by other thread)*/

      } else if ((len >= 6 ) && strncasecmp(userin+1, "connect",6) == 0){
        /*Was used in early stages, might still be usefull*/
        send_command("CHAT",userin +6);
        send_command("USER",username);

      } else if ((len >= 4 ) && strncasecmp(userin+1, "help",4) == 0){
        pthread_mutex_lock(&mutex_print);
        printf("Known commands: %c + :\n\twho\n\tme\n\tquit\n", COMMAND_CHAR);
        printf("\tuser\n\tconnect\n\thelp\n");
        pthread_mutex_unlock(&mutex_print);

      } else {
        pthread_mutex_lock(&mutex_print);
        printf("Unknown command: '%s'\n",userin+1);
        pthread_mutex_unlock(&mutex_print);
      }
    } else {
      /*send it to server*/
      say_user(userin);
    }
  }
  if (error){
    fprintf(stderr, "Error occured, shutting down input\n");
  }
  if (! con){
    fprintf(stderr,"Connection closed, shutting down input\n");
  }
  return NULL;
}

int main(int argc, char **argv) {
  char *hostname = "localhost";
  char *port = "55555";

  if ((argc == 2) && (
     (strcasecmp("--help",argv[1]) == 0) ||
     (strcasecmp("-help",argv[1]) == 0) ||
     (strcasecmp("-h",argv[1]) == 0) ||
     (strcasecmp("-?",argv[1]) == 0)
    )){
    printf("Client for chat server, usage:\n%s\n%s --help\n%s HOSTNAME PORT\n",
            argv[0],argv[0],argv[0]);
    printf("Known commands: %c + :\n\twho\n\tme\n\tquit\n", COMMAND_CHAR);
    printf("\tuser\n\tconnect\n\thelp\n");
  }

  if (argc == 3) {
    hostname = argv[1];
    port = argv[2];
  }
  pthread_mutex_init(&mutex_print,NULL);
  con = con_bootup(hostname,port);
  if (!con){
    return -1;
  }
  if (con_connect(con)){
    return -1;
  }


  if (! con){
    fprintf(stderr,"connection failed\n");
    return -1;
  }
  int num = 0;

  { /*Get chat initial message, check version*/
    char *ischat = NULL;
    int rva = 0;
    int version = -1;
    int minor = -1;
    
    rva = con_line(con, &ischat);
    if (rva ||(1 > strlen(ischat)) ){
      fprintf(stderr,"Invalid welcome message:%s\n", ischat);
      free(ischat);
      con_close(con);
      return 0;
    }
    rva = sscanf(ischat,"CHAT/%d.%d/\r\n",&version,&minor);
    free(ischat);
    if ((rva == 2) && (version <= SERVER_VERSION)&&(minor >= MINOR_VERSION)){
      /*OK*/
      printf("Connection with server alive\n");
    } else {
      printf("Connection not with valid server version: %d,%d\n",version,minor);
      con_close(con);
      return 0;
    }
  }

  { /*Connect using CHAT command*/
    char *ischat = NULL;
    int rva = 0;
    send_command("CHAT",NULL);
    rva = con_line(con, &ischat);
    printf("%s\n",ischat);
      
    if (rva ||(1 > strlen(ischat)) || (! (ischat[0] == '+'))){
      fprintf(stderr,"Invalid CHAT response:%s\n", ischat);
      free(ischat);
      con_close(con);
      return 0;
    }
    free(ischat);
  }

  { /*Try some usernames until success*/
    while (con){
      int rva = 0;
      char buffer[128];
      char *resp = NULL;
      printf("Username please: ");
      fflush(stdout);
      buffer[0] = 0;
      num = scanf("%127[^\r^\n]",buffer);

      if (num == 1){
        username = realloc(username, 1+strlen(buffer));
        strcpy(username,buffer);
      } else {
        scanf("%127[\r\n]",buffer);
        continue;
      }
      send_command("USER",username);
      rva = con_line(con, &resp);
      if (rva ||(1 > strlen(resp)) || (! (resp[0] == '+'))){
        fprintf(stderr,"Name taken:%s\n", resp);
        free(resp);
      } else {
        free(resp);
        break;/*done picking name*/
      }
    }
  }
  num = pthread_create(&user_handler, NULL,handle_user,NULL);
  
  if (num){
    fprintf(stderr,"Error in thread\n");
  }

  while (con){
    int rva;
    char *buffer = NULL;
    num++;
    rva = con_line(con,&buffer);
    if ((rva == CON_ERROR_NONE) &&
        buffer &&
        (strlen(buffer) > 0)
       ){ 
      pthread_mutex_lock(&mutex_print);
      /*Print the message, could be fed to parser....*/
      printf("%s\n",buffer);
      pthread_mutex_unlock(&mutex_print);

    }
    if (buffer) free(buffer);
    if (rva == CON_ERROR_NONE ) continue;
    printf("Receive error %d\n",rva);
    break;
  }
  con_close(con);
  fprintf(stderr,"done\n");

  (void)argc;
  (void)argv;

  return 0;
}

