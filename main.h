#define DATE_FMT "%d/%m/%Y"

#define Assert(cond,msg) Error(cond,msg,__FILE__,__LINE__)
#define UNUSED(x) (void)(x)
#define DEBUG(msg) fprintf(stderr,"%s\n",msg)
#define DATE_SIZE_MAX (15)
#define NEW_EVENT (1)
#define HOURS(x) (x)/100
#define MINUTES(x) (x)%100
#define LOG_SAFETY (2)
#define DAY_SECS (3600*24)
#define NAME_FIELD (0)
#define DURATION_FIELD (1)
#define EVAL_FIELD (2)
#define REPEAT_FIELD (3)
#define LOWERBOUND_FIELD (4)
#define UPPERBOUND_FIELD (5)

#define GETOPT_FMT "nd:ptq:w:s:f:g:"

#ifndef MAIN_H
#define MAIN_H

void Error(const bool cond, const char *msg, const char *file, const int line)
{
	if(!cond) {
		perror(msg);
		fprintf(stderr,"%s: %d\n",file,line);
		exit(EXIT_FAILURE);
	}
}

int max(int a, int b) { return a > b ? a : b; } 

unsigned max_ID = 0;
unsigned first_free_ID = 1;
small_int *have_ID;
event* events;

const struct tm eternity = {
        0, 0, 0, 1, 0, 3100, 4, 0, -1
};

struct tm today;

char *concat(const char *str1,const char *str2);
status iterate_directory(const char *dirname,void* (*func)(void*));
status init();
status destructor();
void event_destructor(void *e);
status load_events(const char *data_dir);
void* filename_new_event(void *arg);
char* lineread(FILE *stream,const char *prompt);
struct tm string_to_time(char *string);
status iterate_events(void (*func)(void*));
void print_event_short(void *e);
void print_event_long(void *e);
void* set_max_ID(void *arg);
event new_event();
status delete_event(unsigned id_to_delete);
status validate_time(int time);
status validate_chrono_order(struct tm *time1, struct tm *time2);
status validate_date_string(const char *string);
double tm_difftime(struct tm *time1, struct tm *time2);
status save_event(event *e);
void print_usage();
time_t shift_time(struct tm *time,shift_t shift,int by_amount);
node_t* event_on_date(const event *e,struct tm *lower_bound, struct tm *upper_bound);
event* get_events_on_date(const struct tm time,event** arr,int *arr_size);
bool event_is_skiped(const event *e,struct tm *date);
struct tm* tm_max(struct tm *time1, struct tm *time2);
struct tm* tm_min(struct tm *time1, struct tm *time2);
void answer_query(const struct tm time);
const char *weekday(int wday);
float free_time(event *arr,const int arr_size,bool *overlap);
status skip_date(struct tm date_to_skip,event *e);
status skip_date_prompt(unsigned event_id,const char *date);
status delete_event_prompt(unsigned event_id);
void clear_date(char *date);
int compare_events_by_id(void *a,void *b);
int compare_goals_by_name(void *a,void *b);
unsigned int get_event_id(node_t *ptr);
char *input(const char *prompt);
node_t *read_goals(const char *goal_file);
void print_goal(void *g);
void print_tm(struct tm *time);
node_t *generate_schedule(char *date_string);

#endif
