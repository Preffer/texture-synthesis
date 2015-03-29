CC			= g++
CFLAGS		= -std=c++11 -fopenmp -Wall -march=native -O3 -DNDEBUG
LINKFLAGS	= -lboost_program_options -lpng -ljpeg -ltiff
SRCS		= main.cpp
OBJS		= $(SRCS:.cpp=.o)
PROG		= main

all: $(SRCS) $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(INCFLAGS) $(LINKFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCFLAGS)

clean:
	rm $(OBJS) $(PROG)
