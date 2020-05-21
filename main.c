#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>

#define DATE_FMT "%d/%m/%Y"
#define DATA_DIR "/home/nikola/.config/ccal/events"

#define Assert(cond,msg) Error(cond,msg,__FILE__,__LINE__)
#define UNUSED(x) (void)(x)
#define DEBUG(msg) fprintf(stderr,"%s\n",msg)

void Error(const bool cond, const char *msg, const char *file, const int line)
{
	if(!cond) {
		perror(msg);
		fprintf(stderr,"%s: %d\n",file,line);
		exit(EXIT_FAILURE);
	}
}

typedef char small_int;
typedef char status;

typedef struct Event {
	unsigned event_id;
	char *description;
	struct tm start_date;
	struct tm end_date;
	small_int start_time;
	small_int end_time;
	small_int repeat_mode;
	small_int repeat_frequency;
	unsigned num_of_skipped_dates;
	struct tm *skipped_dates;
} event;

int max(int a, int b) { return a > b ? a : b; } 

static unsigned max_ID = 1;
static unsigned first_free_ID = -1;
static small_int *have_ID;

status iterate_directory(const char *dirname,void* (*func)(void*)) {
	DIR *data = opendir(dirname);
	Assert(NULL != data,"Error opening directory.");

	errno = 0;
	struct dirent *iterator;

	while(NULL != (iterator = readdir(data))) {

		char *filename = iterator->d_name;

		if(filename[0] != '.') {
			(func)(filename);
		}

	}

	Assert(0 == errno,"Error reading directory");
	return 0;
}

void* set_max_ID(void* arg)
{
	int ID = atoi((char*)arg);
	max_ID = max(max_ID,ID);
}

void* fill_have_ID(void *arg)
{
	int ID = atoi((char*)arg);
	have_ID[ID] = true;
}

status init(const char *data_dir)
{
	iterate_directory(data_dir,set_max_ID);

	have_ID = calloc(max_ID+1,sizeof *have_ID);
	memset(have_ID,0,max_ID+1);

	iterate_directory(data_dir,fill_have_ID);

	for(int i=1;i<=max_ID;++i) {
		if(have_ID == 0) {
			first_free_ID = i;
			break;
		}
	}

	if(first_free_ID == -1) {
		first_free_ID = max_ID+1;
	}

	free(have_ID);
}


event *load_events(const char *data_dir);
void* filename_new_event(void *arg);
char* lineread(FILE *stream);
struct tm *string_to_time(const char *string);

int main()
{
	init(DATA_DIR);
	return 0;
}

event *load_events(const char *data_dir)
{
	event *events = calloc(max_ID,sizeof *events);
}

//typedef struct Event {
//	unsigned event_id;
//	struct tm start_date;
//	struct tm end_date;
//	small_int start_time;
//	small_int end_time;
//	small_int repeat_mode;
//	small_int repeat_frequency;
//	int num_of_skipped_dates;
//	struct tm *skipped_dates;
//} event;


void* filename_new_event(void *arg)
{
	char *filename = arg;
	int ID = atoi(filename);

	event *new_event = malloc(sizeof *new_event);

	FILE *event_file = fopen(filename,"r");

	fscanf(event_file,"%u",&(new_event->event_id));
	new_event->description = lineread(event_file);

	fclose(event_file);
}

char* lineread(FILE *stream)
{
	char *line = NULL;
	size_t bytes_allocated = 0;
	ssize_t bytes_read;

	Assert(-1 != (bytes_read = getline(&line,&bytes_allocated,stream)),"Getline error");

	// Strip newline
	line[bytes_read] = '\0';

	return line;
}

struct tm *string_to_time(const char *string)
{
}
