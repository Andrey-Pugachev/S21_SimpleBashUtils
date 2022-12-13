#include <stdio.h>
#include <getopt.h>
#include <string.h>
typedef struct Options {
  int b;
  int e;
  int n;
  int s;
  int t;
  int v;
} Options;
int main(int argc, char* argv[]) {
  if (argc == 1) {
    return 0;
  }
  Options options = {0};
  struct option opts[] = {
      {"number-nonblank", 0, NULL, 'b'},
      {"number", 0, NULL, 'n'},
      {"squeeze-blank", 0, NULL, 's'},
      {0, 0, 0, 0},
  };
  int opt;
  while ((opt = getopt_long(argc, argv, "+ebnstvTE", opts, NULL)) != -1) {
    switch (opt) {
      case 'b':
        options.b = 1;
        break;
      case 'e':
        options.e = 1;
        options.v = 1;
        break;
      case 'n':
        options.n = 1;
        break;
      case 's':
        options.s = 1;
        break;
      case 't':
        options.t = 1;
        options.v = 1;
        break;
      case 'v':
        options.v = 1;
        break;
      case 'T':
        options.t = 1;
        break;
      case 'E':
        options.e = 1;
        break;
      case '?':
        fprintf(stderr, "usage: s21_cat [-beEnstTuv] [number] [--number-nonblank] [squeeze-blank] [file ...]");
        break;
    }
  }
  argc -= optind;
  argv += optind;
  FILE* fin = NULL;
  for (int i = 0; argv[i] != NULL; i++) {
    fin = fopen(argv[i], "r");
    if (!fin) {
      fprintf(stderr, "s21_cat: %s: No such file or directory\n", argv[i]);
      continue;
    }
    char currentChar = '\0';
    char previousChar = '\n';
    int isMoreThenOneEmptyLine = 0;
    int lineCount = 0;
    while ((currentChar = fgetc(fin)) != EOF) {     
      if (options.s) {
        if (currentChar == '\n' && previousChar == '\n') {
          if (isMoreThenOneEmptyLine)
            continue;
          isMoreThenOneEmptyLine = 1;  
        }       
      }     
       if (options.b) {
        options.n = 0;
        if (previousChar == '\n' && currentChar != '\n')
          printf("%6d\t", ++lineCount);
      }      
      if (options.n) {
        if (previousChar == '\n')
          printf("%6d\t", ++lineCount);
      }
      if (options.e) {
        if (currentChar == '\n')
          printf("$");
      }
      if (options.t) {
        if (currentChar == '\t') {
          printf("^");
          currentChar = 'I';      
        }  
      } 
      if (options.v) {
        if ((currentChar >= 0 && currentChar <= 31 && currentChar != '\n' && currentChar != '\t') || currentChar == 127) {
          if (currentChar == 127) {
            printf("^");
            currentChar = currentChar - 64;
          } else {
            printf("^");
            currentChar = currentChar + 64;
          }
        }
      }
      if (currentChar != '\n')
        isMoreThenOneEmptyLine = 0;
      previousChar = currentChar;
      printf("%c", currentChar);
    }
  }
  fclose(fin);
  return 0;
}

