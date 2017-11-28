/**
 * Operating System Simulator
 *
 * Copyright (c) 2017 G Brenden Roques
 */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "oss.h"
#include "lib/myclock.h"
#include "lib/stats.h"
#include "lib/sem.h"
#include "lib/shm.h"

#define INIT_VAL -10

int should_run = 1;

static FILE* log;
int verbose = 0;

// Shared Memory Globals
static int clock_id;
static my_clock* clock_shm;

static int page_tables_id;
static page* page_tables;

static int mem_ops_id;
static mem_op_t* mem_ops;

// For proteting the clock
static int clock_sem_id;

// For making memory references
static int mem_sem_ids[MAX_PROCS];

static char unallocated_frames[TOTAL_PAGES];

static stats_t stats[MAX_PROCS];

pid_t children[MAX_PROCS];

int main(int argc, char* argv[]) {
  srand(time(0));

  parse_command_options(argc, argv);

  setup_interrupt_handler();
  setup_interval_timer(2);  // 2 seconds
  signal(SIGALRM, handle_timer_interrupt);
  signal(SIGCHLD, handle_child_termination);

  open_log_file();

  setup_data_structures();

  fprintf(stderr, "Running oss. See oss.out for log.\n");

  fork_and_exec_children();

  // Break out of loop after timer interrupt
  while (should_run) {
    check_for_mem_requests();
  }

  wait_for_all_children();

  free_shm();

  return EXIT_SUCCESS;
}

static void parse_command_options(int argc, char* argv[]) {
  int help_flag = 0;
  int c;

  while ((c = getopt(argc, argv, "hv")) != -1) {
    switch (c) {
      case 'h':
        help_flag = 1;
        break;
      case 'v':
        verbose = 1;
        break;
      default:
        abort();
    }
  }

  if (help_flag) {
    print_help_message(argv[0]);
    exit(EXIT_SUCCESS);
  }
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
  printf(" -v  Verbose log output.\n");
}

static void setup_data_structures() {
  clock_id = get_clock_shm();
  clock_shm = attach_to_clock_shm(clock_id);
  clock_shm->secs = 1;

  page_tables_id = get_page_tables();
  page_tables = attach_to_page_tables(page_tables_id);
  setup_page_tables();

  setup_unallocated_frames();

  mem_ops_id = get_mem_ops(MAX_PROCS);
  mem_ops = attach_to_mem_ops(mem_ops_id);
  setup_mem_ops(mem_ops);

  setup_clock_sem();

  setup_mem_sems();
}

static void setup_unallocated_frames() {
  int i = 0;
  for (; i < TOTAL_PAGES; i++) {
    unallocated_frames[i] = 0;
  }
}

static void open_log_file() {
  log = fopen("oss.out", "w");

  if (log == NULL) {
    perror("Failed to open log file");
    exit(EXIT_FAILURE);
  }
}

static void setup_page_tables() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    int j = 0;
    for (; j < NUM_FRAMES; j++) {
      page* pg = get_page(page_tables, i, j);
      reset_page(pg);
    }
  }
}

static void reset_page(page* pg) {
  pg->num = INIT_VAL;
  pg->valid = 0;
  pg->dirty = 0;
}

/**
 * Free shared memory and abort program
 */
static void free_shm_and_abort(int signum) {
  free_shm();
  abort();
}

/**
 * Frees all allocated shared memory
 */
static void free_shm() {
  detach_from_clock_shm(clock_shm);
  shmctl(clock_id, IPC_RMID, 0);

  detach_from_page_tables(page_tables);
  shmctl(page_tables_id, IPC_RMID, 0);

  detach_from_mem_ops(mem_ops);
  shmctl(mem_ops_id, IPC_RMID, 0);

  deallocate_sem(clock_sem_id);

  deallocate_mem_sems();
}

/**
 * Set up the interrupt handler.
 */
