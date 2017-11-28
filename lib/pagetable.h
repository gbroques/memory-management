#ifndef PAGETABLE_H_
#define PAGETABLE_H_

#define MAX_PROCS 12

// Total System Memory (in bytes)
#define TOTAL_MEM 256000

#define PAGE_SIZE 1000  // (in bytes)

#define TOTAL_PAGES 256

// Total pages / Maximum processes
#define NUM_FRAMES 21  // frames per process

// Amount of Memory per Process (in bytes)
#define PROC_MEM 32000

typedef struct page {
  unsigned int num;  // frame number
  unsigned char valid;
  unsigned char dirty;
} page;

// I/O Operation
typedef enum { READ, WRITE } io_op;

/**
 * Memory Operation
 */
typedef struct mem_op_t {
  int addr;  // Address of the operation
  io_op op;  // Read or write
} mem_op_t;

int get_page_tables();
page* attach_to_page_tables(int id);
int detach_from_page_tables(page* page_tables);
int get_page_num(unsigned int mem_addr);
page* get_page(page* page_tables, int pid, int page_num);

#endif