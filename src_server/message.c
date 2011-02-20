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
  STATUS_POS,
  STATUS_NEG
} enum_status;

typedef  status;

typedef struct {
  char *arg;
  enum_status status;
  enum_msg_type msg_type;
  enum_client_state to_whom;
} msg_t;
*/

int msg_get(client_state_t *client) {
  con_t *con = &client->connection;
  client_list_t *clients = client->global->clients;
  client_state_t *curr_client;
  char *buffer = NULL;
  
  int status = con_line(*con, &buffer);
  if (status != CON_ERROR_NONE) {
    printf("ERROR! No. : %d\r\n", status);
    return 2;
  }
  unsigned int textsize = 128;
  char *text = malloc(textsize);
  if (!(text)) return 1;
  cmd_t *command = parse_command(buffer);
  msg_t *msg = malloc(sizeof(msg_t));
  if (!(msg)) return 1;
  if (cmd_compare(command->command, "CHAT")) {
    /* Received "CHAT", client auths, send "+OK" */
    msg->arg = "";
    msg->status = STATUS_POS;
    msg->msg_type = MSG_TYPE_CHAT;
    msg->to_whom = CLIENT_STATE_ONLY_ME;
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
    msg->status = (found) ? STATUS_NEG : STATUS_POS;
    msg->msg_type = (found) ? MSG_TYPE_IN_USE : MSG_TYPE_OK;
    msg->arg = (found) ? "Username in use" : "";
    msg->to_whom = CLIENT_STATE_ONLY_ME;
    /*
      Send all other clients RENAME msg.
    */
    if (!(found)) {
      msg_t *mesg = malloc(sizeof(msg_t));
      mesg->arg = malloc(strlen(curr_client->username) + strlen(command->msg) + 2);
      sprintf(mesg->arg, "%s/%s\r\n", curr_client->username, command->msg);
      mesg->msg_type = MSG_TYPE_RENAME;
      mesg->status = STATUS_POS;
      mesg->to_whom = CLIENT_STATE_NOT_ME;
      send_msg(mesg, client);
    }
  } else if(cmd_compare(command->command, "NAMES")) {
    /* Received "NAMES"*/
    while(clients) {
      curr_client = clients->current;
      if (strlen(text) < textsize - 2) {
        sprintf(text, "%s\r\n", curr_client->username);
      } else {
        text = realloc(text, textsize += 128);
        sprintf(text, "%s\r\n", curr_client->username);
      }
      clients = clients->next;
    }
    msg->arg = text;
    msg->status = STATUS_POS;
    msg->msg_type = MSG_TYPE_NAMES;
    msg->to_whom = CLIENT_STATE_ONLY_ME;
  } else if(cmd_compare(command->command, "SAY")) {
    /* Received "SAY"*/
    msg->arg = malloc(strlen(command->msg) + strlen(client->username) + 4);
    if (msg->arg) return 1;
    sprintf(msg->arg, "%s/%s\r\n", client->username, command->msg);
    msg->status = STATUS_POS;
    msg->msg_type = MSG_TYPE_SAY;
    msg->to_whom = CLIENT_STATE_LOGGED_IN;
  }
  send_msg(msg, client);
  return 0;
}

void send_msg(msg_t *msg, client_state_t *client) {
  char *text = malloc(1 + 6 + 1 + strlen(msg->arg) + 1);
  char sign = (msg->status == STATUS_POS) ? '+' : '-';
  char *msg_type = malloc(7);
  if (msg->msg_type == MSG_TYPE_CHAT) {
    msg_type = "CHAT";
  } else if (msg->msg_type == MSG_TYPE_SAY) {
    msg_type = "SAY";
  } else if (msg->msg_type == MSG_TYPE_OK) {
    msg_type = "OK";
  } else if (msg->msg_type == MSG_TYPE_IN_USE) {
    msg_type = "";
  } else if (msg->msg_type == MSG_TYPE_JOIN) {
    msg_type = "JOIN";
  } else if (msg->msg_type == MSG_TYPE_LEAVE) {
    msg_type = "LEAVE";
  } else if (msg->msg_type == MSG_TYPE_RENAME) {
    msg_type = "RENAME";
  }
  sprintf(text, "%c%s %s\r\n", sign, msg_type, msg->arg);
  global_send_some(client->global, text, msg->to_whom, client);
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

