#ifndef H_TYPES
#define H_TYPES

typedef enum {
  CLIENT_STATE_CONNECTED = 1,
  CLIENT_STATE_AUTHED = 2,
  CLIENT_STATE_LOGGED_IN = 4,
}  enum_client_state;

typedef enum {
  SERVER_STATE_CONNECTED = 1,
  SERVER_STATE_OPEN = 2,
  SERVER_STATE_FULL = 3
} enum_server_state;

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

typedef struct struct_client_state{
  enum_client_state state;
  int thread_state;
  pthread_mutex_t access;
  pthread_t handler_thread;

  con_t connection;
  dlog_t *log;
  char *username;
  char *current_line;

  struct struct_global_state *global;  
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

typedef struct {
  char *arg;
  enum_status status;
  enum_msg_type msg_type;
} msg_t;

typedef struct {
  char *command, *msg;
} cmd_t;

#endif
