
#ifndef H_SERV_MSG
#define H_SERV_MSG

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

#endif