static void setup_interrupt_handler() {
  struct sigaction act;
  act.sa_handler = free_shm_and_abort;
  act.sa_flags = 0;
  int return_value = (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
  if (return_value == -1) {
    perror("Failed to set up handler for SIGPROF");
    exit(EXIT_FAILURE);
  }
}

/**
 * Sets up an interval timer
 *
 * @param time The duration of the timer in seconds
 */
static void setup_interval_timer(int time) {
  struct itimerval value;
  value.it_interval.tv_sec = time;
  value.it_interval.tv_usec = 0;
  value.it_value = value.it_interval;
  int return_value = setitimer(ITIMER_REAL, &value, NULL);
  if (return_value == -1) {
    perror("Faled to set up interval timer");
    exit(EXIT_FAILURE);
  }
}

/**
 * Handle the timer intterupt.
 *
 * Stops the programming from running.
 */
static void handle_timer_interrupt() {
  should_run = 0;
}

static void fork_and_exec_children() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    fork_and_exec_child(i);
  }
}

static void handle_child_termination(int signum) {
  int status;
  pid_t pid;

  pid = wait(&status);
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    if (children[i] == pid) {
      break;
    }
  }
  children[i] = INIT_VAL;
  fprintf(log,
          "PID %d terminating. Freeing memory\n\n",
          i);
  free_memory(i);
  stats[i].end_time.secs     = clock_shm->secs;
  stats[i].end_time.nanosecs = clock_shm->nanosecs;

  print_stats_report(i);
}

static int get_num_procs_completed() {
  int i = 0;
  int num_procs_completed = 0;
  for (; i < MAX_PROCS; i++) {
    if (children[i] == INIT_VAL) {
      num_procs_completed++;
    }
  }
  return num_procs_completed;
}

static void free_memory(int pid) {
  int i = 0;
  do {
    page* pg = get_page(page_tables, pid, i);
    reset_page(pg);
    i++;
  } while (i < NUM_FRAMES);
}

static void print_stats_report(int pid) {
  int title_length = 22;
  fprintf(log, "Process %d Stats Report\n", pid);
  print_stats_report_separator(title_length);

  my_clock start = stats[pid].start_time;
  my_clock end = stats[pid].end_time;
  int num_page_faults = stats[pid].num_page_faults;
  int mem_accesses = stats[pid].num_mem_accesses;

  int secs_lived = end.secs - start.secs;
  int mem_accesses_per_sec = mem_accesses / secs_lived;
  int page_faults_per_mem_access = num_page_faults * 100 / mem_accesses;
  unsigned int avg_mem_access_speed = get_avg_mem_access_speed(mem_accesses,
                                                               num_page_faults);
  int num_procs_completed = get_num_procs_completed();
  double throughput = (double) num_procs_completed / (double) clock_shm->secs;
  fprintf(log, "Start Time: %d:%d\n", start.secs, start.nanosecs);
  fprintf(log, "End Time: %d:%d\n", end.secs, end.nanosecs);
  fprintf(log, "Number of Memory Accesses: %d\n", mem_accesses);
  fprintf(log, "Number of Page Faults: %d\n", num_page_faults);
  fprintf(log, "Memory Accesses per Second: %d\n", mem_accesses_per_sec);
  fprintf(log, "Page Faults per Memory Access: %d%%\n", page_faults_per_mem_access);
  fprintf(log, "Average Memory Acess Speed: %d millseconds\n", avg_mem_access_speed);
  fprintf(log, "Throughput: %f processes per second\n", throughput);
  print_stats_report_separator(title_length);
  fprintf(log, "\n");
}

static void print_stats_report_separator(int length) {
  int i = 0; for (; i < length; i++) fprintf(log, "-");
  fprintf(log, "\n");
}

static unsigned int get_avg_mem_access_speed(int mem_accesses, int page_faults) {
  unsigned int mem_access_time = (mem_accesses - page_faults) * 10;
  unsigned int page_fault_time = page_faults * 15 * NANOSECS_PER_MILLISEC;
  unsigned int total_time = mem_access_time + page_fault_time;
  unsigned int avg_time_in_nanosecs = total_time / mem_accesses;
  return avg_time_in_nanosecs / NANOSECS_PER_MILLISEC;
}

/**
 * Forks and execs a child process.
 * 
 * @param pid Simulated PID of child
 */
