#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include "con.h"

pthread_t user_handler;
pthread_mutex_t mutex_print;
//pthread_mutex_t mutex_send;

con_t con = NULL;
int error = 0;

char *username = NULL;

#define SERVER_VERSION (1)
#define MINOR_VERSION (0)

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

int say_user(const char *what){
  
  if (!what) return -1;

  return send_command("SAY",what); 
}
#define COMMAND_CHAR '#'
void *handle_user(void* data){
  /*anti warning*/ (void)data;
  printf("Hoi\n");
  while (con && (!error)){
    char userin[1024];
    int rva = 0;
    userin[0] = 0;

    rva = scanf("%1023[^\n]",userin);
    printf("Got%d: %s\n",rva,userin);
    if (rva == EOF){
      error = 1;
    } else if (rva == 0){
      rva = scanf("%1023[\n]",userin);
      continue;
    } else if (strlen(userin) < 1){
      continue;
    }

    if (userin[0] == COMMAND_CHAR){
      int len = strlen(userin) -1;
      /*look for a command*/
      if ((len >= 3 ) && strncasecmp(userin+1, "who",3) == 0){
        send_command("NAMES",NULL);
      } else if ((len >= 2 ) && strncasecmp(userin+1, "me",2) == 0){
        pthread_mutex_lock(&mutex_print);
        printf("You are known as: %s\n",username);
        pthread_mutex_unlock(&mutex_print);
      } else if ((len >= 4 ) && strncasecmp(userin+1, "quit",4) == 0){
        error = 42;/*quit*/
      } else if ((len >= 5 ) && strncasecmp(userin+1, "user ",5) == 0){
        send_command("USER",userin +5);
      } else if ((len >= 6 ) && strncasecmp(userin+1, "connect",6) == 0){
        send_command("CHAT",userin +5);
        send_command("USER",username);

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
  char hostname[] = "localhost";
  char port[] = "55555";
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
//    printf("%s\n",ischat);
    free(ischat);
    if ((rva == 2) && (version == SERVER_VERSION)&&(minor >= MINOR_VERSION)){
      /*OK*/
      printf("Connection with server alive\n");
    } else {
      printf("Connection not with valid server: %d,%d\n",version,minor);
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
      num = scanf("%127[^\n]",buffer);

      if (num == 1){
        username = realloc(username, 1+strlen(buffer));
        strcpy(username,buffer);
      } else { 
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

