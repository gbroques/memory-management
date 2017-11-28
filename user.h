#ifndef USER_H_
#define USER_H_

#include "lib/myclock.h"
#include "lib/pagetable.h"

#define ARGC 6

static void validate_number_of_args(int argc);
static void update_clock_with_creation_time(my_clock* clock_shm,
                                            const int clock_sem_id);
static unsigned int get_creation_time();
static unsigned int get_mem_addr();
static io_op get_read_or_write();
static void check_should_terminate(int* terminate_flag);
static void make_mem_request(mem_op_t* mem_ops, const int pid);
static int should_check_whether_to_terminate(int num_requests);

#endif