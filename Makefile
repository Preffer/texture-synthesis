CC			= g++
CFLAGS		= -std=c++14 -Wall -march=native
LINKFLAGS	= -ljpeg
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