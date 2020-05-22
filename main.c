#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>

#define DATE_FMT "%d/%m/%Y"
#define DATA_DIR "/home/nikola/.config/ccal/events"

#define Assert(cond,msg) Error(cond,msg,__FILE__,__LINE__)
#define UNUSED(x) (void)(x)
#define DEBUG(msg) fprintf(stderr,"%s\n",msg)
#define DATE_SIZE_MAX (15)
#define NEW_EVENT (1)
#define HOURS(x) (x)/100
#define MINUTES(x) (x)%100
#define LOG_SAFETY (2)

#define GETOPT_FMT "nd:pt"

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

typedef struct {
	unsigned event_id;
	char *description;
	struct tm start_time;
	struct tm end_time;
	small_int repeat_mode;
	small_int repeat_frequency;
	unsigned num_of_skipped_dates;
	struct tm *skipped_dates;
} event;

typedef enum {
	none,
	daily,
	weekly,
	monthly,
	yearly
} repeat_t, shift_t;

int max(int a, int b) { return a > b ? a : b; } 

static unsigned max_ID = 1;
static unsigned first_free_ID = -1;
static small_int *have_ID;
event* events;

status iterate_directory(const char *dirname,void* (*func)(void*));
status init(const char *data_dir);
status destructor();
void* event_destructor(void *e);
status load_events(const char *data_dir);
void* filename_new_event(void *arg);
char* lineread(FILE *stream,const char *prompt);
struct tm string_to_time(const char *string);
status iterate_events(void* (*func)(void*));
void* print_event_short(void *e);
void* print_event_long(void *e);
void* set_max_ID(void *arg);
event new_event();
status delete_event(unsigned id_to_delete);
status validate_time(int time);
status validate_chrono_order(struct tm *time1, struct tm *time2);
double tm_difftime(struct tm *time1, struct tm *time2);
status save_event(event *e);
void print_usage();
time_t shift_time(struct tm *time,shift_t shift,int by_amount);
bool event_on_date(const event *e,struct tm date);
bool event_is_skiped(const event *e,struct tm date);

void print_tm(struct tm *time)
{
	printf("SEC: %d\nMIN: %d\nHOUR: %d\nDAY: %d\nMON: %d\nYEAR: %d\n\n",
	time->tm_sec,
	time->tm_min,
	time->tm_hour,
	time->tm_mday,
	time->tm_mon,
	time->tm_year);
}

int main(int argc, char **argv)
{
	init(DATA_DIR);

	int new_event_flag = 0;
	int print_flag = 0;
	int delete_flag = 0;
	int delete_arg = 0;
	int test_flag = 0;

	int long_opt_index;
	int option;

	struct option long_options[] = {

		{ "new", no_argument, &new_event_flag, 1 },
		{ "print", no_argument, &print_flag, 1 },
		{ "delete", required_argument, &delete_flag, 1 },
		{ 0, 0, 0, 0 }
	};

	while(-1 != (option = getopt_long(argc,argv,GETOPT_FMT,long_options,&long_opt_index))) {

		// Long option found
		if(option == 0) {
			switch(long_opt_index) {
				case 2:
					delete_arg = atoi(optarg);
					break;
			}
		}

		else {
			switch(option) {

				case 'n':
					new_event_flag = 1;
					break;
				case 'p':
					print_flag =1;
					break;
				case 'd':
					delete_arg = atoi(optarg);
					delete_flag = 1;
					break;
				case 't':
					test_flag = 1;
					break;
			}
		}
	}

	if(test_flag) {
		event new = new_event();

		printf("%d\n",event_on_date(&new,string_to_time(lineread(stdin,"Enter query date: "))));
	}
	else if(delete_flag) {
		delete_event(delete_arg);
	}
	else if(new_event_flag) {
		event new = new_event();
		save_event(&new);
		event_destructor(&new);
	}
	else if(print_flag) {
		iterate_events(print_event_long);
	}
	else {
		print_usage();
	}



	destructor();
	return 0;
}

#define DAY_SECS (3600*24)

bool event_on_date(const event *e,struct tm date)
{
	struct tm date_day_after = date;
	shift_time(&date_day_after,1,1);

	return false;
}

time_t shift_time(struct tm *time,shift_t shift,int by_amount)
{
	switch(shift) {
		case daily:
			time->tm_mday += by_amount;
			break;
		case weekly:
			time->tm_mday += 7 * by_amount;
			break;
		case monthly:
			time->tm_mon += by_amount;
			break;
		case yearly:
			time->tm_year += by_amount;
			break;
		default:
			Assert(false,"Bad shift option");
	}

	time_t shifted_Epoch;
	Assert(-1 != (shifted_Epoch = mktime(time)),"Error shifting time");

	return shifted_Epoch;
}

