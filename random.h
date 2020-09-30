#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef RANDOM_H
#define RANDOM_H

double uniform();
void mult_arr(double *arr,int n,double x);
double sum(double *arr,int n);
int weighted_choice(double *weights,int n);

#endif