#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <string.h>
#include "con.h"
#include "dlogger.h"
#include "server.h"

global_state_t global_null(void){
  global_state_t ret;
  ret.clients = NULL;
  pthread_mutex_init(&ret.access,NULL);
  ret.users_online = 0;
  ret.listen_connection = NULL;
  ret.log = NULL;
  ret.listen_thread_state = THREAD_STATE_DEAD;
  ret.outgoing_thread_state = THREAD_STATE_DEAD;
  return ret;
}

/*
void * global_spim(void * par){
  global_state_t *state = (global_state_t *)par;
  //int i=0;
  while (1){
//  printf(".%d\n",i);
 // i++;
    //insert spimming code here
    global_send_all(state, "Want cookie? give me money!\n\r");
    
    sleep(1);
  }
  return NULL;
}
*/

/*unsigned int sleep(unsigned int secs) */
int listen_subrun(global_state_t *state){
  int rva = 0;
  client_state_t *new_client = NULL; 
  if (!state) return CON_ERROR_UNKNOWN;
  new_client = malloc(sizeof(client_state_t));
 
  if (!new_client) return CON_ERROR_MEMORY;
  *new_client = client_null();
  new_client->global = state;
  new_client->log = dlog_newlog_file("client log",
                                "client.log",
                                DLOG_FLAG_FFLUSH | DLOG_FLAG_NLINE);
  rva = con_serve_accept(state->listen_connection, &new_client->connection);
  if (new_client->connection){
    con_send_line(new_client->connection,
                  SERVER_WELCOME);
  } else { 
    fprintf(stderr,"Yikes %d\n",rva);
  }
  pthread_mutex_lock(&new_client->access);
  new_client->thread_state = THREAD_STATE_RUNNING;
  rva = pthread_create(&new_client->handler_thread, NULL, client_handler, (void*) new_client);
  if (rva) perror("oops, thread");
  pthread_mutex_unlock(&new_client->access);


  return CON_ERROR_NONE;
}

void *listen_handler(void *data){
  global_state_t *state = (global_state_t *)data;
  if (!state) return NULL;
 
  if (con_init_serve(state->listen_connection)){
    dlog_log_text("serve fail",state->log);
    return NULL;
  }

  while (state->listen_thread_state == THREAD_STATE_RUNNING){
    int rva = listen_subrun(state);
    if (rva == CON_ERROR_NONE) continue;
    break;
  }
  
  return NULL;
}
int main(int argc, char **argv) {

  global_state_t *state = NULL;
  /*client_state_t *new_client = NULL;*/

  char *hostname = "localhost";
  char *port = "55555";
  int rva = 0;

  state = malloc(sizeof(global_state_t));
  *state = global_null();
  

  state->log = dlog_newlog_file("Server log",
                                      "server.log",
                                      DLOG_FLAG_FFLUSH | DLOG_FLAG_NLINE);
  dlog_log_text("Bootup start",state->log);

  state->listen_connection = con_bootup(hostname,port);
  if (!state->listen_connection){
    dlog_log_text("con fail",state->log);
    return -1;
  }


  pthread_mutex_lock(&state->access);
  state->listen_thread_state = THREAD_STATE_RUNNING;
  rva = pthread_create(&state->listen_thread, NULL, listen_handler, (void*) state);
  if (rva) perror("oops, thread");
  pthread_mutex_unlock(&state->access);
/*
  {
    pthread_t spimmer ;
    rva = pthread_create(&spimmer,NULL,global_spim,(void*)state);
    
  }
*/
  pthread_join(state->listen_thread,NULL);
  //con_close(state->listen_connection);
  //con_close(new_client->connection);

  //dlog_log_text("Main done, closing log",state->log);
  //dlog_close_log(state->log);

  fprintf(stderr,"done\n");

  (void)argc;
  (void)argv;

  return 0;
}

