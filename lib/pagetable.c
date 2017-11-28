#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "pagetable.h"

/**
 * Allocates shared memory for page tables.
 * 
 * @return The shared memory segment ID
 */
int get_page_tables() {
  size_t size = sizeof(page) * TOTAL_PAGES;
  int id = shmget(IPC_PRIVATE, size,
    IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  if (id == -1) {
    perror("Failed to get shared memory for page tables");
    exit(EXIT_FAILURE);
  }
  return id;
}

/**
 * Attaches to page tables in shared memory.
 * 
 * @return A pointer to page tables in shared memory.
 */
page* attach_to_page_tables(int id) {
  void* page_tables = shmat(id, NULL, 0);

  if (*((int*) page_tables) == -1) {
    perror("Failed to attach to page tables in shared memory");
    exit(EXIT_FAILURE);
  }

  return (page*) page_tables;
}

/**
 * Detaches from page tables in shared memory
 * 
 * @param A pointer to page tables in shared memory
 * @return On success, 0. On error -1.
 */
int detach_from_page_tables(page* page_tables) {
  int success = shmdt(page_tables);
  if (success == -1) {
    perror("FaileROCd to detach from page tables");
  }
  return success;
}

/**
 * Get the page number for a particular
 * memory address.
 * 
 * Returns -1 if the memory address
 * exceeds the process' maximum memory bound.
 * 
 * @param  mem_addr Memory address
 * @return          The page number the address belongs to
 */
int get_page_num(unsigned int mem_addr) {
  if (mem_addr > PROC_MEM) {
    return -1;  // Out of bounds
  } else {
    return mem_addr / PAGE_SIZE;
  }
}

page* get_page(page* page_tables, int pid, int page_num) {
  return page_tables + pid * NUM_FRAMES + page_num;
} 