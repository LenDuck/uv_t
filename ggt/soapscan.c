#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

int bloknaam_gelijk(char *a, char*b){
// fprintf(stderr, "compare: %s,%s,%d\n",a,b,strcasecmp(a,b)); 
  return 0 == strcasecmp(a,b);
}

/*
 * This scans for the following, where it prints whatever replaces the X's,
 * Neatly ignoring anything else.
 *
 * "translatedText": "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
 *
 * which is the core of all valid/usefull translations given by google.
 *
 * This might not be a full SOAP parser but it gets the job done,
 * although errors currently result in a blank screen, which should be 
 * interpeted as "no valid translation, though luck"
 *
 * ...The code is written containing a lot of Dutch...
 * */
int main(int argc, char **argv){
/*  int haakjes = 0;
  int accolades = 0;*/
  int dubbelpunt = 0;

  int gevonden = 0;
  char *doel = (argc == 2)?(argv[1]):("translatedText"); 

  int status = 0;
  char word[1024*1024] = {0};
  /*big buffer, more than google translate will return in a day...*/
  
  int word_index = 0;
  int reading = 0;

  while ( (status == 0)){
    
    /*scan a character*/
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
      dubbelpunt = 0;
    } else if (current == ')'){
      dubbelpunt = 0;
    } else if (current == '{'){
      dubbelpunt = 0;
    } else if (current == '}'){
      dubbelpunt = 0;
    } else if (current == ':'){
      dubbelpunt = 1;
    } /*else if ((current == '\r') ||(current == '\n')){
      dubbelpunt = 0;
    }*/
  }
  return 0;
}
