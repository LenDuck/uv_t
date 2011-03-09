#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "defines.h"

void print_cleaned(char *in){
  while (in && *in){
    char c = *in;
    in++;

    /*print all characters, anything not alphanumeric as %...*/
    if (isalnum(c)){
      printf("%c",c);
    } else {
      printf("%%%02x",c);
    }
  }
}

#define USAGE "\nProgram KEY (optional)from (optional)to (optional)inputfile\n"
int main(int argc,char **argv){

  char *key = NULL;
  char base_url[] = "https://www.googleapis.com/language/translate/v2?";
 
  char src[128] = {0};
  char dst[128] = {0};
  char longread[maxread + 1] = {0};
  int red = 0;

  if ((argc < 2) ||
      ((argc == 2) && (0 == strcasecmp("--help", argv[1])))
     ){
    fprintf(stderr,"\n%s\n",USAGE);
    return -1;
  }

  key = argv[1];

  if (argc > 2){
    strncpy(src,argv[2],128);
  } else {
    fprintf(stderr,"Please pick a source language (en,nl,ga,pl,yi,...)\n");
    scanf("%127s",src);
  }

  if (argc > 3){
    strncpy(dst, argv[3],128);
  } else {
    fprintf(stderr,"Please pick a destination language (en,nl,ga,pl,yi,...)\n");
    scanf("%127s",dst);
  }

  if (argc > 4){
    FILE *fin = fopen(argv[4],"rb");
    if ((!fin) || feof(fin)) return -1;
    red = fread(longread,1,maxread,fin);
    longread[red]=0;
  } else {
    fprintf(stderr,"Please enter the text to be translated, no more than %d characters allowed\n",maxread);
    fprintf(stderr,"Terminate the data with a ctrl-d / EOF please\n");
    red = fread(longread,1,maxread,stdin);
    longread[red] = 0;
  }
  /*
plainly print the target and source, only clean up the text before printing
  */

  /*
   * This url can be pasted in a browser to receive human readable output,
   * removing the callback=... part confused my browser....
   * */
  printf("%skey=%s&source=%s&target=%s&callback=handleResponse&q=",base_url,key,src,dst);
  print_cleaned(longread);
  return 0;

}

