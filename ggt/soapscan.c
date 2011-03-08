#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

int bloknaam_gelijk(char *a, char*b){
// fprintf(stderr, "compare: %s,%s,%d\n",a,b,strcasecmp(a,b)); 
  return 0 == strcasecmp(a,b);
}

int main(int argc, char **argv){
  int haakjes = 0;
  int accolades = 0;
  int dubbelpunt = 0;

  int gevonden = 0;
  char *doel = (argc == 2)?(argv[1]):("translatedText");
  int status = 0;
  char word[1024*1024] = {0};
  /*big buffer, more than google translate will return in a day...*/
  
  int word_index = 0;
  int reading = 0;

  while ( (haakjes > -1) &&
          (accolades > -1) &&
          (status == 0)
      ){ /*scan a character*/
    int current = getchar();

    if (current == EOF){
      break;
    } else if (current == '"'){
      if (reading){
        /*read a word*/
        if (gevonden){
          printf("%s",word);
          break;
        } else if (bloknaam_gelijk(word,doel)){
          if (! dubbelpunt){
            gevonden = 1;
          }
        }
        word[0] = 0;
        word_index = 0;
      } else {
        /*start reading a word*/
      }
      reading = ~reading;
    } else if (reading){
      word[word_index++] = (char)current;
      word[word_index] = 0;
    } else if (current == '('){
      haakjes++;
    } else if (current == ')'){
      haakjes--;
    } else if (current == '{'){
      accolades++;
    } else if (current == '}'){
      accolades--;
    } else if (current == ':'){
      dubbelpunt = 1;
    } else if ((current == '\r') ||(current == '\n')){
      dubbelpunt = 0;
    }
  }
  return 0;
}
