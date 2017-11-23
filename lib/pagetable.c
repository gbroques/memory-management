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
int get_page_tables(unsigned int num_page_tables) {
  size_t size = sizeof(struct page) * PAGE_TABLE_SIZE * num_page_tables;
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
struct page** attach_to_page_tables(int id) {
  void* page_tables = shmat(id, NULL, 0);

  if (*((int*) page_tables) == -1) {
    perror("Failed to attach to page tables in shared memory");
    exit(EXIT_FAILURE);
  }

  return (struct page**) page_tables;
}

/**
 * Detaches from page tables in shared memory
 * 
 * @param A pointer to page tables in shared memory
 * @return On success, 0. On error -1.
 */
int detach_from_page_tables(struct page** page_tables) {
  int success = shmdt(page_tables);
  if (success == -1) {
    perror("Failed to detach from page tables");
  }
  return success;
}