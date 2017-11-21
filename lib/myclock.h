#ifndef MYCLOCK_H
#define MYCLOCK_H

// 1 * 10^9 nanoseconds
#define NANOSECS_PER_SEC 1000000000

// 1 * 10^6 nanoseconds
#define NANOSECS_PER_MILLISEC 1000000  

/*--------------------------*
 | A simple simulated clock |
 *--------------------------*/
struct my_clock {
  unsigned int secs;      // Amount of time in seconds
  unsigned int nanosecs;  // Amount of time in nanoseconds
};

void round_clock(struct my_clock* myclock);

#endif
