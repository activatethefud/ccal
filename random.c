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

	double acc = 0;
	int i = 0;

	while(acc < choice) {
		acc += weights[i];
		++i;
	}

	return i-1;
}

void print_arr(double *arr,int n)
{
	for(int i=0;i<n;++i) {
		printf("%lf ",arr[i]);
	}

	printf("\n");
}