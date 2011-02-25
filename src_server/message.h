
#ifndef H_SERV_MSG
#define H_SERV_MSG

#include "server.h"
#include "types.h"

#define THREAD_STATE_DEAD (1)
#define THREAD_STATE_RUNNING (2)
#define THREAD_STATE_KILLNOW (3)
#define THREAD_STATE_ERROR (4)

#define SERVER_VERSION_MAYOR (1)
#define SERVER_VERSION_MINOR (0)
#define SERVER_VERSION_STRING "1.0"

#define SERVER_WELCOME "CHAT/1.0/\r\n"

int msg_get(client_state_t *client);
void send_msg(msg_t*, client_state_t *client);
int cmd_compare(char*, char*);
cmd_t* parse_command(char*);
int mask_matches(enum_client_state in, enum_client_state flags);
int init_client(client_state_t *client);
#endif
