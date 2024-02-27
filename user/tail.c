#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "coinflip.h"

int main(int argc, char *argv[]) {
  if(argc > 1) {
    int verbose = 0;
    int num = 1; //1 = lines, 0 = bytes
    int flip = 0;
    int counter = -1; //number of bytes or lines

    for(int i=1; i < argc; i++) {
      if(strcmp(argv[i], "-c") == 0) {
        if(i + 1 < argc) {
          if(strcmp(argv[i+1], "0") == 0) {
            counter = 0;
            num = 0;
          } else {
            int numInput = atoi(argv[i+1]);

            if(numInput > 0 && numInput <= 10000) {
              counter = numInput;
              num = 0;
            } else {
              printf("ERROR: Number is required after '-c' flag and is limited to 10,000 characters\n");
              return 1;
            }

            i++;
          }
        } else {
          printf("ERROR: Number is required after '-c' flag\n");
          return 1;
        }
      } else if(strcmp(argv[i], "-n") == 0) {
        if(i + 1 < argc) {
          if(strcmp(argv[i+1], "0") == 0) {
            counter = 0;
            num = 1;
          } else {
            int numInput = atoi(argv[i+1]);

            if(numInput > 0 && numInput <= 1000) {
              counter = numInput;
              num = 1;
            } else {
              printf("ERROR: Number is required after '-n' flag and is limited to 1,000 lines\n");
              return 1;
            }

            i++;
          }
        } else {
          printf("ERROR: Number is required after '-n' flag\n");
          return 1;
        }
      } else if(strcmp(argv[i], "-v") == 0) {
        verbose = 1;
      } else if(strcmp(argv[i], "-f") == 0) {
        flip = 1;
      }
    }

    if(counter == -1) {
      counter = 10;
    }

    if(flip == 1) {
        int flipResult = coinflip();
        if (flipResult != -1) {
            return 0;
        }
    }

    for(int i=1; i < argc; i++) {
      int fd = open(argv[i], O_RDONLY);

      if (fd >= 0) {
          if (verbose == 1) {
              printf("==> %s <==\n", argv[i]);
          }
          uint sz = 0;
          char *buf;
          char **lines = malloc(1000 * sizeof(char *));
          int lineCount = 0;
          int byteCount = 0;
          int bytesRead = 0;
          while (lineCount < 1000) {
              bytesRead = getline(&buf, &sz, fd);
              if (bytesRead <= 0) {
                  break;
              }

              byteCount += bytesRead;
              lines[lineCount] = malloc(strlen(buf) + 1);
              strcpy(lines[lineCount], buf);

              lineCount++;
          }

          // For lines
          if (num == 1 && counter > 0) {
              if (counter > lineCount) {
                  counter = lineCount;
              }
              for (int i = lineCount - counter; i < lineCount; i++) {
                  printf("%s", lines[i]);
                  free(lines[i]);
              }
              free(lines);
              // printf("\n");

          // For bytes
          } else if (counter > 0) {
              if (byteCount >= 10000) {
                  byteCount = 10000;
              }
              if (counter > byteCount) {
                  counter = byteCount;
              }
              char line[10000];
              line[counter--] = '\0';
              for (int i = lineCount - 1; i >= 0 && counter >= 0; i--) {
                  for (int j = strlen(lines[i]) - 1; j >= 0 && counter >= 0; j--) {
                      line[counter--] = lines[i][j];
                  }
                  free(lines[i]);
              }
              free(lines);
              printf("%s", line);
              // printf("\n");
          }
      } else {
        if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-n") == 0) {
          i = i + 1;
        } else if(strcmp(argv[i], "-v") != 0 && strcmp(argv[i], "-f") != 0) {
          printf("ERROR: %s is not a file\n\n", argv[i]);
        }
      }
    }
  } else {
    printf("ERROR: Need arguments\n");
  }

  return 0;
}

