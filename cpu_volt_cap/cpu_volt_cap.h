/*
#
# Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef VOLT_CAP_H
#define VOLT_CAP_H

struct rail_stats {
    int milli_volts;
    long time_10msec;
};

struct temperatures_stats {
    int current_temp;
    int is_rise;
};

struct saved_data {
    float balance_points;
    struct timespec saved_time;
    unsigned int edid;
};

struct thread_data {
    pthread_t t;
    pthread_attr_t a;
    void * (*f)(void *);
    void *d;
};

struct command{
    char *cmd;
    int (*f)(FILE *fp);
};

int get_burnout_table(float **bt, int *size);
int get_temperature_list(int **tlist, int *size);

#endif
