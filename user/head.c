#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// Function prototypes
void display_usage();
void parse_arguments(int argc, char *argv[], int *verbose, int *num, int *counter, int *flip, int*file);
void display_user_input(int num, int counter);
void display_file_contents(const char *filename, int verbose, int num, int counter, int *i);
int coinflip();

int main(int argc, char *argv[]) {
  // Initialize variables
  int verbose = 0;
  int num = 1; // 1 = lines, 0 = bytes
  int flip = 0;
  int counter = -1; // Number of bytes or lines
  int file = 0;

  // Parse command-line arguments
  parse_arguments(argc, argv, &verbose, &num, &counter, &flip, &file);
  if (counter == -1) {
    counter = 10;
  }

  // Flip a coin if the -f flag is provided
  if (flip == 1) {
    int flipResult = coinflip();
    if (flipResult != 1) {
      if (flipResult == -1) fprintf(2, "Try flipping again for heads!\n");
      return 0;
    }
  }

  if(file == 0) {
    display_user_input(num, counter);
    return 0;
  }

  // Look for a file to read and display its contents depending on arguments given
  for (int i = 1; i < argc; i++) {
    display_file_contents(argv[i], verbose, num, counter, &i);
  }

  return 0;
}

void display_usage() {
  printf("Calling head returns the first 10 lines of the first file found in the input arguments. Adding flags as arguments will modify the output.\n\n");
  printf("\tFlags:\n");
  printf("\t\t-n [num]\tInstead of returning the first 10 lines, return the first num amount of lines. This flag must have a number after it (max 1,000).\n\n");
  printf("\t\t-c [num]\tReturns the first num amount of bytes. This flag must have a number after it (max 10,000).\n\n");
  printf("\t\tNOTE\t\tIt will take the last `-n` or `-c` that's given. So if you input `-n 3` and `-c 5`, it would disregard `-n 3` and take `-c 5`\n\n");
  printf("\t\t-v\t\tDisplays the file name before displaying all the lines or bytes.\n\n");
  printf("\t\t-f\t\tFlips a coin that determines whether or not head will work or not. There is 40% chance for head to work properly.\n");
  exit(0);
}

// Function to parse command-line arguments
void parse_arguments(int argc, char *argv[], int *verbose, int *num, int *counter, int *flip, int *file) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      // Display usage
      display_usage();
      exit(0);
    } else if(strcmp(argv[i], "-c") == 0) {
      if(i + 1 < argc) {
        if(strcmp(argv[i+1], "0") == 0) {
          *counter = 0;
          *num = 0;
        } else {
          int numInput = atoi(argv[i+1]);

          if(numInput > 0 && numInput <= 10000) {
            *counter = numInput;
            *num = 0;
          } else {
            fprintf(2, "ERROR: Number is required after '-c' flag and is limited to 10,000 characters\n");
            exit(1);
          }

          i++;
        }
      } else {
        fprintf(2, "ERROR: Number is required after '-c' flag\n");
        exit(1);
      }
    } else if(strcmp(argv[i], "-n") == 0) {
      if(i + 1 < argc) {
        if(strcmp(argv[i+1], "0") == 0) {
          *counter = 0;
          *num = 1;
        } else {
          int numInput = atoi(argv[i+1]);

          if(numInput > 0 && numInput <= 1000) {
            *counter = numInput;
            *num = 1;
          } else {
            fprintf(2, "ERROR: Number is required after '-n' flag and is limited to 1,000 lines\n");
            exit(1);
          }

          i++;
        }
      } else {
        fprintf(2, "ERROR: Number is required after '-n' flag\n");
        exit(1);
      }
    } else if(strcmp(argv[i], "-v") == 0) {
      *verbose = 1;
    } else if(strcmp(argv[i], "-f") == 0) {
      *flip = 1;
    } else {
      *file = 1;
    }
  }
}
// Function to display user input based on specified flags
void display_user_input(int num, int counter) {
  char *buf = 0;
  uint sz = 0;

  if(num == 1 && counter > 0) { //for lines
    for(int n = 0; n < counter; n++) {
      if(getline(&buf, &sz, 0) <= 0) {
        break;
      }
      printf("%s", buf);
    }
  } else if(counter > 0) { //for bytes
    int bytes = 0;

    while(counter > 0) {
      int lineBytes = getline(&buf, &sz, 0);

      bytes += lineBytes;

      if(lineBytes <= 0) {
        break;
      }

      for(int i=0; i < lineBytes && counter > 0 ; i++) {
        counter--;
        printf("%c", buf[i]);
      }
    }
  }
}

// Function to display file contents based on specified flags
void display_file_contents(const char *filename, int verbose, int num, int counter, int *i) {
  // Open the file, read and display contents based on flags
  int fd = open(filename, O_RDONLY);

  if(fd >= 0) {
    if(verbose == 1) {
      printf("==> %s <==\n", filename);
    }

    char *buf = 0;
    uint sz = 0;
    if(num == 1 && counter > 0) { //for lines
      for(int n = 0; n < counter; n++) {
        if(getline(&buf, &sz, fd) <= 0) {
          break;
        }
        printf("%s", buf);
      }
    } else if(counter > 0) { //for bytes
      int bytes = 0;

      while(counter > 0) {
        int lineBytes = getline(&buf, &sz, fd);

        bytes += lineBytes;

        if(lineBytes <= 0) {
          break;
        }

        for(int i=0; i < lineBytes && counter > 0 ; i++) {
          counter--;
          printf("%c", buf[i]);
        }
      }
    }
  } else {
    if(strcmp(filename, "-c") == 0 || strcmp(filename, "-n") == 0) {
      *i = *i + 1;
    } else if(strcmp(filename, "-v") != 0 && strcmp(filename, "-f") != 0) {
      fprintf(2, "ERROR: %s is not a file\n", filename);
    }
  }
}

