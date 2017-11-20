#ifndef PAGETABLE_H
#define PAGETABLE_H

#define PAGE_TABLE_SIZE 256

struct page {
  unsigned int frame;
  unsigned char valid;
  unsigned char dirty;
};

int get_page_table();
struct page* attach_to_page_table(int id);
int detach_from_page_table(struct page* page_table);


#endif