void print_usage()
{
	printf("Usage:\n"
		"    -n --new - Add new event\n"
		"    -p - Print all events\n"
		"    -d <id> - Delete event with ID <id>\n"
		);
}

void* event_destructor(void *e)
{
	event* ev = (event*)e;
	free(ev->description);

	if(ev->num_of_skipped_dates > 0) {
		free(ev->skipped_dates);
	}
}

status save_event(event *e)
{
	int txt_len = 5;

	char *filename = calloc(
		ilogb(log10(e->event_id + LOG_SAFETY))+1 + txt_len,
		sizeof *filename
		);

	Assert(NULL != filename,"Error allocating space for file name");

	sprintf(filename,"%u.txt",e->event_id);

	char start_date[DATE_SIZE_MAX];
	char end_date[DATE_SIZE_MAX];
	char tmp[DATE_SIZE_MAX];

	strftime(start_date,DATE_SIZE_MAX,DATE_FMT,&(e->start_time));
	strftime(end_date,DATE_SIZE_MAX,DATE_FMT,&(e->end_time));

	FILE *output_file = fopen(filename,"w");
	Assert(NULL != output_file,"Error opening file for writing");

	fprintf(output_file,"%u\n%s\n%s\n%s\n%d%02d\n%d%02d\n%hd\n%hd\n%u\n",
		e->event_id,
		e->description,
		start_date,
		end_date,
		e->start_time.tm_hour,
		e->start_time.tm_min,
		e->end_time.tm_hour,
		e->end_time.tm_min,
		e->repeat_mode,
		e->repeat_frequency,
		e->num_of_skipped_dates);
	
	for(int i=0;i<e->num_of_skipped_dates;++i) {
		strftime(tmp,DATE_SIZE_MAX,DATE_FMT,&(e->skipped_dates[i]));
		fprintf(output_file,"%s\n",tmp);
	}

	fclose(output_file);
	free(filename);
	return 0;
}

status validate_chrono_order(struct tm *time1, struct tm *time2)
{
	time1->tm_sec = 0;
	time1->tm_isdst = 1;

	time2->tm_sec = 0;
	time2->tm_isdst = 1;

	if(tm_difftime(time1,time2) <= 0) {
		return 0;
	}
	return -1;
}

double tm_difftime(struct tm *time1, struct tm *time2)
{
	return difftime(
		mktime(time1),
		mktime(time2)
	);
}

status delete_event(unsigned id_to_delete)
{
	int txt_len = 5;
	
	char filename[ilogb(log10(id_to_delete + LOG_SAFETY))+1 + txt_len];
	sprintf(filename,"%u.txt",id_to_delete);

	Assert(-1 != unlink(filename),"Error while removing event file");
	return 0;
}

status validate_time(int time)
{
	if(HOURS(time) <= 23 && HOURS(time) >= 0 && MINUTES(time) <= 59 && MINUTES(time) >= 0) {
		return 0;
	}

	return -1;
}

event new_event()
{	
	event new;
	char *tmp;

	// ID
	new.event_id = first_free_ID;

	// Description
	new.description = lineread(stdin,"Event description: ");

	Assert('\0' != new.description[0],"Description must not be empty");

	// Start date
	tmp = lineread(stdin,"Start date: ");
	Assert(NULL != strptime(tmp,DATE_FMT,&(new.start_time)),"Error parsing start date");
	free(tmp);

	// End date
	tmp = lineread(stdin,"End date: ");

	if(tmp[0] == '\0') {
		new.end_time = new.start_time;
	}
	else {
		Assert(NULL != strptime(tmp,DATE_FMT,&(new.end_time)),"Error parsing end date");
	}
	free(tmp);

	// Start time
	tmp = lineread(stdin,"Start time: ");

	if(tmp[0] == '\0') {
		new.start_time.tm_hour = 0;
		new.start_time.tm_min = 0;
	}
	else {
		int time = atoi(tmp);
		Assert(-1 != validate_time(time),
			"Error validating start time");
		new.start_time.tm_hour = HOURS(time);
		new.start_time.tm_min = MINUTES(time);
	}
	free(tmp);

	// End time
	tmp = lineread(stdin,"End time: ");

	if(tmp[0] == '\0') {
		new.end_time.tm_hour = 23;
		new.end_time.tm_min = 59;
	}
	else {
		int time = atoi(tmp);
		Assert(-1 != validate_time(time),
			"Error validating end time");
		new.end_time.tm_hour = HOURS(time);
		new.end_time.tm_min = MINUTES(time);
	}
	free(tmp);

	Assert(-1 != validate_chrono_order(&new.start_time,&new.end_time),
		"End time before start time error");

	// Repeat mode
	tmp = lineread(stdin,"Repeat [n/0|d/1|w/2|m/3|y/4]: ");

	if(tmp[0] == '\0') {
		new.repeat_mode = 0;
	}
	else {
		new.repeat_mode = abs(atoi(tmp))%5;
	}

	free(tmp);

	// Repeat frequency
	tmp = lineread(stdin,"Repeat frequency: ");

	if(tmp[0] == '\0') {
		new.repeat_frequency = 0;
	}
	else {
		new.repeat_frequency = abs(atoi(tmp));
	}
	free(tmp);

	new.num_of_skipped_dates = 0;

	return new;
}

