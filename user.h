#ifndef USER_H
#define USER_H

#include "lib/myclock.h"

static void validate_number_of_args(int argc);
static unsigned int get_creation_time();
static void update_clock(struct my_clock* myclock, unsigned int nanosecs);

#endif