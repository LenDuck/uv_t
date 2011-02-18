
#ifndef H_SERV_MSG
#define H_SERV_MSG

#include "con.h"

#define THREAD_STATE_DEAD (1)
#define THREAD_STATE_RUNNING (2)
#define THREAD_STATE_KILLNOW (3)
#define THREAD_STATE_ERROR (4)

typedef enum {
  CLIENT_STATE_CONNECTED = 1,
  CLIENT_STATE_LOGGED_IN = 2
} enum_client_state;

typedef enum {
  SERVER_STATE_CONNECTED = 1,
  SERVER_STATE_OPEN = 2,
  SERVER_STATE_FULL = 3
} enum_server_state;

#define SERVER_VERSION_MAYOR (1)
#define SERVER_VERSION_MINOR (0)
#define SERVER_VERSION_STRING "1.0"

#define SERVER_WELCOME "CHAT/1.0/\r\n"


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

typedef struct {
  char *arg;
  enum_status status;
  enum_msg_type msg_type;
} msg_t;

typedef struct {
  char *command, *msg;
} cmd_t;

msg_t* msg_get(con_t con);
int cmd_compare(char*, char*);
cmd_t* parse_command(char*);

#endif
