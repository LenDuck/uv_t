#include <stdio.h>
#include <stdlib.h>

#include "server.h"
#include "con.h"

client_list_t *client_list_null(void){
  client_list_t *ret = malloc(sizeof(client_list_t));
  if (!ret) return NULL;
  ret->next = NULL;
  ret->current = NULL;

  return ret;
}

/*0 is ok, -? is error*/
int global_addclient(global_state_t *gs, client_state_t *cl){
  int rva;
  int i;
  client_list_t *newc = NULL;
  client_list_t *cur = NULL;

  if (!(gs && cl)) return -1;
  newc = client_list_null();
  if (!newc) return -2;
  newc->current = cl;
  pthread_mutex_lock(&gs->access);
  cur = gs->clients;
  if (!cur){
    gs->clients = newc;
    pthread_mutex_unlock(&gs->access);
    return 0;
  }
  cur->next = NULL;

  while (cur && cur->next){
    cur = cur->next;
  }
  gs->users_online++;
  if (cur){
    cur->next = newc;
  } else {
    gs->clients = newc;
  }
  pthread_mutex_unlock(&gs->access);
  return 0;
}

/*0 is ok, -? is error*/
int global_delclient(global_state_t *gs, client_state_t *cl){
  int rva = 0;
  client_list_t *prev = NULL;
  client_list_t *newc = NULL;
  client_list_t *cur = NULL;

  if (!(gs && cl)) return -1;

  pthread_mutex_lock(&gs->access);
  cur = gs->clients;
  cur->next = NULL;

  while (cur && (! (cur->current == cl)) && cur->next){
    prev = cur;
    cur = cur->next;
  }
  if (cur && (cur->current == cl)){
    gs->users_online--;
    if (prev == NULL){
      gs->clients = cur->next;
    } else {
      prev->next = cur->next;
    }
  } else {
    /*not found?*/
    rva = -3;
  }
  pthread_mutex_unlock(&gs->access);
  return rva;
}

int global_send_all(global_state_t *gs,char *sendme){
  client_list_t *cur = NULL;
  if (!(gs && sendme)) return -1;
  pthread_mutex_lock(&gs->access);
  cur = gs->clients;
  /*add error check*/
  while (cur){
    if (cur->current) con_send_line(cur->current->connection, sendme);
    cur = cur->next;
  }
  
  pthread_mutex_unlock(&gs->access);
  return 0;
}
