# schedule Makefile
# File: Makefile
# Author: Bryce Jarboe | RedID 825033151
# Date: 3/7/2024

COMPILER=g++

FLAGS=-std=c++11 -Wall -g3 -c

OBJS = schedule.o log.o 

PROGRAM = schedule

# Rules format:
# target : dependency1 dependency2 ... dependencyN
#     Command to make target, uses default rules if not specified

# First target is the one executed if you just type make
# make target specifies a specific target
# $^ is an example of a special variable.  It substitutes all dependencies
$(PROGRAM) : $(OBJS)
	$(COMPILER) -o $(PROGRAM) $^

schedule.o : schedule.h schedule.cpp
	$(COMPILER) $(FLAGS) schedule.cpp
log.o : log.h log.c
	$(COMPILER) $(FLAGS) log.c

clean :
	rm -f *.o $(PROGRAM)