/**
 * Operating System Simulator
 *
 * Copyright (c) 2017 G Brenden Roques
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "oss.h"

int main(int argc, char* argv[]) {
  int help_flag = 0;
  int c;

  while ((c = getopt(argc, argv, "h")) != -1) {
    switch (c) {
      case 'h':
        help_flag = 1;
        break;
      default:
        abort();
    }
  }

  if (help_flag) {
    print_help_message(argv[0]);
    exit(EXIT_SUCCESS);
  }
  return EXIT_SUCCESS;
}

/**
 * Prints a help message.
 * The parameters correspond to program arguments.
 */
static void print_help_message(char* executable_name) {
  printf("Operating System Simulator\n\n");
  printf("Usage: ./%s\n\n", executable_name);
  printf("Arguments:\n");
  printf(" -h  Show help.\n");
}