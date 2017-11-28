#ifndef MYCLOCK_H_
#define MYCLOCK_H_

// 1 * 10^9 nanoseconds
#define NANOSECS_PER_SEC 1000000000

// 1 * 10^6 nanoseconds
#define NANOSECS_PER_MILLISEC 1000000  

/*--------------------------*
 | A simple simulated clock |
 *--------------------------*/
typedef struct my_clock {
  unsigned int secs;      // Amount of time in seconds
  unsigned int nanosecs;  // Amount of time in nanoseconds
} my_clock;

int round_clock(my_clock* myclock);
int update_clock(my_clock* myclock, unsigned int nanosecs);

#endif