status load_events(const char *data_dir)
{
	events = calloc(max_ID+1+NEW_EVENT,sizeof *events);
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
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%u",&(new_event.event_id));
	free(tmp);

	// Description
	new_event.description = lineread(event_file,NULL);

	// Start date
	tmp = lineread(event_file,NULL);
	new_event.start_time = string_to_time(tmp);
	free(tmp);

	// End date

	tmp = lineread(event_file,NULL);
	new_event.end_time = string_to_time(tmp);
	free(tmp);

	// Start time
	tmp = lineread(event_file,NULL);
	short int start_time;
	sscanf(tmp,"%hd",&start_time);
	new_event.start_time.tm_hour = HOURS(start_time);
	new_event.start_time.tm_min = MINUTES(start_time);
	free(tmp);

	// End time
	tmp = lineread(event_file,NULL);
	short int end_time;
	sscanf(tmp,"%hd",&end_time);
	new_event.end_time.tm_hour = HOURS(end_time);
	new_event.end_time.tm_min = MINUTES(end_time);
	free(tmp);

	Assert(-1 != validate_chrono_order(&new_event.start_time,&new_event.end_time),
		"End time before start time");

	// Repeat mode
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%d",&(new_event.repeat_mode));
	free(tmp);

	// Repeat frequency
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%d",&(new_event.repeat_frequency));
	free(tmp);

	// Skipped dates count
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%u",&(new_event.num_of_skipped_dates));
	free(tmp);

	// Skipped dates
	new_event.skipped_dates = calloc(
		new_event.num_of_skipped_dates,
		sizeof *(new_event.skipped_dates)
	);

	for(int i=0;i<new_event.num_of_skipped_dates;++i) {
		tmp = lineread(event_file,NULL);
		new_event.skipped_dates[i] = string_to_time(tmp);
		new_event.skipped_dates[i].tm_sec = 0;
		new_event.skipped_dates[i].tm_isdst = 1;
		free(tmp);
	}

	unsigned event_id = new_event.event_id;

	events[event_id] = new_event;
	have_ID[event_id] = 1;

	Assert(-1 != fclose(event_file),"Error closing file");
}

char* lineread(FILE *stream,const char *prompt)
{
	if(NULL != prompt) {
		printf(prompt);
	}

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
	time.tm_sec = 0;
	time.tm_isdst = 1;
	return time;
}

status destructor()
{
	iterate_events(event_destructor);
	free(events);
}

status init(const char *data_dir)
{
	// Set timezone from system timezone
	tzset();
	
	// Change working directory to data_dir
	Assert(-1 != chdir(data_dir),"Error changing directory");

	iterate_directory(data_dir,set_max_ID);

	have_ID = calloc(max_ID+1,sizeof *have_ID);
	memset(have_ID,0,max_ID+1);

	load_events(data_dir);

	for(int i=1;i<=max_ID;++i) {
		if(have_ID[i] == 0) {
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

	printf("(%u) [%02d:%02d] -> [%02d:%02d] %s\n",
		ev.event_id,
		ev.start_time.tm_hour,
		ev.start_time.tm_min,
		ev.end_time.tm_hour,
		ev.end_time.tm_min,
		ev.description);
}

void* print_event_long(void *e)
{
	event ev = *(event*)e;
	char start_date_str[DATE_SIZE_MAX];
	char end_date_str[DATE_SIZE_MAX];

	char repeat_mode = '0';

	strftime(start_date_str,DATE_SIZE_MAX,DATE_FMT,&ev.start_time);
	strftime(end_date_str,DATE_SIZE_MAX,DATE_FMT,&ev.end_time);

	switch(ev.repeat_mode) {
		case 1: repeat_mode = 'd'; break;
		case 2: repeat_mode = 'w'; break;
		case 3: repeat_mode = 'm'; break;
		case 4: repeat_mode = 'y'; break;
	}

	printf("(%u) [%s|%s] [%02d:%02d -> %02d:%02d] [%c|%d] %s\n",
		ev.event_id,
		start_date_str,
		end_date_str,
		ev.start_time.tm_hour,
		ev.start_time.tm_min,
		ev.end_time.tm_hour,
		ev.end_time.tm_min,
		repeat_mode,
		ev.repeat_frequency,
		ev.description);

}

void* set_max_ID(void *arg)
{
	unsigned id = atoi((char*)arg);
	max_ID = max(max_ID,id);
}
