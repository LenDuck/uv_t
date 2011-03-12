#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include "con.h"
#include "dlogger.h"

#define THREAD_STATE_DEAD (1)
#define THREAD_STATE_RUNNING (2)
#define THREAD_STATE_KILLNOW (3)
#define THREAD_STATE_ERROR (4)

typedef struct struct_client_state{
  int thread_state;
  pthread_mutex_t access;
  pthread_t handler_thread;

  con_t connection;
  dlog_t *log;
  char *username;
  char *current_line;

  
} client_state_t;

typedef struct struct_global_state{
  pthread_mutex_t access;

  int users_online;
  con_t listen_connection;
  dlog_t *log;
  int listen_thread_state;
  int outgoing_thread_state;
  pthread_t listen_thread;
  pthread_t outgoing_thread;
} global_state_t;

client_state_t client_null(void){
  client_state_t ret;

  pthread_mutex_init(&ret.access,NULL);
  
  ret.thread_state = THREAD_STATE_DEAD;
  ret.connection = NULL;
  ret.log = NULL;

  return ret;
}
global_state_t global_null(void){
  global_state_t ret;

  pthread_mutex_init(&ret.access,NULL);
  ret.users_online = 0;
  ret.listen_connection = NULL;
  ret.log = NULL;
  ret.listen_thread_state = THREAD_STATE_DEAD;
  ret.outgoing_thread_state = THREAD_STATE_DEAD;
  return ret;
}
int client_subrun(client_state_t *state){
 char *buffer = NULL;
  int rva = 0;
  if ((!state->connection) ||
      (!state->log)
      ) {
    state->thread_state = THREAD_STATE_ERROR;
    return CON_ERROR_UNKNOWN;
  }
  pthread_mutex_lock(&state->access);
  rva = con_line(state->connection,
                 &buffer);
  if (rva == CON_ERROR_NONE){
    if (state->current_line) free(state->current_line);
    state->current_line = buffer;
  } else if (rva == CON_ERROR_CLOSED){ 
    state->thread_state = THREAD_STATE_KILLNOW; 
  }
  pthread_mutex_unlock(&state->access);

  printf("%s",buffer);
  printf("State:%d\n",state->thread_state);
  /*This the only place you can use state->current_line without the lock*/
  return CON_ERROR_NONE;
}
void *client_handler(void *input){
  client_state_t *state = (client_state_t *) input;

  if (!input) return NULL;
  printf("client_handler starting\n");
  while (state->thread_state == THREAD_STATE_RUNNING){
    if (client_subrun(state) == CON_ERROR_NONE) continue;
    printf("Client break\n");
    break;
  }
  printf("Done client %d\n",state->thread_state);
  return NULL;
}
int outgoing_subrun(global_state_t *state){
  int i=0;
  if (!state) return CON_ERROR_UNKNOWN;

  /*insert spimming code here*/
  return CON_ERROR_NONE;
}

void *outgoing_handler(void *data){
  global_state_t *state = (global_state_t *)data;
  if (!state) return NULL;
  
  while (state->outgoing_thread_state == THREAD_STATE_RUNNING){
    int rva = outgoing_subrun(state);
    if (rva == CON_ERROR_NONE) continue;
    break;
  }
  return NULL;
}
int main(int argc, char **argv) {

  global_state_t *state = NULL;
  client_state_t *new_client = NULL;

  char *hostname = "localhost";
  char *port = "55555";
  int rva = 0;

  state = malloc(sizeof(global_state_t));
  *state = global_null();
  
  new_client = malloc(sizeof(client_state_t));
  *new_client = client_null();

  state->log = dlog_newlog_file("Server log",
                                      "server.log",
                                      DLOG_FLAG_FFLUSH | DLOG_FLAG_NLINE);
  dlog_log_text("Bootup start",state->log);

  state->listen_connection = con_bootup(hostname,port);
  if (!state->listen_connection){
    dlog_log_text("con fail",state->log);
    return -1;
  }

  if (con_init_serve(state->listen_connection)){
    dlog_log_text("serve fail",state->log);
    return -1;
  }


  new_client->log = dlog_newlog_file("client log",
                                "client.log",
                                DLOG_FLAG_FFLUSH | DLOG_FLAG_NLINE);
  rva = con_serve_accept(state->listen_connection, &new_client->connection);
  if (new_client->connection){
    con_send_line(new_client->connection,
"Phasers do not include batteries,\nPhaser don't even include tangibility\n");
  } else {
    fprintf(stderr,"Yikes %d\n",rva);
  }
  pthread_mutex_lock(&new_client->access);
  new_client->thread_state = THREAD_STATE_RUNNING;
  rva = pthread_create(&new_client->handler_thread, NULL, client_handler, (void*) new_client);
  if (rva) perror("oops, thread");
  pthread_mutex_unlock(&new_client->access);

  pthread_join(new_client->handler_thread,NULL);
  con_close(state->listen_connection);
  con_close(new_client->connection);

  dlog_log_text("Main done, closing log",state->log);
  dlog_close_log(state->log);

  fprintf(stderr,"done\n");

  (void)argc;
  (void)argv;

  return 0;
}

