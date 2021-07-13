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
#include <stdbool.h>
#include "llist.h"
#include "tokenizer.h"
#include "random.h"

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

void Error(const bool cond, const char *msg, const char *file, const int line)
{
	if(!cond) {
		perror(msg);
		fprintf(stderr,"%s: %d\n",file,line);
		exit(EXIT_FAILURE);
	}
}

int max(int a, int b) { return a > b ? a : b; } 

static unsigned max_ID = 0;
static unsigned first_free_ID = 1;
static small_int *have_ID;
event* events;

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

int contains(event *e1, event *e2)
{
        int result = tm_difftime(&e1->start_time,&e2->start_time) < 0 &&
               tm_difftime(&e1->end_time,&e2->end_time) > 0;
        return result;
}

int compare_chrono_order_endtime(void *e1,void *e2)
{
        event *ev1 = (event*)e1;
        event *ev2 = (event*)e2;

        return tm_difftime(&ev1->end_time,&ev2->end_time);
}

void float_to_tm(float time,struct tm *t)
{
        t->tm_hour += (int)floorf(time);
        t->tm_min += (int)floorf(60*(time-floor(time)));
        mktime(t);
}

int compare_24h(struct tm *t1,struct tm *t2)
{
        int time1 = 100 * t1->tm_hour + t1->tm_min;
        int time2 = 100 * t2->tm_hour + t2->tm_min;

        if(time1 < time2) return -1;
        if(time1 > time2) return 1;
        return 0;
}

#define between(x,a,b) ((a) <= (x) && (x) <= (b))

status validate_date_string(const char *string)
{
        unsigned int d,m,y;
        char c1,c2;
        sscanf(string,"%u%c%u%c%u",&d,&c1,&m,&c2,&y);

        if(between(d,1,31) && between(m,1,12) && c1 == '/' && c2 == '/') {
                return 0;
        }
        return -1;
}

