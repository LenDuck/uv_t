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
  ret.state = 0;
  pthread_mutex_init(&ret.access,NULL);
  ret.current_line = NULL; 
  ret.thread_state = THREAD_STATE_DEAD;
  ret.connection = NULL;
  ret.log = NULL;
  ret.global = NULL;
  ret.username = NULL;
  return ret;
}

void state_client_free(client_state_t *state){
  if (!state) return;
/*leaks*/
  free(state);
}

int client_subrun(client_state_t *state){
  if (!(state->connection) &&
      (state->log)
      ) {
    state->thread_state = THREAD_STATE_ERROR;
    return CON_ERROR_UNKNOWN;
  }
  if (msg_get(state)) return CON_ERROR_UNKNOWN;
  
  /*This the only place you can use state->current_line without the lock*/
  return CON_ERROR_NONE;
}

void *client_handler(void *input){
  client_state_t *state = (client_state_t *) input;

  if (!input) return NULL;
  init_client(input);
  printf("client_handler starting\n");
  state->state |= CLIENT_STATE_CONNECTED;
  global_addclient(state->global,state);
  while (state->thread_state == THREAD_STATE_RUNNING){
    if (client_subrun(state) == CON_ERROR_NONE) continue;
    printf("Client break\n");
    break;
  }
  printf("Done client %d\n",state->thread_state);
  char *text = malloc(128);
  sprintf(text, "+LEAVE %s", state->username);
  global_send_others(state->global, text, state);
  global_delclient(state->global, state);
  state_client_free(state);
  return NULL;
}

int init_client(client_state_t *client) {
  char default_name[] = "Stinky2001";
  client->username = malloc(1+strlen(default_name));
  strcpy(client->username, default_name);
  
  return 0;
}
