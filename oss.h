#ifndef OSS_H_
#define OSS_H_

#include "lib/pagetable.h"

static void parse_command_options(int argc, char* argv[]);
static void print_help_message(char* executable_name);
static void setup_data_structures();
static void setup_unallocated_frames();
static void open_log_file();
static void free_shm_and_abort(int signum);
static void free_shm();
static void setup_interrupt_handler();
static void setup_interval_timer(int time);
static void handle_timer_interrupt();
static void handle_child_termination(int signum);
static void fork_and_exec_children();
static void fork_and_exec_child(int pid);
static void check_for_mem_requests();
static int has_mem_request(mem_op_t mem_op);
static void handle_mem_request(int pid, mem_op_t* mem_op);
static void print_received_memory_request(io_op op, int pid, int page_num);
static void setup_clock_sem();
static void setup_mem_sems();
static void setup_page_tables();
static void deallocate_mem_sems();
static void wait_for_all_children();
static void setup_mem_ops(mem_op_t* mem_ops);
static int is_page_in_memory(int pid, int page_num);
static int is_page_table_full(int pid);
static int get_next_available_page_table_index(int pid);
static int find_page(int pid, int frame_number);
static void print_page_table(int pid);
static int should_run_page_replacement(int pid);
static void run_page_replacement(int pid);
static void print_running_page_replacement_messsage();
static void print_percentage_of_frames_allocated(int percentage);
static void print_marking_frame_for_replacement(int frame);
static void print_freeing_frame(int frame);
static void print_time();
static void print_page_tables();
static void reset_page(page* pg);
static void free_memory(int pid);
static void print_stats_report(int pid);
static unsigned int get_avg_mem_access_speed(int mem_accesses, int page_faults);
static int get_num_procs_completed();
static void print_stats_report_separator(int length);

#endif