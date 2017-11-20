#ifndef OSS_H
#define OSS_H

static void parse_command_options(int argc, char* argv[]);
static void print_help_message(char* executable_name);
static void free_shm_and_abort(int signum);
static void free_shm();
static int setup_interrupt();

#endif