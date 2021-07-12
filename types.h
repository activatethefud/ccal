#include <time.h>

#ifndef TYPES_H
#define TYPES_H

typedef char small_int;
typedef char status;

typedef enum {
	none,
	daily,
	weekly,
	monthly,
	yearly
} repeat_t, shift_t;

typedef struct {
	struct tm time;
	small_int opening_closing;
} interval_t;


typedef struct {
	unsigned event_id;
	char *description;
	struct tm start_time;
	struct tm end_time;
	repeat_t repeat_mode;
	small_int repeat_frequency;
	unsigned num_of_skipped_dates;
	struct tm *skipped_dates;
} event;

typedef event event_t;

typedef struct {
        char *name;
        float duration;
        double e_val;
	int repeating;
        struct tm lower_bound;
        struct tm upper_bound;
} goal_t;

#endif
