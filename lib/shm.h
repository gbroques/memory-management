#ifndef SHM_H_
#define SHM_H_

#include "myclock.h"
#include "pagetable.h"

/*------------------------------------------*
 | Operating System Simulator Shared Memory |
 *------------------------------------------*/

int get_clock_shm();
my_clock* attach_to_clock_shm(int id);
int detach_from_clock_shm(my_clock* shm);

int get_mem_ops(int num_procs);
mem_op_t* attach_to_mem_ops(int id);
int detach_from_mem_ops(mem_op_t* shm);

#endif
