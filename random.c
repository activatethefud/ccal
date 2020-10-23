#include "random.h"

double uniform()
{
	return (double)rand()/(double)RAND_MAX;
}

void mult_arr(double *arr,int n,double x)
{
	for(int i=0;i<n;++i) {
		arr[i] *= x;
	}
}

double sum(double *arr,int n)
{
	double _sum = 0;
	for(int i=0;i<n;++i) _sum += arr[i];
	return _sum;
}

int weighted_choice(double *weights,int n)
{
	double choice = uniform()*sum(weights,n);
	double acc;
	int i;

        for(i=0,acc=0;acc < choice;++i) {
		acc += weights[i];
        }

	return i-1;
}

void e_vals_to_probabilities(double *e_vals,int n)
{
        for(int i=0;i<n;++i) {
                e_vals[i] = 1/e_vals[i];
        }

        double _sum = sum(e_vals,n);

        for(int i=0;i<n;++i) {
                e_vals[i] *= 1/_sum;
        }
}

int weighted_choice_goals(node_t *goals)
{
        int n = list_size(goals);
        double *weights = calloc(n,sizeof *weights);

        int i=0;
        while(goals != NULL) {
                weights[i++] = ((goal_t*)goals->data)->e_val;
                goals = goals->next;
        }

        e_vals_to_probabilities(weights,n);
        return weighted_choice(weights,n);
}

void print_arr(double *arr,int n)
{
	for(int i=0;i<n;++i) {
		printf("%lf ",arr[i]);
	}

	printf("\n");
}
