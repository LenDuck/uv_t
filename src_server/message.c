#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "server.h"
#include "message.h"


/*
typedef enum {
  MSG_TYPE_CHAT,
  MSG_TYPE_USER,
  MSG_TYPE_NAMES,
  MSG_TYPE_SAY,
  MSG_TYPE_OK,
  MSG_TYPE_IN_USE,
  MSG_TYPE_JOIN,
  MSG_TYPE_LEAVE,
  MSG_TYPE_RENAME
} enum_msg_type;

typedef enum {
  STATUS_OK,
  STATUS_NOT_OK
} enum_status;

typedef  status;

typedef struct {
  char *arg;
  enum_status status;
  enum_msg_type msg_type;
} msg_t;
*/

msg_t* msg_get(client_state_t *client) {
  /* Set random Phineas and Ferb dude to 0 (zero)*/
  char* buffer = NULL;
  con_t *con = client->connection;
  client_list_t *clients = client->global->clients;
  client_state_t *curr_client;
  
  int status = con_line(con, &buffer);
  if (status != CON_ERROR_NONE) {
    printf("ERROR! No. : %d\n", status);
    return NULL;
  }
  int textsize = 128;
  char *text = malloc(textsize);
  cmd_t *command = parse_command(buffer);
  msg_t *msg = malloc(sizeof(msg_t));
  if (cmd_compare(command->command, "CHAT")) {
    /* Received "CHAT", client auths, send "+OK" */
    msg->arg = "Authed";
    msg->status = STATUS_OK;
    msg->msg_type = MSG_TYPE_CHAT;
  } else if(cmd_compare(command->command, "USER")) {
    /* Received "USER", check in users_list if exists, if no match, reg.*/
    int found = 0;
    while(clients) {
      curr_client = clients->current;
      if (strcmp(curr_client->username, command->msg)) {
        // Match in list: return neg. msg.
        found = 42;
        break;
      }
      clients = clients->next;
    }
    msg->status = (found) ? STATUS_NOT_OK : STATUS_OK;
    msg->msg_type (found) ? MSG_TYPE_IN_USE : MSG_TYPE_OK;
    msg->arg = NULL;
    /*
      Send all other clients RENAME msg.
    */
    clients = client->global->clients;
    
  } else if(cmd_compare(command->command, "NAMES")) {
    /* Received "NAMES"*/
    /*
      user_t *users_list = *FIRST_NODE*
      while (users_list->next)
      {
        if (strln(text) < textsize - 2) {
          sprintf(text, "%s\r\n", users_list->data);
        } else {
          text = realloc(text, textsize += 128);
          sprintf(text, "%s\r\n", users_list->data);
        }
      }
      msg->status = STATUS_OK;
      msg->msg_type = MSG_TYPE_NAMES;
      msg->arg = text;
    */
    msg->arg = "NAMES test";
    msg->status = STATUS_OK;
    msg->msg_type = MSG_TYPE_NAMES;
  } else if(cmd_compare(command->command, "SAY")) {
    /* Received "SAY"*/
    msg->arg = command->msg;
    msg->status = STATUS_OK;
    msg->msg_type = MSG_TYPE_SAY;
  }
  return msg;
}

int cmd_compare(char *a, char *b)
{
  int x, y;
  x = strcmp(a, b);
  y = strcasecmp(a, b);
  if  (!x && !y) { return 0; }
  else if (x && !y) { return 1; }
  else if (x && y) { return 2; }
  else return 4;
}

cmd_t* parse_command(char *buffer) {
  int i = 0, j = 0, size1 = 32, size2 = 32;
  char *command = malloc(size1), *text = malloc(size2);
  
  while(buffer[i]) {
    while(buffer[i] != ' ') {
      if (i < size1 - 1) {
        command[i] = buffer[i];
      } else {
        command = realloc(command, size1 *= 2);
        command[i] = buffer[i];
      }
      i++;
    }
    if (j < size2 - 1) {
      char c = buffer[i++];
      if ((c < ' ') || (c == 127)) {
        printf("Error: wrong characters!\n");
        return NULL;
      }
      text[j++] = c;
    } else {
      command = realloc(text, size2 *= 2);
      char c = buffer[i++];
      if ((c < ' ') || (c == 127)) {
        printf("Error: wrong characters!\n");
        return NULL;
      }
      text[j++] = c;
    }
  }
  cmd_t *rvl = malloc(sizeof(cmd_t));
  rvl->command = command;
  rvl->msg = text;
  
  return rvl;
}

