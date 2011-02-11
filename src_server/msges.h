#ifndef H_MSGES
#define H_MSGES
#include <server.h>

typedef enum {
  MSG_TYPE_CHAT_CLIENT,
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
  char *text;
  enum_status status;
  enum_msg_type msg_type;
} msg_t;

typedef struct {
  char *command, *msg;
} cmd_t;

msg_t* msg_get(con_t con);
#endif

