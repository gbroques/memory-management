#ifndef STATS_H_
#define STATS_H_

#include "lib/myclock.h"

typedef struct stats_t {
  unsigned int num_mem_accesses;
  unsigned int num_page_faults;
  my_clock start_time;
  my_clock end_time;
} stats_t;

#endif