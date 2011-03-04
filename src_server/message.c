#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "server.h"
#include "message.h"
/*
  Returns 0 if OK, 1 if *any* error!
*/
int msg_get(client_state_t *client) {
  con_t con = client->connection;
  client_list_t *clients = client->global->clients;
  client_state_t *curr_client;
  char *buffer = NULL;
  unsigned int textsize = 128;
  char *text = malloc(textsize);
  //printf("sr\n");
  int status = con_line(con, &buffer);
  //printf("srozly: %s\n",buffer);
  //fflush(stdout);

  /*Sanity check for input, len<4 is nonsense (say ,the shortest message*/
  if ( (status != CON_ERROR_NONE) || (!buffer) || (4 > strlen(buffer))){
    free(buffer);
    return (status != CON_ERROR_NONE);/*Not an error, just some noise, closed?*/

  }
  if (status != CON_ERROR_NONE) {
    printf("ERROR! No. : %d\r\n", status);
    return 2;
  }
  if (!(text)) return 1;
  cmd_t *command = parse_command(buffer);
  msg_t *msg = malloc(sizeof(msg_t));
  if (!(msg && command && command->command)) return 1;
  if (cmd_compare(command->command, "CHAT")) {
    if (client->state & CLIENT_STATE_CONNECTED) {
      /* Received "CHAT", client auths, send "+OK" */
      msg->arg = "";
      msg->status = STATUS_POS;
      msg->msg_type = MSG_TYPE_CHAT;
      send_msg(msg, client);
      client->state |= CLIENT_STATE_AUTHED;
    } else {
      return -1;
    }
  } else if(cmd_compare(command->command, "USER")) {
    if (client->state & CLIENT_STATE_AUTHED) {
      /* Received "USER", check in users_list if exists, if no match, reg.*/
      int found = 0;
      while(clients) {
	curr_client = clients->current;
	if ((curr_client) && (curr_client->username) && (cmd_compare(curr_client->username, command->msg))) {
	  // Match in list: return neg. msg.
	  found = 42;
	  break;
	}
	clients = clients->next;
      }
      msg->status = (found) ? STATUS_NEG : STATUS_POS;
      msg->msg_type = (found) ? MSG_TYPE_IN_USE : MSG_TYPE_OK;
      msg->arg = (found) ? "Username in use" : "";
      /*
	Send all other clients RENAME or JOIN msg.
      */
      if (!(found)) {
	if (client->state & CLIENT_STATE_LOGGED_IN) {
	  msg_t *mesg = malloc(sizeof(msg_t));
	  mesg->arg = malloc(strlen(curr_client->username) + strlen(command->msg) + 4);
	  sprintf(mesg->arg, "%s/%s\r\n", curr_client->username, command->msg);
	  mesg->msg_type = MSG_TYPE_RENAME;
	  mesg->status = STATUS_POS;
	  send_msg(mesg, client);
	} else if (client->state & CLIENT_STATE_AUTHED) { 
	  msg_t *mesg = malloc(sizeof(msg_t));
	  mesg->arg = malloc(strlen(curr_client->username) + strlen(command->msg) + 4);
	  sprintf(mesg->arg, "%s\r\n", command->msg);
	  mesg->msg_type = MSG_TYPE_JOIN;
	  mesg->status = STATUS_POS;
	  send_msg(mesg, client);
	}
      }
      client->username = (found) ? client->username : command->msg;
      client->state |= (found) ? client->state : CLIENT_STATE_LOGGED_IN;
      send_msg(msg, client);
    } else {
      return -1;
    }
  } else if(cmd_compare(command->command, "NAMES")) {
    /* Received "NAMES"*/
    if (client->state & CLIENT_STATE_LOGGED_IN) {
      while(clients) {
	curr_client = clients->current;
	if (curr_client)
	{
	  if (curr_client->state & CLIENT_STATE_LOGGED_IN)
	  {
	    if (strlen(text) < textsize - 2) {
	      sprintf(text, "%s%s\r\n", text, curr_client->username);
	    } else {
	      text = realloc(text, textsize += 128);
	      sprintf(text, "%s%s\r\n", text, curr_client->username);
	    }
	  }
	  clients = clients->next;
	}
      }
      msg->arg = text;
      msg->status = STATUS_POS;
      msg->msg_type = MSG_TYPE_NAMES;
      send_msg(msg, client);
    } else {
      return -1;  
    }
  } else if(cmd_compare(command->command, "SAY")) {
    /* Received "SAY"*/
    if (client->state & CLIENT_STATE_LOGGED_IN) {
      msg->arg = malloc(strlen(command->msg) + strlen(client->username) + 4);
      if (! msg->arg) return 1;
      sprintf(msg->arg, "%s/%s\r\n", client->username, command->msg);
      msg->status = STATUS_POS;
      msg->msg_type = MSG_TYPE_SAY;
      send_msg(msg, client);
    } else {
      return -1;
    }
  } else {
    return -1;
  }
  free(buffer);
  return 0;
}

