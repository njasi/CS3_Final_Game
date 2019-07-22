#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int **load_txt(char *filename) {
  int rows;
  int columns;

  FILE *f = fopen(filename, "r");
  fscanf(f, "%d %d", &rows, &columns);

  int** layout = malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    layout[i] = malloc(columns * sizeof(int));
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      fscanf(f, "%d", &layout[i][j]);
      //printf("%d\n", layout[i][j]);
    }
  }
  fclose(f);

  return layout;
}

// test it?
int main(int argc, char* argv[]) {
  load_txt(argv[1]);
  return 0;
}
