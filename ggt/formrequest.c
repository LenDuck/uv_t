#include <stdio.h>
#include <ctype.h>

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

int main(int argc,char **argv){
#define maxread (1500)

  char *key = NULL;

  char base_url[] = "https://www.googleapis.com/language/translate/v2?";
  /*key=&q=flowers&source=en&target=fr&callback=handleResponse*/

 
  char src[128] = {0};
  char dst[128] = {0};
  char longread[maxread] = {0};
  int red = 0;

  if (argc == 2) {
    key = argv[1];
  } else {
    fprintf(stderr,"No key found, can't continue\n");
    return -1;
  }

  fprintf(stderr,"Please pick a source language (en,nl,ga,pl,yi,...)\n");
  scanf("%127s",src);

  fprintf(stderr,"Please pick a destination language (en,nl,ga,pl,yi,...)\n");
  scanf("%127s",dst);

  fprintf(stderr,"Please enter the text to be translated, no more than %d characters allowed\n",maxread-1);
  fprintf(stderr,"Terminate the data with a ctrl-d / EOF please\n");
  red = fread(longread,1,maxread-1,stdin);
  longread[red] = 0;
  
  /*plainly print the target and source, only clean up the text before printing*/
  printf("%skey=%s&source=%s&target=%s&callback=handleResponse&q=",base_url,key,src,dst);
  print_cleaned(longread);
  return 0;
#undef maxread
}