// Function to flip a coin
int coinflip() {
  uint64 result = (timeframe() / 1000) % 10;

  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⡿⢿⣿⣿⣿⣟⣛⣛⣛⣉⣉⣛⠛⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣟⡵⠞⠉⠀⢈⣿⣿⣿⠿⠟⠛⠛⠛⠻⡆⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⡀⣀⣠⣴⣿⣿⣿⠁⣶⣿⣿⣿⠿⠛⣡⡋⠹⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣤⣭⣥⣤⣶⣾⣿⣿⠀⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣇⣾⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠁⠈⠉⠉⢻⣿⡇⠀⠙⢿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠶⣶⣶⣿⣿⣿⣄⠀⠀⠈⢻⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣤⣀⣀⡀⣹⡉⠙⠻⠀⠀⠀⠀⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠉⠉⠙⣿⡇⠀⠀⠀⠀⠀⠀⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠓⠶⠶⠤⣿⡇⠀⠀⠀⠀⠀⠀⠻⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣶⣤⣉⣁⣀⣀⡀⠀⠀⠀⠀⠀⠀⢸⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣶⣶⣶⣶⣶⣾⣿\n\n\n\n\n");

  sleep(10);
  printf("⣿⣿⣿⣿⣿⡿⢿⣿⣿⣿⣟⣛⣛⣛⣉⣉⣛⠛⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣟⡵⠞⠉⠀⢈⣿⣿⣿⠿⠟⠛⠛⠛⠻⡆⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⡀⣀⣠⣴⣿⣿⣿⠁⣶⣿⣿⣿⠿⠛⣡⡋⠹⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣤⣭⣥⣤⣶⣾⣿⣿⠀⣿⣿⣿⣿⣿⣿⣿⣿\n\n\n\n\n");
  sleep(10);
  printf("⣿⣿⣿⣿⣿⡿⢿⣿⣿⣿⣟⣛⣛⣛⣉⣉⣛⠛⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣟⡵⠞⠉⠀⢈⣿⣿⣿⠿⠟⠛⠛⠛⠻⡆⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⡀⣀⣠⣴⣿⣿⣿⠁⣶⣿⣿⣿⠿⠛⣡⡋⠹⣿⣿⣿⣿⣿⣿⣿⣿\n");
  printf("⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣤⣭⣥⣤⣶⣾⣿⣿⠀⣿⣿⣿⣿⣿⣿⣿⣿\n\n\n\n\n");
  sleep(10);

  if (result < 4) {
    printf(" /$$                                 /$$           /$$\n");
    printf("| $$                                | $$          | $$\n");
    printf("| $$$$$$$   /$$$$$$   /$$$$$$   /$$$$$$$  /$$$$$$$| $$\n");
    printf("| $$__  $$ /$$__  $$ |____  $$ /$$__  $$ /$$_____/| $$\n");
    printf("| $$  \\ $$| $$$$$$$$  /$$$$$$$| $$  | $$|  $$$$$$ |__/\n");
    printf("| $$  | $$| $$_____/ /$$__  $$| $$  | $$ \\____  $$    \n");
    printf("| $$  | $$|  $$$$$$$|  $$$$$$$|  $$$$$$$ /$$$$$$$/ /$$\n");
    printf("|__/  |__/ \\_______/ \\_______/ \\_______/|_______/ |__/\n\n\n\n\n");
    return 1;
  } else if (result < 8) {
    printf("   /$$               /$$ /$$           /$$\n");
    printf("  | $$              |__/| $$          | $$\n");
    printf(" /$$$$$$    /$$$$$$  /$$| $$  /$$$$$$$| $$\n");
    printf("|_  $$_/   |____  $$| $$| $$ /$$_____/| $$\n");
    printf("  | $$      /$$$$$$$| $$| $$|  $$$$$$ |__/\n");
    printf("  | $$ /$$ /$$__  $$| $$| $$ \\____  $$    \n");
    printf("  |  $$$$/|  $$$$$$$| $$| $$ /$$$$$$$/ /$$\n");
    printf("   \\___/   \\_______/|__/|__/|_______/ |__/\n\n\n\n\n");
    return -1;
  }

  printf("  /$$$$   /$$$$  /$$\n");
  printf(" /$$  $$ /$$  $$| $$\n");
  printf("    /$$/    /$$/| $$\n");
  printf("   /$$/    /$$/ |__/\n");
  printf("  |__/    |__/\n");
  printf("   /$$     /$$   /$$\n");
  printf("  |__/    |__/  |__/\n");
  printf("                    \n");
  printf("                    \n\n\n\n\n");
  printf("Oops! Looks like the coin got stuck in a quantum loop between heads and tails. Let's try flipping again and hope the universe makes up its mind this time!\n");
  return 0;
}