static void fork_and_exec_child(int pid) {
  stats[pid].start_time.secs     = clock_shm->secs;
  stats[pid].start_time.nanosecs = clock_shm->nanosecs;
  children[pid] = fork();

  if (children[pid] == -1) {
    perror("Failed to fork");
    exit(EXIT_FAILURE);
  }

  if (children[pid] == 0) {  // Child
    char pid_str[12];
    snprintf(pid_str,
             sizeof(pid_str),
             "%d",
             pid);

    char clock_id_str[12];
    snprintf(clock_id_str,
             sizeof(clock_id_str),
             "%d",
             clock_id);

    char clock_sem_id_str[12];
    snprintf(clock_sem_id_str,
             sizeof(clock_sem_id_str),
             "%d",
             clock_sem_id);

    char mem_ops_id_str[12];
    snprintf(mem_ops_id_str,
             sizeof(mem_ops_id_str),
             "%d",
             mem_ops_id);

    char mem_sem_id_str[12];
    snprintf(mem_sem_id_str,
             sizeof(mem_sem_id_str),
             "%d",
             mem_sem_ids[pid]);

    execlp("user",
           "user",
           pid_str,
           clock_id_str,
           clock_sem_id_str,
           mem_ops_id_str,
           mem_sem_id_str,
           (char*) NULL);
    perror("Failed to exec");
    _exit(EXIT_FAILURE);
  }
}

static void check_for_mem_requests() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    if (has_mem_request(mem_ops[i])) {
      handle_mem_request(i, (mem_ops + i));
      if (verbose) print_page_table(i);
      if (should_run_page_replacement(i)) {
        run_page_replacement(i);
      }
      if (verbose) print_page_table(i);
    }
  }
}

static int has_mem_request(mem_op_t mem_op) {
  return mem_op.addr != INIT_VAL;
}

static void handle_mem_request(int pid, mem_op_t* mem_op) {
  int page_num = get_page_num(mem_op->addr);

  int is_full = is_page_table_full(pid);
  int is_in_memory = is_page_in_memory(pid, page_num);

  print_received_memory_request(mem_op->op, pid, page_num);

  page* pg;
  if (is_in_memory) {  // Set valid bit to 1
    int i = find_page(pid, page_num);
    pg = get_page(page_tables, pid, i);
    int has_been_a_second = update_clock(clock_shm, 10);
    if (has_been_a_second && !verbose) {
      print_page_tables();
    }
  } else if (!is_full && !is_in_memory) {  // Set frame number
    int i = get_next_available_page_table_index(pid);
    pg = get_page(page_tables, pid, i);
    pg->num = page_num;
    int k = pid * NUM_FRAMES + i;
    unallocated_frames[k] = 1;
    unsigned int fifteen_millisecs = 15 * NANOSECS_PER_MILLISEC;
    int has_been_a_second = update_clock(clock_shm, fifteen_millisecs);
    if (has_been_a_second  && !verbose) {
      print_page_tables();
    }
    stats[pid].num_page_faults++;
  }
  pg->valid = 1;

  if (mem_op->op == WRITE) {
    pg->dirty = 1;
  }

  mem_op->addr = INIT_VAL;
  stats[pid].num_mem_accesses++;
  sem_post(mem_sem_ids[pid]);
}

static void print_received_memory_request(io_op op, int pid, int page_num) {
  if (verbose) {
    char* op_str = op == READ ? "read" : "write";
    fprintf(log,
            "Received PID %d request to %s page %d\n\n",
            pid,
            op_str,
            page_num);
  }
}

static int is_page_in_memory(int pid, int page_num) {
  page* pg = get_page(page_tables, pid, 0);
  int is_in_memory = 0;
  int i = 0;
  for (; i < NUM_FRAMES; i++) {
    if ((pg + i)->num == page_num) {
      is_in_memory = 1;
    }
  }
  return is_in_memory;
}

static int is_page_table_full(int pid) {
  page* pg = get_page(page_tables, pid, 0);
  int is_full = 1;
  int i = 0;
  for (; i < NUM_FRAMES; i++) {
    if ((pg + i)->num == INIT_VAL) {
      is_full = 0;
    }
  }
  return is_full;
}

static int get_next_available_page_table_index(int pid) {
  page* pg;
  int i = 0;

  do {
    pg = get_page(page_tables, pid, i);
    i++;
  } while (pg->num != INIT_VAL && i < NUM_FRAMES);
  
  return --i;
}

static int find_page(int pid, int frame_number) {
  int i = 0;
  page* pg = get_page(page_tables, pid, 0);
  for (; i < NUM_FRAMES; i++) {
    if ((pg + i)->num == frame_number) {
      return i;
    }
  }
  return -1;
}

