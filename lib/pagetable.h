#ifndef PAGETABLE_H
#define PAGETABLE_H

#define PAGE_TABLE_SIZE 32  // frames

// Total System Memory (in bytes)
#define TOTAL_MEM 256000

// Amount of Memory per Process (in bytes)
#define PROC_MEM  32000

#define PAGE_SIZE 1000

struct page {
  unsigned char valid;
  unsigned char dirty;
};

int get_page_tables();
struct page** attach_to_page_tables(int id);
int detach_from_page_tables(struct page** page_tables);


#endif