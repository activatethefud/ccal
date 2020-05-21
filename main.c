#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

#define DATE_FMT "%d/%m/%Y"
#define DATA_DIR "/home/nikola/.config/ccal/events"

#define Assert(cond,msg) Error(cond,msg,__FILE__,__LINE__)
#define UNUSED(x) (void)(x)
#define DEBUG(msg) fprintf(stderr,"%s\n",msg)
#define DATE_SIZE_MAX (15)

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
	short int start_time;
	short int end_time;
	small_int repeat_mode;
	small_int repeat_frequency;
	unsigned num_of_skipped_dates;
	struct tm *skipped_dates;
} event;

int max(int a, int b) { return a > b ? a : b; } 

static unsigned max_ID = 1;
static unsigned first_free_ID = -1;
static small_int *have_ID;
event* events;

status iterate_directory(const char *dirname,void* (*func)(void*));
status init(const char *data_dir);
status destructor();
status load_events(const char *data_dir);
void* filename_new_event(void *arg);
char* lineread(FILE *stream);
struct tm string_to_time(const char *string);
status iterate_events(void* (*func)(void*));
void* print_event_short(void *e);
void* print_event_long(void *e);
void* set_max_ID(void *arg);

int main()
{
	init(DATA_DIR);
	iterate_events(print_event_short);

	destructor();
	return 0;
}

status load_events(const char *data_dir)
{
	Assert(-1 != chdir(data_dir),"Error changing directory");
	events = calloc(max_ID+1,sizeof *events);
	Assert(NULL != events,"Error allocating space for all events");

	iterate_directory(data_dir,filename_new_event);
	return 0;
}

void* filename_new_event(void *arg)
{
	char *filename = arg;
	int ID = atoi(filename);

	char *tmp;

	event new_event;

	FILE *event_file = fopen(filename,"r");
	Assert(NULL != event_file,"Error opening event file.");

	// ID
	tmp = lineread(event_file);
	sscanf(tmp,"%u",&(new_event.event_id));
	free(tmp);

	// Description
	new_event.description = lineread(event_file);

	// Start date
	tmp = lineread(event_file);
	new_event.start_date = string_to_time(tmp);
	free(tmp);

	// End date

	tmp = lineread(event_file);
	new_event.end_date = string_to_time(tmp);
	free(tmp);

	// Start time
	tmp = lineread(event_file);
	sscanf(tmp,"%hd",&(new_event.start_time));
	free(tmp);

	// End time
	tmp = lineread(event_file);
	sscanf(tmp,"%hd",&(new_event.end_time));
	free(tmp);

	// Repeat mode
	tmp = lineread(event_file);
	sscanf(tmp,"%d",&(new_event.repeat_mode));
	free(tmp);

	// Repeat frequency
	tmp = lineread(event_file);
	sscanf(tmp,"%d",&(new_event.repeat_frequency));
	free(tmp);

	// Skipped dates count
	tmp = lineread(event_file);
	sscanf(tmp,"%u",&(new_event.num_of_skipped_dates));
	free(tmp);

	// Skipped dates
	new_event.skipped_dates = calloc(
		new_event.num_of_skipped_dates,
		sizeof *(new_event.skipped_dates)
	);

	for(int i=0;i<new_event.num_of_skipped_dates;++i) {
		tmp = lineread(event_file);
		new_event.skipped_dates[i] = string_to_time(tmp);
		free(tmp);
	}

	unsigned event_id = new_event.event_id;

	events[event_id] = new_event;
	have_ID[event_id] = 1;

	Assert(-1 != fclose(event_file),"Error closing file");
}

char* lineread(FILE *stream)
{
	char *line = NULL;
	size_t bytes_allocated = 0;
	ssize_t bytes_read;

	Assert(-1 != (bytes_read = getline(&line,&bytes_allocated,stream)),"Getline error");

	// Strip newline
	line[bytes_read-1] = '\0';

	return line;
}

struct tm string_to_time(const char *string)
{
	struct tm time;
	strptime(string,DATE_FMT,&time);
	return time;
}

status destructor()
{
	free(events);
}

status init(const char *data_dir)
{
	iterate_directory(data_dir,set_max_ID);

	have_ID = calloc(max_ID+1,sizeof *have_ID);
	memset(have_ID,0,max_ID+1);

	load_events(data_dir);

	for(int i=1;i<=max_ID;++i) {
		if(have_ID == 0) {
			first_free_ID = i;
			break;
		}
	}

	if(first_free_ID == -1) {
		first_free_ID = max_ID+1;
	}

	return 0;
}

status iterate_directory(const char *dirname,void* (*func)(void*))
{
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
	Assert(-1 != closedir(data),"Error closing directory");
	return 0;
}

status iterate_events(void* (*func)(void*))
{
	for(int i=0;i<max_ID;++i) {
		if(have_ID[i]) {
			(func)(events+i);
		}
	}
}

void* print_event_short(void *e)
{
	event ev = *(event*)e;

	printf("(%u) [%04hd] -> [%04hd] %s\n",
		ev.event_id,
		ev.start_time,
		ev.end_time,
		ev.description);
}

void* print_event_long(void *e)
{
}

void* set_max_ID(void *arg)
{
	unsigned id = atoi((char*)arg);
	max_ID = max(max_ID,id);
}
