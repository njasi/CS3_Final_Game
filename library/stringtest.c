#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARSE(t, d, n) \
  for (int i = 0; i < (n); i++) { \
    t = strtok(NULL, d); \
  }

int main(void) {
  int id;
  double x, y;
  char message[] = "ADDFORCE VECTOR[1,2] 69";
  const char *delim = " [,]\0";
  printf("COMMAND:\t%s\n", message);

  char *token = strtok(message, delim);
  if (strcmp(token, "ADDFORCE") == 0) {
    printf("Adding force...\n");
    PARSE(token, delim, 2);
    x = atof(token);
    PARSE(token, delim, 1);
    y = atof(token);
    PARSE(token, delim, 1);
    id = atoi(token);
  }

  printf("VECTOR\tX\t%f\tY\t%f\nOBJECT\t%d\n", x, y, id);
  return 0;
}
