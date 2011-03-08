#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include "chat.h"
#include "dlogger.h"
int main(int argc, char **argv) {
  char server[] = "localhost";
  char port[] = "55555";
  char *response = NULL;
  char user[] = "duck_test";
  chat_con_t *con = NULL;
  dlog_t *mainlog = NULL;
  
  mainlog = dlog_newlog_file("Client log", "main_client", DLOG_FLAG_FFLUSH  |DLOG_FLAG_NLINE);

  /*dlog_log_text("?",mainlog);*/
  dlog_log_text("Bootup start",mainlog);
  
  /*Start a valid connection*/
  con = chat_bootup(server,port);
  dlog_log_text("Bootup done",mainlog);  
  fprintf(stderr,"Going\n");
  usleep(1000);

  /*Test here*/
  if (chat_set_user(con,user)){
    dlog_log_text("Username denied",mainlog);  
  }

  if (chat_say(con,"Quote me and be recursive")){
    dlog_log_text("Say failed",mainlog);  
  }
  
  /*Here pleaze, insert code*/

  dlog_log_text("Main done, closing", mainlog);
  /*dirc_close(con);*/
  if (chat_quit(con)){
    dlog_log_text("Closing failed",mainlog);  
  }

  
  dlog_log_text("Main done, closing log",mainlog);
  dlog_close_log(mainlog);
  mainlog = NULL;
  fprintf(stderr,"done\n");

  (void)argc;
  (void)argv;

  return 0;
}

