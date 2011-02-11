
#ifndef H_SERV
#define H_SERV




#include <pthread.h>
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

typedef struct struct_client_list{
  struct struct_client_list *next;
  client_state_t *current; 
} client_list_t;

typedef struct struct_global_state{
  client_list_t *clients;
  pthread_mutex_t access;

  int users_online;
  con_t listen_connection;
  dlog_t *log;
  int listen_thread_state;
  int outgoing_thread_state;
  pthread_t listen_thread;
  pthread_t outgoing_thread;
} global_state_t;

client_state_t client_null(void);
global_state_t global_null(void);
client_list_t *client_list_null(void);
int global_addclient(global_state_t *gs,client_state_t *cl);
int global_delclient(global_state_t *gs,client_state_t *cl);
int global_send_all(global_state_t *gs, char *sendthis);

int client_subrun(client_state_t *state);
void *client_handler(void *input);
int outgoing_subrun(global_state_t *state);

//void *outgoing_handler(void *data);
#endif
