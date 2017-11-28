#include "myclock.h"

/**
 * Adds a second to the clock when nanoseconds
 * exceed the amount of nanoseconds per second.
 * @param  myclock A pointer to a clock
 * @return         1 if the clock was rounded. 0 otherwise.
 */
int round_clock(my_clock* myclock) {
  if (myclock->nanosecs < NANOSECS_PER_SEC) {
    return 0;
  }
  myclock->secs++;
  myclock->nanosecs -= NANOSECS_PER_SEC;
  return 1;
}

/**
 * Updates the clock with an amount of nanoseconds.
 * Rounds the clock if needed.
 * 
 * @param  myclock  A pointer to a clock.
 * @param  nanosecs The amount of nanoseconds to increment by.
 * @return          1 if the clock was rounded. 0 otherwise.
 */
int update_clock(my_clock* myclock, unsigned int nanosecs) {
  myclock->nanosecs += nanosecs;
  return round_clock(myclock);
}
