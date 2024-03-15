#Source files
SRCS = daemonSetup.c facDaemon.c logger.c fileTransfer.c signal.c

#Object files
OBJS = $(SRCS:.c=.o)

TARGET = facDaemon

$(TARGET): $(OBJS) #Link object files
	gcc $^ -o $@

%.o: %.c #Compile all object files
	gcc -c $< -o $@

removeObjects:
	rm -f $(OBJS)