static void print_page_tables() {
  print_time();
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    print_page_table(i);
  } 
}

static void print_time() {
  fprintf(log, "Current Time: %d:%d\n\n", clock_shm->secs, clock_shm->nanosecs);
}

static void print_page_table(int pid) {
  fprintf(log, "Process %d Page Table\n", pid);

  int i = 0;
  fprintf(log, "| ");

  do {
    page* pg = get_page(page_tables, pid, i);
    if (pg->num == INIT_VAL) {
      fprintf(log, "--");
    } else {
      fprintf(log, "%02d", pg->num);
    }
    fprintf(log, " | ");
    i++;
  } while (i < NUM_FRAMES);

  fprintf(log, "\n");

  int k = 0;
  fprintf(log, "| ");

  do {
    page* pg = get_page(page_tables, pid, k);
    char* display_symbol;
    if (pg->valid && pg->dirty) {
      display_symbol = "*D";
    } else if (pg->valid && !pg->dirty) {
      display_symbol = "*-";
    } else if (!pg->valid && pg->dirty) {
      display_symbol = "-D";
    } else {
      display_symbol = "--";
    }
    fprintf(log, "%s | ", display_symbol);
    k++;
  } while (k < NUM_FRAMES);

  fprintf(log, "\n\n");
}

/**
 * Allocates semaphores for
 * making memory references with
 * an initial value of 0.
 */
static void setup_mem_sems() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    int sem_flags = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR;
    mem_sem_ids[i] = allocate_sem(IPC_PRIVATE, sem_flags);
    init_sem(mem_sem_ids[i], 0);
  }
}

/**
 * Deallocate semaphores for
 * making memory references.
 */
static void deallocate_mem_sems() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    deallocate_sem(mem_sem_ids[i]);
  }
}

/**
 * Allocates a semaphore for protecting
 * the logical clock with an initial
 * value of 1.
 */
static void setup_clock_sem() {
  int sem_flags = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR;
  clock_sem_id = allocate_sem(IPC_PRIVATE, sem_flags);
  init_sem(clock_sem_id, 1);
}

static void wait_for_all_children() {
  pid_t pid;
  while ((pid = waitpid(-1, NULL, 0))) {
    if (errno == ECHILD) {
      break;
    }
  }
}

static void setup_mem_ops(mem_op_t* mem_ops) {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    mem_ops[i].addr = INIT_VAL;
  }
}

static int should_run_page_replacement(int pid) {
  int frames_allocated = 0;
  int i = 0;
  do {
    int index = pid * NUM_FRAMES + i;
    if (unallocated_frames[index] == 1) {
      frames_allocated++;
    }
    i++;
  } while (i < NUM_FRAMES);

  frames_allocated *= 100;
  int percentage = frames_allocated / NUM_FRAMES;
  print_percentage_of_frames_allocated(percentage);

  if ((100 - percentage) <= 10) {
    print_running_page_replacement_messsage();
    return 1;
  } else {
    return 0;
  }
}

static void print_running_page_replacement_messsage() {
  if (verbose) {
    fprintf(log, "Running page replacement routine...\n");
  }
}

static void print_percentage_of_frames_allocated(int percentage) {
  if (verbose) {
    fprintf(log,
          "%%%d of total frames allocated\n\n",
          percentage);
  }
}

static void run_page_replacement(int pid) {
  int i = 0;
  do {
    page* pg = get_page(page_tables, pid, i);
    if (pg->valid == 1) {
      print_marking_frame_for_replacement(pg->num);
      pg->valid = 0;
    } else if (pg->num != INIT_VAL && pg->valid == 0) {
      print_freeing_frame(pg->num);
      int index = pid * NUM_FRAMES + i;
      unallocated_frames[index] = 0;
      reset_page(pg);
    }
    i++;
  } while (i < NUM_FRAMES);
  if (verbose) fprintf(log, "\n");
}

static void print_marking_frame_for_replacement(int frame) {
  if (verbose) {
    fprintf(log, "  Marking frame %d for replacement\n", frame);
  }
}

static void print_freeing_frame(int frame) {
  if (verbose) {
    fprintf(log, "  Freeing frame %d\n", frame);
  }
}