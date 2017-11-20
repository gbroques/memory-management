#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "pagetable.h"

/**
 * Allocates shared memory for the page table.
 * 
 * @return The shared memory segment ID
 */
int get_page_table() {
  size_t size = sizeof(struct page) * PAGE_TABLE_SIZE;
  int id = shmget(IPC_PRIVATE, size,
    IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  if (id == -1) {
    perror("Failed to get shared memory for page table");
    exit(EXIT_FAILURE);
  }
  return id;
}

/**
 * Attaches to the page table in shared memory.
 * 
 * @return A pointer to the page table in shared memory.
 */
struct page* attach_to_page_table(int id) {
  void* page_table = shmat(id, NULL, 0);

  if (*((int*) page_table) == -1) {
    perror("Failed to attach to page table shared memory");
    exit(EXIT_FAILURE);
  }

  return (struct page*) page_table;
}

/**
 * Detaches from clock shared memory.
 * 
 * @param Page table shared memory
 * @return On success, 0. On error -1.
 */
int detach_from_page_table(struct page* page_table) {
  int success = shmdt(page_table);
  if (success == -1) {
    perror("Failed to detach from page table");
  }
  return success;
}