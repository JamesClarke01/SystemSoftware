#Source files
SRCS = facDaemon.c logger.c

#Object files
OBJS = $(SRCS:.c=.o)

TARGET = facDaemon

$(TARGET): $(OBJS) #Link object files
	gcc $^ -o $@

%.o: %.c #Compile all object files
	gcc -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
