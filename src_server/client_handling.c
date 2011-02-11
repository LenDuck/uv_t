#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include "con.h"
#include "dlogger.h"
#include "server.h"

client_state_t client_null(void){
  client_state_t ret;

  pthread_mutex_init(&ret.access,NULL);
  
  ret.thread_state = THREAD_STATE_DEAD;
  ret.connection = NULL;
  ret.log = NULL;

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

