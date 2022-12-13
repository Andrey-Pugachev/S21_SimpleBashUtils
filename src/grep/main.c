#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <regex.h>

typedef struct Options {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
} Options;

void addNewTemplate(char* templatesString, char* optarg); //Функцыя добавления нового считанного шаблона
void printProcessedByFlagsStrings(FILE* fileWhichSearchIn, char* currentFileName, Options options, regex_t regEx, int isSingleFile); //Функцыя по обработки и выводу совпавших строк

int main(int argc, char* argv[]) {
  char templatesString[4096] = {0};
  if (argc == 1) {
    return 0;
  }
  Options options = {0};
  int opt;
  while ((opt = getopt_long(argc, argv, "e:ivclnhsf:o", NULL, NULL)) != -1) {
    switch (opt) {
      case 'e': //Дополнительный шаблон (если шаблонов более одного то перед каждым должен стоять флаг -e) должен иметь после себя параметр для этого ставим :
        options.e = 1;
        addNewTemplate(templatesString, optarg);
        break;
      case 'i': //Игнорирует регистр шаблона
        options.i = 1;
        break;
      case 'v': //Выводит строки, в которых нет шаблона
        options.v = 1;
        break;
      case 'c': //Выводит только колличество совпадающих строк
        options.c = 1;
        break;
      case 'l': //Выводит только названия файлов (файлы идут в той последовательности как они были заданны в аргументаха)
        options.l = 1;
        break;
      case 'n': //Указывает номера строк с совпадениями ИмяФайла:НомерСтроки:Строка (файды идут в той последовательности как они были заданны в аргументаха)
        options.n = 1;
        break;
      case 'h': //Выводит совпавшые строки не предваряя их именами файлов (не отменяет флаг -n)
        options.h = 1;
        break;
      case 's': //Подавляет сообщение об ошибках о несуществующих или нечитаемых файлах (No such file or directory - не выводится ни в STDOUT ни в STDERR)
        options.s = 1;
        break;
      case 'f': //Получает шаблоны/регулярные выражения из файла или из опции флага -e: и тогда воспринемает просто шаблон как название файла
        options.f = 1;  
        FILE* templatesFile = fopen(optarg, "r");
        if (!templatesFile) {
          fprintf(stderr, "s21_grep: %s: No such file or directory\n", optarg);
          return 0;
        } else {
          char templateStringFromFile[4096] = {0};
          while (fgets(templateStringFromFile, 4095, templatesFile) != NULL) {
            for (char* p = templateStringFromFile; *p != '\0'; p++)
              if (*p == '\n') {
                *p = '\0';
                break;
              }
            addNewTemplate(templatesString, templateStringFromFile);
          }
        }
        fclose(templatesFile);
        break;
      case 'o':
        options.o = 1; //Выводит только совпавшый текст шаблона  (НазваниеФайла:Шаблон  или  НазваниеФайла:Шаблон  -  если шаблон встретился в строке несколько раз)
        break;
      case '?':
        fprintf(stderr, "usage: s21_grep [-e:ivclnhsf:o] template [file_name]"); //Сообщение выводится в случае не распознования флага
        return 0;
        break;
    }
  }
  argc -= optind;
  argv += optind;
  if (options.e == 0 && options.f == 0) { //Отработали наличие еденичного шаблона
    addNewTemplate(templatesString, argv[0]);
    argc--;
    argv++;
  }
  if (argc == 0) //Выход при отсутствии файлов для поиска
    return 0;
  int isSingleFile = (argc == 1) ? 1 : 0; //При наличии более чем одного файла, будет отображаться его имя
  regex_t regEx; //Создаём переменную для хранения наших шаблонов
  regcomp(&regEx, templatesString, options.i ? REG_EXTENDED|REG_ICASE : REG_EXTENDED); //Компилируем регулярные выражения
  FILE* fileWhichSearchIn = NULL;
  for (int i = 0; argv[i] != NULL; i++) {
    fileWhichSearchIn = fopen(argv[i], "r");
    if (!fileWhichSearchIn) {
      if (!options.s)
        fprintf(stderr, "s21_grep: %s: No such file or directory\n", argv[i]);
      continue;
    }
    printProcessedByFlagsStrings(fileWhichSearchIn, argv[i], options, regEx, isSingleFile);
  }
  fclose(fileWhichSearchIn);
  regfree(&regEx);
  return 0;
}

void addNewTemplate(char* templatesString, char* newTemplate) {
  if (*templatesString != '\0') {
    strcat(templatesString, "|");
    strcat(templatesString, newTemplate);
  } else
    strcat(templatesString, newTemplate);  
}

void printProcessedByFlagsStrings(FILE* fileWhichSearchIn, char* currentFileName, Options options, regex_t regEx, int isSingleFile) {
  char currentString[4096] = {0};
  int countOfStrings = 0;
  int countOfMatches = 0;
  int isMatchedString = 0;
  while (fgets(currentString, 4095, fileWhichSearchIn) != NULL) {
    for (char* p = currentString; *p != '\0'; p++) //Замена символа '\n' на символ '\0'
      if (*p == '\n') {
        *p = '\0';
        break;
      }
    countOfStrings++;
    isMatchedString = !(regexec(&regEx, currentString, 0, NULL, 0));
    if (isMatchedString && options.l) {
      if (options.c) {
        if (isSingleFile) {
          printf("1\n%s\n", currentFileName);
          return;
        } else {
          printf("%s:1\n%s\n", currentFileName, currentFileName);
          return;
        }
      } else {
        printf("%s\n", currentFileName);
        return;
      } 
    }
    if (options.v)
      isMatchedString = !isMatchedString;
    if (isMatchedString) {
      countOfMatches++;
      if (!options.c) {
        if (options.o && (!options.v) && (!options.l)) {
          regmatch_t caughtTemplate[1];
          char* tmpCurrentString = currentString;
          if (!isSingleFile)
            printf("%s:", currentFileName);
          while ((regexec(&regEx, tmpCurrentString, 1, caughtTemplate, 0)) == 0) {
            char stringForPrintcaughtTemplate[4096] = {0};
            printf("%s\n", strncpy(stringForPrintcaughtTemplate, tmpCurrentString + caughtTemplate[0].rm_so, caughtTemplate[0].rm_eo - caughtTemplate[0].rm_so));
            tmpCurrentString += caughtTemplate[0].rm_eo;
          }
        } else {
          if (isSingleFile) {
            if (options.n)
              printf("%d:%s\n", countOfStrings, currentString);
            else
              printf("%s\n", currentString);
          } else {
            if (options.h) {
              if (options.n)
                printf("%d:%s\n", countOfStrings, currentString);
              else
                printf("%s\n", currentString);
            } else {
              if (options.n)
                printf("%s:%d:%s\n", currentFileName, countOfStrings, currentString);
              else
                printf("%s:%s\n", currentFileName, currentString);
            }
          }
        }
      }
    }
  }
  if (options.c) {
    if (isSingleFile)
      printf("%d\n", countOfMatches);
    else {
      if (options.h)
        printf("%d\n", countOfMatches);
      else  
        printf("%s:%d\n", currentFileName, countOfMatches);
    }  
  }
}