int main(int argc, char **argv)
{
	init();

	int new_event_flag = 0;
	int print_flag = 0;
	int delete_flag = 0;
	int delete_arg = 0;
	int test_flag = 0;
	int query_flag = 0;
	int week_flag = 0;
	int skip_flag = 0;
	int clear_date_flag = 0;
	int forward_flag = 0;
	int forward_arg;
	int generate_flag = 0;

	char* query_arg = NULL;
        char* generate_arg = NULL;

	int long_opt_index;
	int option;

	struct option long_options[] = {

		{ "new", no_argument, &new_event_flag, 1 },
		{ "print", no_argument, &print_flag, 1 },
		{ "delete", required_argument, &delete_flag, 1 },
		{ "query", required_argument, &query_flag, 1},
		{ "clear-date", no_argument, &clear_date_flag, 1},
		{ "generate", required_argument, &generate_flag, 1 },
		{ 0, 0, 0, 0 }
	};

	while(-1 != (option = getopt_long(argc,argv,GETOPT_FMT,long_options,&long_opt_index))) {

		// Long option found
		if(option == 0) {
			switch(long_opt_index) {
				case 2:
					delete_arg = atoi(optarg);
					break;
				case 3:
					query_arg = optarg;
					break;
                                case 5:
                                        generate_arg = strdup(optarg);
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
				case 'q':
					query_flag = 1;
					query_arg = optarg;
					break;
				case 'w':
					query_arg = optarg;
					week_flag = 1;
					break;
				case 's':
					skip_flag = atoi(optarg);
					break;
				case 'f':
					forward_flag = 1;
					forward_arg = atoi(optarg);
					break;
				case 'g':
					generate_flag = 1;
                                        generate_arg = strdup(optarg);
					break;
			}
		}
	}

	if(skip_flag) {


		if(skip_date_prompt(skip_flag,query_arg)) {
			skip_date(string_to_time(query_arg),&events[skip_flag]);
			printf("Date skipped.\n");
		}
		else {
			printf("Date not skipped.\n");
		}
	}
	else if(generate_flag) {
		//generate_schedule(input("Date: "));
                Assert(-1 != validate_date_string(generate_arg),"Invalid date string");
		generate_schedule(generate_arg);
                free(generate_arg);
	}
	else if(query_flag && forward_flag) {

		struct tm start_time = string_to_time(query_arg);
		shift_t shift = daily;

		for(int i=0;i<forward_arg;++i) {
			answer_query(start_time);
			shift_time(&start_time,shift,1);
		}
	}
	else if(query_flag && clear_date_flag) {
		clear_date(query_arg);
	}
	else if(query_flag && !skip_flag && !test_flag) {
		answer_query(string_to_time(query_arg));
	}
	else if(week_flag) {

		struct tm start_time = string_to_time(query_arg);
		shift_t shift = daily;

		for(int i=0;i<7;++i) {
			answer_query(start_time);
			shift_time(&start_time,shift,1);
		}

	}
        // TEST
	else if(test_flag) {
                puts(concat("Ayy","Lmao"));
	}
	else if(delete_flag) {

		if(delete_event_prompt(delete_arg)) {
			delete_event(delete_arg);
			printf("Event deleted.\n");
		}
		else {
			printf("Event not deleted.\n");
		}
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

// END

node_t *generate_schedule(char *date_string)
{
                //char *date_string = strdup(date_string_);
                // Set up day start and end events, and add them to the event list
                struct tm time = string_to_time(date_string);

                struct tm day_start = string_to_time(date_string);
                struct tm day_end = string_to_time(date_string);

		// No need to free date_string because it's optarg
		//free(date_string);

                //day_end.tm_mday++;
                day_end.tm_hour += 23;
                day_end.tm_min += 59;
                mktime(&day_end);

                event day_start_event;
                day_start_event.event_id = 99998;
                day_start_event.description = "Day start";
                day_start_event.start_time = day_start;
                day_start_event.end_time = day_start;
                
                event day_end_event;
                day_end_event.event_id = 99999;
                day_end_event.description = "Day end";
                day_end_event.start_time = day_end;
                day_end_event.end_time = day_end;
                //

                // Get events on date and convert to linked list
                event *e_arr;
                int e_arr_size;
                get_events_on_date(time,&e_arr,&e_arr_size);

                node_t *events = NULL;

                // Array to linked list, skip overlapped events
                int i=0;
                while(i<e_arr_size) {
                        
                        add_right(&events,e_arr+i,sizeof *(e_arr+i));
                        
                        int j=i+1;
                        // Check for overlap to skip adding event to linked list
                        while(j<e_arr_size && contains(e_arr+i,e_arr+j)) {
                                ++j;
                        }
                        //
                        i=j;
                }

                // Sort by endtime for easier event fitting
                comparison_t *c_endtime = malloc(sizeof *c_endtime);
                c_endtime->fptr = compare_chrono_order_endtime;
                sort_list(events,c_endtime);
                free(c_endtime);
                //

                add_left(&events,&day_start_event,sizeof day_start_event);
                add_right(&events,&day_end_event,sizeof day_end_event);
                //

                node_t *ptr1 = events;
                node_t *ptr2 = events;

                // Relative to data directory
                node_t *goals_base = read_goals("goals.txt");
                //

                // Set up comparisons
                comparison_t *c = malloc(sizeof *c);
                c->fptr = compare_goals_by_name;
                comparison_t *c_e = malloc(sizeof *c_e);
                c_e->fptr = compare_events_by_id;
                //

                // FEAT
                // From goals delete those already scheduled on working date, provided the goal does not repeat
                // Also, those repeating will have their e_values increased
                {
                        node_t *_iter = events;
                        while(_iter != NULL) {
                                goal_t *g = malloc(sizeof *g);
                                event_t *e = _iter->data;
                                g->name = ((event*)_iter->data)->description;

                                node_t *found = find_node(goals_base,c,g);

                                if(NULL != found) {
                                        goal_t *found_goal = (goal_t*)found->data;

                                        if(found_goal->repeating == 0) {
                                                delete_node(&goals_base,c,found_goal);
                                        }
                                        else {
                                                //found_goal->e_val *= 2;
                                                found_goal->e_val *= pow(
                                                        2,
                                                          fabs(tm_difftime(&e->end_time,&e->start_time)/3600)
                                                          /
                                                          found_goal->duration
                                                );
                                        }
                                }

                                free(g);
                                _iter = _iter->next;
                        }
                }
                //

                // Main loop - while first pointer is not at the end
                while(get_event_id(ptr1) != day_end_event.event_id) {

                        event *e1 = (event*)ptr1->data;
                        event *e2 = (event*)ptr2->data;

                        double free_time = 
                                tm_difftime(
                                        &(e1->end_time),
                                        &(e2->start_time)
                                        );

                        if(get_event_id(ptr1) == get_event_id(ptr2)) {
                                ptr2 = ptr2->next;
                                continue;
                        }

                        if(free_time >= 0) {
                                ptr1 = ptr1->next;
                                continue;
                        }

                        // If events are not overlapping
                        free_time = fabs(free_time);
                        //

			node_t *goals = copy_list(goals_base);
			//int n = list_size(goals);

                        // Delete all goals not withing time bounds
                        // NEW
                        {
                                node_t *iter = goals;
                                while(iter != NULL) {

                                        goal_t *goal = (goal_t*)iter->data;

                                        bool lower_check = compare_24h(&e1->end_time,&goal->lower_bound) >= 0;
                                        struct tm upper_bound = e1->end_time;
                                        float_to_tm(goal->duration,&upper_bound);

                                        bool upper_check = compare_24h(&upper_bound,&goal->upper_bound) <= 0;

                                        if(!(lower_check && upper_check)) {
                                                delete_node(&goals,c,goal);
                                        }
                                        iter = iter->next;
                                }
                        }
                        //

                        while(goals != NULL) {
                                int choice_index = weighted_choice_goals(goals);
                                goal_t *choice = (goal_t*)(get_node_at(goals,choice_index)->data);

                                if(free_time >= choice->duration*3600) {

                                        event *new_event = malloc(sizeof *new_event);
                                        new_event->event_id = first_free_ID++;
                                        new_event->description = strdup(choice->name);
			        	new_event->repeat_mode = 0;
			        	new_event->repeat_frequency = 0;
                                        new_event->num_of_skipped_dates = 0;

                                        memcpy(&new_event->start_time,&((event*)ptr1->data)->end_time,sizeof((event*)ptr1->data)->end_time);
                                        memcpy(&new_event->end_time,&new_event->start_time,sizeof(new_event->start_time));
                                        //new_event->end_time.tm_hour += (int)choice->duration;
                                        float_to_tm(choice->duration,&new_event->end_time);

                                        int insert_index = find_node_index(events,c_e,ptr1->data);

                                        insert_after(&events,new_event,sizeof *new_event,insert_index);

                                        free_time -= choice->duration*3600;

			        	if(choice->repeating == 0) {
			        		delete_node(&goals,c,choice);
			        		delete_node(&goals_base,c,choice);
			        	}
                                        else if(choice->repeating == 1) {
                                                node_t *found = find_node(goals_base,c,choice);

                                                if(found != NULL) {
                                                        goal_t *tmp = (goal_t*)found->data;
                                                        tmp->e_val *= 2;
                                                }
                                        }

                                        break;

                                }
                                else {
                                        delete_node(&goals,c,choice);
                                }
                        }

			ptr1 = ptr1->next;
			delete_list(goals);

                }

                // Remove start and end events
                delete_node(&events,c_e,get_node_at(events,0)->data);
                delete_node(&events,c_e,get_node_at(events,list_size(events)-1)->data);
                //
		
		print_list(events,print_event_short);

                node_t *iter = events;

                while(iter != NULL) {
                        if(!have_ID[((event*)(iter->data))->event_id]) {
				print_event_long(iter->data);
                                save_event(iter->data);
                        }

                        iter = iter->next;
                }

		//delete_list(goals_base);
                //delete_list(events);

                free(c);
                free(c_e);
                //free(date_string);
		return events;
}


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

void print_goal(void *g)
{
        goal_t *goal = (goal_t*)g;

        printf("Name: %s, Duration: %f, E_val: %lf, Repeating: %d, Lower bound: %f, Upper bound: %f\n",
                goal->name,
                goal->duration,
                goal->e_val,
		goal->repeating,
                1.0*goal->lower_bound.tm_hour + 0.01*goal->lower_bound.tm_min,
                1.0*goal->upper_bound.tm_hour + 0.01*goal->upper_bound.tm_min);
}

node_t *read_goals(const char *goal_file)
{
        FILE *input = fopen(goal_file,"r");

        if(NULL == input) {
                input = fopen(goal_file,"w");
                fprintf(input,"# Description,Duration(float)(hours),Expected days to event(float),Repeating(0/1),Lower Bound(float)(time),Upper Bound(float)(time)");
                fclose(input);
                Assert(0,"Error opening goals file. Creating goals.txt in data directory.");
        }

        char *line = NULL;
        size_t bytes_allocated = 0;

        node_t *goals = NULL;

        while(-1 != getline(&line,&bytes_allocated,input)) {
                
                // Skip comments or newlines
                if(line[0] == '#' || line[0] == '\n') continue;

                goal_t *goal = malloc(sizeof *goal);

                tokenizer_t *t = create_tokenizer(line,",");

                goal->name = tokenizer_get(t,NAME_FIELD);
                goal->duration = strtof(tokenizer_get(t,DURATION_FIELD),NULL);
                goal->e_val = strtod(tokenizer_get(t,EVAL_FIELD),NULL);
		goal->repeating = strtol(tokenizer_get(t,REPEAT_FIELD),NULL,10);

                // read bounds
                goal->lower_bound.tm_sec = 0;
                goal->lower_bound.tm_min = 0;
                goal->lower_bound.tm_hour = 0;
                float_to_tm(strtof(tokenizer_get(t,LOWERBOUND_FIELD),NULL),&goal->lower_bound);

                goal->upper_bound.tm_sec = 0;
                goal->upper_bound.tm_min = 0;
                goal->upper_bound.tm_hour = 0;
                float_to_tm(strtof(tokenizer_get(t,UPPERBOUND_FIELD),NULL),&goal->upper_bound);

                add_right(&goals,goal,sizeof *goal);

        }

	fclose(input);
        return goals;
}


char *input(const char *prompt)
{
        printf("%s",prompt);
        char *line = NULL;
        size_t bytes_allocated = 0;

        ssize_t bytes_read = getline(&line,&bytes_allocated,stdin);
        Assert(-1 != bytes_read,"Readline failed");

        line[bytes_read-1] = '\0';
        return line;
}

unsigned int get_event_id(node_t *ptr)
{
        return ((event*)ptr->data)->event_id;
}

int compare_goals_by_name(void *a,void *b)
{
        goal_t *g1 = a;
        goal_t *g2 = b;

        return strcmp(g1->name,g2->name);
}

int compare_events_by_id(void *a,void *b)
{
        event *e1 = (event*)a;
        event *e2 = (event*)b;

        int id1 = e1->event_id;
        int id2 = e2->event_id;

        return id1 - id2;
}


void clear_date(char *date)
{
	event *arr;
	int arr_size;

	get_events_on_date(string_to_time(date),&arr,&arr_size);

	for(int i=0;i<arr_size;++i) {

                if(arr[i].repeat_mode == 0 && delete_event_prompt(arr[i].event_id)) {
                        delete_event(arr[i].event_id);
                }

		if(arr[i].repeat_mode != 0 && skip_date_prompt(arr[i].event_id,date)) {
			skip_date(string_to_time(date),arr + i);
		}
	}

	free(arr);
}

status delete_event_prompt(unsigned event_id)
{

		Assert(have_ID[event_id],"Delete argument ID doesn't exist");

		printf("Delete event: ");
		print_event_long(events + event_id);
		printf("[y/[N]]?: ");

		char *opt = lineread(stdin,"");
                int result = opt[0] == 'y';

                free(opt);
                return result;
}

status skip_date_prompt(unsigned event_id,const char *date)
{
	Assert(have_ID[event_id],"Skip ID doesn't exist");

	printf("Skip event ");
	print_event_long(events + event_id);
	printf("On date %s\n",date);
	                                    
        char *opt = lineread(stdin,"[y/[N]]?: ");

        int yes = opt[0] == 'y';
        free(opt);
        return yes;
}

status skip_date(struct tm date_to_skip,event *e)
{
	Assert(0 == delete_event(e->event_id),"Error deleting event");

	e->num_of_skipped_dates++;
	Assert(NULL != (e->skipped_dates = realloc(e->skipped_dates,e->num_of_skipped_dates * sizeof(*e->skipped_dates))),
		"Realloc failed");

	e->skipped_dates[e->num_of_skipped_dates-1] = date_to_skip;

	save_event(e);
	return 0;
}

int compare_intervals(const void *i1, const void *i2)
{
	double diff = tm_difftime(&((interval_t*)i1)->time,&((interval_t*)i2)->time);

        if(diff < 0) return 1;
        if(diff > 0) return -1;
        return 0;
}


float free_time(event *arr,const int arr_size,bool *overlap)
{
	int intervals_count = 0;
	interval_t *intervals = calloc(arr_size*2,sizeof *intervals);
	Assert(NULL != intervals,"Calloc failed");
	float freetime = DAY_SECS;

	for(int i=0;i<arr_size;++i) {

		event e = arr[i];

		// Skip whole day events
		intervals[intervals_count].time = e.start_time;
		intervals[intervals_count].opening_closing = 1;
		++intervals_count;

		intervals[intervals_count].time = e.end_time;
		intervals[intervals_count].opening_closing = -1;
		++intervals_count;
	}

	qsort(intervals,intervals_count,sizeof *intervals,compare_intervals);

	int prev = 0;
	for(int i=0;i<intervals_count;++i) {

		if(prev != 0 && i != 0) {
			freetime -= fabs(tm_difftime(&intervals[i].time,&intervals[i-1].time));
		}

		prev += intervals[i].opening_closing;

		if(prev > 1 || prev < -1) {
			*overlap = true;
		}
	}

	free(intervals);
	return freetime/(3600);
}

const char *weekday(int wday)
{
	// Adjust wday because tm_wday starts with 0 -> Sunday
	switch((wday+6)%7) {
		case 0: return "Monday";
		case 1: return "Tuesday";
		case 2: return "Wednesday";
		case 3: return "Thursday";
		case 4: return "Friday";
		case 5: return "Saturday";
		case 6: return "Sunday";
		default: return NULL;
	}
}

int compare_chrono_order(const void *e1,const void *e2)
{
	event ev1 = *(event*)e1;
	event ev2 = *(event*)e2;

	//time_t diff = tm_difftime(&ev1.end_time,&ev2.end_time);
	time_t diff = tm_difftime(&ev1.start_time,&ev2.start_time);

	if(diff < 0) { return -1; }
	else if (diff > 0) { return 1; }
	return 0;
}

event* get_events_on_date(const struct tm time,event** arr,int *arr_size)
{
	struct tm lower_bound = time;
	struct tm upper_bound = lower_bound;
	shift_t shift = daily;
	shift_time(&upper_bound,shift,1);

	int arr_allocated = 1;
	(*arr_size) = 0;

	(*arr) = calloc(arr_allocated,sizeof **arr);
	Assert(NULL != (*arr),"Error allocating memory");

	for(unsigned i=1;i<=max_ID;++i) {
		if(have_ID[i]) {
			node_t *ev = event_on_date(events+i,&lower_bound,&upper_bound);

			while(NULL != ev) {
				// Realloc if needed
				if(*arr_size == arr_allocated) {
					arr_allocated *= 2;
					Assert(NULL != ((*arr) = realloc((*arr),arr_allocated * sizeof(**arr))),"Realloc failed");
				}
				
				(*arr)[(*arr_size)++] = *(event*)ev->data;
                                ev = ev->next;
			}
		}
	}
	
	qsort((*arr),*arr_size,sizeof **arr,compare_chrono_order);

	return (*arr);
}

void answer_query(const struct tm time)
{
	struct tm lower_bound = time;

	event *arr;
	int arr_size;

	get_events_on_date(time,&arr,&arr_size);

	bool events_overlapping = false;
	float freetime = free_time(arr,arr_size,&events_overlapping);

	char tmp[DATE_SIZE_MAX];
	strftime(tmp,DATE_SIZE_MAX,DATE_FMT,&lower_bound);

	printf("%s - %s\n",weekday(time.tm_wday),tmp);
	printf("Free time: %.2fh\n",freetime);

	if(events_overlapping) {
		printf("Events overlapping!\n");
	}

	for(int i=0;i<arr_size;++i) {
		print_event_short(arr + i);
	}

	printf("\n");

	free(arr);

}

bool event_is_skipped(const event *e,struct tm *date)
{
	for(unsigned i=0;i<e->num_of_skipped_dates;++i) {

		if(tm_difftime(e->skipped_dates + i,date) == 0){ 
			return true;
		}
	}

	return false;
}

struct tm* tm_min(struct tm *time1, struct tm *time2)
{
	return tm_difftime(time1,time2) < 0 ? time1 : time2;
}

struct tm* tm_max(struct tm *time1, struct tm *time2)
{
	return tm_difftime(time1,time2) < 0 ? time2 : time1;
}


node_t* event_on_date(const event *e,struct tm *lower_bound,struct tm *upper_bound)
{

        node_t* events = NULL;
	if(event_is_skipped(e,lower_bound)) return NULL;

	event *e_cpy = malloc(sizeof *e_cpy);
	Assert(NULL != memcpy(e_cpy,e,sizeof *e),"Error copying struct");

	while(tm_difftime(&e_cpy->start_time,upper_bound) < 0) {

		// Intersection
		if(!(tm_difftime(&e_cpy->end_time,lower_bound) < 0)) {

                        event *new_event = malloc(sizeof *new_event);
	                Assert(NULL != memcpy(new_event,e_cpy,sizeof *e_cpy),"Error copying struct");
			new_event->start_time = *(tm_max(lower_bound,&new_event->start_time));
			new_event->end_time = *(tm_min(upper_bound,&new_event->end_time));
                        add_right(&events,new_event,sizeof *new_event);

                        free(new_event);

		}
		// Break if event not repeating
		if(e->repeat_mode == none) break;

	        shift_time(&e_cpy->start_time,e->repeat_mode,e->repeat_frequency);
	        shift_time(&e_cpy->end_time,e->repeat_mode,e->repeat_frequency);

	}

	free(e_cpy);
        return events;
}

time_t shift_time(struct tm *time,shift_t shift,int by_amount)
{

        time->tm_isdst = -1;

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
        time->tm_isdst = -1;

	return shifted_Epoch;
}

void print_usage()
{
	printf("Usage:\n"
		"    -n --new - Add new event\n"
		"    -p - Print all events\n"
		"    -g --generate <date> - Generate schedule\n"
		"    -d --delete <id> - Delete event with ID <id>\n"
		"    -q --query <date> - Query events on <date>\n"
		"    -w <date> - Print week's worth of events from <date>\n"
		"    -q --query <date> -s <id> - Skip event with <id> on <date>\n"
		"    -q --query <date> --clear-date - Skip all events on <date>\n"
		"    -q --query <date> -f <num> - Print events for <num> of dates from <date>\n"
		);
}

void event_destructor(void *e)
{
	event* ev = (event*)e;
	free(ev->description);

	if(ev->num_of_skipped_dates > 0) {
		free(ev->skipped_dates);
	}
}

status save_event(event *e)
{
        Assert(-1 != chdir("events"),"Error changing directory.");
	//int txt_len = 5;

        // Save prompt
        printf("Save event: ");
        print_event_long(e);
        printf("[y/[N]]?: ");

        char *opt = lineread(stdin,"");

        if(opt[0] != 'y') {
                free(opt);
                return -1;
        }
        free(opt);
        //

	//char *filename = calloc(
	//	ilogb(log10(e->event_id + LOG_SAFETY))+1 + txt_len,
	//	sizeof *filename
	//	);

        char filename[100];

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
	
	for(unsigned i=0;i<e->num_of_skipped_dates;++i) {
		strftime(tmp,DATE_SIZE_MAX,DATE_FMT,&(e->skipped_dates[i]));
		fprintf(output_file,"%s\n",tmp);
	}

	fclose(output_file);
	//free(filename);
        Assert(-1 != chdir(".."),"Error changing directory.");
	return 0;
}

status validate_chrono_order(struct tm *time1, struct tm *time2)
{
	time1->tm_sec = 0;
	time1->tm_isdst = -1;

	time2->tm_sec = 0;
	time2->tm_isdst = -1;

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

        Assert(-1 != chdir("events"),"Error changing directory!");
	Assert(-1 != unlink(filename),"Error while removing event file");
        Assert(-1 != chdir(".."),"Error returning from directory");
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
		new.end_time.tm_hour = 0;
		new.end_time.tm_min = 0;
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
	tmp = lineread(stdin,"Repeat [0/n|1/d|2/w|3/m|4/y]: ");

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
	Assert(NULL != strptime(tmp,DATE_FMT,&(new_event.start_time)),"Error parsing start date");
	free(tmp);

	// End date

	tmp = lineread(event_file,NULL);
	Assert(NULL != strptime(tmp,DATE_FMT,&(new_event.end_time)),"Error parsing start date");
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

        int tmp_enum;

	// Repeat mode
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%d",&tmp_enum);
        new_event.repeat_mode = tmp_enum;
	free(tmp);

	// Repeat frequency
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%d",&tmp_enum);
        new_event.repeat_frequency = tmp_enum;
	free(tmp);

	// Skipped dates count
	tmp = lineread(event_file,NULL);
	sscanf(tmp,"%u",&(new_event.num_of_skipped_dates));
	free(tmp);

	// LBL
	// Skipped dates
	new_event.skipped_dates = calloc(
		new_event.num_of_skipped_dates,
		sizeof *(new_event.skipped_dates)
	);

	for(unsigned i=0;i<new_event.num_of_skipped_dates;++i) {
		tmp = lineread(event_file,NULL);
		Assert(NULL != strptime(tmp,DATE_FMT,new_event.skipped_dates + i),
			"Error parsing skipped date");
		new_event.skipped_dates[i].tm_sec = 0;
		new_event.skipped_dates[i].tm_min = 0;
		new_event.skipped_dates[i].tm_hour = 0;
		new_event.skipped_dates[i].tm_isdst = -1;
		free(tmp);
	}

	unsigned event_id = new_event.event_id;

	events[event_id] = new_event;
	have_ID[event_id] = 1;

	Assert(-1 != fclose(event_file),"Error closing file");

        return NULL;
}

char* lineread(FILE *stream,const char *prompt)
{
	if(NULL != prompt) {
		printf("%s",prompt);
	}

	char *line = NULL;
	size_t bytes_allocated = 0;
	ssize_t bytes_read;

	Assert(-1 != (bytes_read = getline(&line,&bytes_allocated,stream)),"Getline error");

	// Strip newline
	line[bytes_read-1] = '\0';

	return line;
}

struct tm string_to_time(char *string)
{
	struct tm time;
	strptime(string,DATE_FMT,&time);
	time.tm_sec = 0;
	time.tm_min = 0;
	time.tm_hour = 0;
	time.tm_isdst = -1;
	return time;
}

status destructor()
{
	iterate_events(event_destructor);
	free(events);
        return 0;
}

status init()
{
        // Set data directory
        char *data_dir = concat("/home/",concat(getenv("USER"),"/.config/ccal"));
        //char *events_dir = concat(data_dir,"/events");

        // Set random seed
        srand(time(NULL));
	
	// Change working directory to data_dir, and create if missing
        if(-1 == chdir(data_dir)) {
                system(concat("mkdir -p ",data_dir));
                Assert(0,"Data dir missing. Creating now.");
        }

	iterate_directory("events",set_max_ID);
	have_ID = calloc(max_ID+50,sizeof *have_ID);

	load_events("events");

	for(unsigned int i=1;i<=max_ID;++i) {
		if(have_ID[i] == 0) {
			first_free_ID = i;
			break;
		}
	}

	if(first_free_ID == 1 && max_ID == 0) {
		first_free_ID = 1;
	}
	else if(first_free_ID == 1 && max_ID != 0) {
		first_free_ID = max_ID + 1;
	}

        free(data_dir);

	return 0;
}

status iterate_directory(const char *dirname,void* (*func)(void*))
{
	DIR *data = opendir(dirname);

        if(NULL == data) {
                Assert(-1 != system(concat("mkdir ",dirname)),"Error making directory");
                Assert(0,"Error opening directory. Creating now");
        }
        Assert(-1 != chdir(dirname),"Error changing directory.");

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
        Assert(-1 != chdir(".."),"Error changing directory.");
	return 0;
}

status iterate_events(void (*func)(void*))
{
	for(unsigned int i=1;i<=max_ID;++i) {
		if(have_ID[i]) {
			(func)(events+i);
		}
	}

        return 0;
}

void print_event_short(void *e)
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

void print_event_long(void *e)
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
                case 0: repeat_mode = '0'; break;
                default: Assert(0,"Repeat mode must be in range [1,4]");
	}

	printf("(%u) [%s|%s] [%02d:%02d -> %02d:%02d] [%c|%u] %s\n",
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
        return NULL;
}

char *concat(const char *str1,const char *str2)
{
        int n1 = strlen(str1);
        int n2 = strlen(str2);

        char *tmp = malloc(n1+1 + n2+1);
        strncpy(tmp,str1,n1);
        strncat(tmp,str2,n2);
        return tmp;
}

/* TODO
- swap arrays with llist
- export to .ics
- odvojiti answer_query na delove
- napraviti main.h
*/
