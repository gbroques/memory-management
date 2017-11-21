#include "myclock.h"

void round_clock(struct my_clock* myclock) {
  if (myclock->nanosecs >= NANOSECS_PER_SEC) {
    myclock->secs++;
    myclock->nanosecs -= NANOSECS_PER_SEC;
  }
}