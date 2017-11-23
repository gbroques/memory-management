#ifndef PAGETABLE_H
#define PAGETABLE_H

#define PAGE_TABLE_SIZE 256

// Total System Memory (in bytes)
#define TOTAL_MEM 256000

// Amount of Memory per Process (in bytes)
#define PROC_MEM  32000

#define PAGE_SIZE 1000

struct page {
  unsigned int frame;
  unsigned char valid;
  unsigned char dirty;
  unsigned int pid;
};

int get_page_table();
struct page* attach_to_page_table(int id);
int detach_from_page_table(struct page* page_table);


#endif