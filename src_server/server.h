
#ifndef H_SERV
#define H_SERV




#include <pthread.h>

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

client_state_t client_null(void);
global_state_t global_null(void);
int client_subrun(client_state_t *state);
void *client_handler(void *input);
int outgoing_subrun(global_state_t *state);

void *outgoing_handler(void *data);
#endif
