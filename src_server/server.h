
#ifndef H_SERV
#define H_SERV

#include <pthread.h>
#include "con.h"
#include "dlogger.h"
#include "message.h"

#define THREAD_STATE_DEAD (1)
#define THREAD_STATE_RUNNING (2)
#define THREAD_STATE_KILLNOW (3)
#define THREAD_STATE_ERROR (4)

client_state_t client_null(void);
global_state_t global_null(void);
client_list_t *client_list_null(void);
int global_addclient(global_state_t *gs,client_state_t *cl);
int global_delclient(global_state_t *gs,client_state_t *cl);
int global_send_all(global_state_t *gs, char *sendthis);
int global_send_others(global_state_t *gs, char *sendme, client_state_t *whoiam);
int global_send_logged_in(global_state_t *gs, char *sendme);

int client_subrun(client_state_t *state);
void *client_handler(void *input);
int outgoing_subrun(global_state_t *state);

//void *outgoing_handler(void *data);
#endif