void send_msg(msg_t *msg, client_state_t *client) {
  char *text = malloc(32 + strlen(msg->arg) + 1);
  char sign = (msg->status == STATUS_POS) ? '+' : '-';
  char *msg_type = malloc(7);
  if (msg->msg_type == MSG_TYPE_CHAT) {
    msg_type = "CHAT";
    sprintf(text, "%c%s %s\r\n", sign, msg_type, msg->arg);
    con_send_line(client->connection, text);
  } else if (msg->msg_type == MSG_TYPE_OK) {
    msg_type = "OK";
    sprintf(text, "%c%s %s\r\n", sign, msg_type, msg->arg);
    con_send_line(client->connection, text);
  } else if (msg->msg_type == MSG_TYPE_IN_USE) {
    sprintf(text, "%c %s\r\n", sign, msg->arg);
    con_send_line(client->connection, text);
  } else if (msg->msg_type == MSG_TYPE_NAMES) {
    sprintf(text, "%c\r\n%s\r\n", sign, msg->arg);
    con_send_line(client->connection, text);
  } else if (msg->msg_type == MSG_TYPE_SAY) {
    msg_type = "SAY";
    sprintf(text, "%s %s\r\n", msg_type, msg->arg);
    global_send_logged_in(client->global, text);
  } else if (msg->msg_type == MSG_TYPE_LEAVE) {
    msg_type = "LEAVE";
    sprintf(text, "%s %s\r\n", msg_type, msg->arg);
    global_send_others(client->global, text, client);
  } else if (msg->msg_type == MSG_TYPE_JOIN) {
    msg_type = "JOIN";
    sprintf(text, "%s %s\r\n", msg_type, msg->arg);
    global_send_others(client->global, text, client);
  } else if (msg->msg_type == MSG_TYPE_RENAME) {
    msg_type = "RENAME";
    sprintf(text, "%s %s\r\n", msg_type, msg->arg);
    global_send_others(client->global, text, client);
  } 
  free(text);
}

int cmd_compare(char *a, char *b)
{
  int x, y;
  x = 0 == strcmp(a, b);
  y = 0 == strcasecmp(a, b);
  return x + y;
}

cmd_t* parse_command(char *buffer) {
  unsigned int i = 0, j = 0, size1 = 32, size2 = 32;
  char *command = malloc(size1), *text = malloc(size2);
  
  while(buffer[i] && (buffer[i] != ' ') && (i < strlen(buffer))) {
    if (i < size1 - 1) {
      command[i] = buffer[i];
      command[i+1] = 0;
    } else {
      command = realloc(command, size1 *= 2);
      command[i] = buffer[i];
      command[i+1] = 0;
    }
    i++;
  }
  command[i] = 0;
    while(buffer[i]) {
    if (j < size2 - 1) {
      char c = buffer[i];
      if (((!(c == '\r' || c == '\n')) && (c < ' ')) || (c == 127)) {
        printf("Error: wrong characters!\n");
        return NULL;
      }
      text[j] = c;
      text[j+1] = 0;
    } else {
      command = realloc(text, size2 *= 2);
      char c = buffer[i];
      if (((!(c == '\r' || c == '\n')) && (c < ' ')) || (c == 127)) {
        printf("Error: wrong characters!\n");
        return NULL;
      }
      text[j] = c;
      text[j+1] = 0;
    }
    i++;
    j++;
  }
  text[j] = 0;
  cmd_t *rvl = malloc(sizeof(cmd_t));
  rvl->command = command;
  rvl->msg = text;
  return rvl;
}

