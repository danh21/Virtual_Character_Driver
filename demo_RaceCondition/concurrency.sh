#!/bin/bash

# create 3 processes execute change_CR program at the same time on different cores

taskset -c 0 ./change_CR &
taskset -c 1 ./change_CR &
taskset -c 2 ./change_CR &
