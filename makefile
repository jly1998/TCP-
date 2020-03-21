CC = gcc
s_objects = server.o server_func.o
c_objects = client.o client_main.o client_time.o client_poll.o client_epoll.o
p_objects = g01_common.o
targets = 	server client
headfiles = g01_server.h g01_common.h

all : $(targets)

server : $(s_objects) $(p_objects) $(headfiles)
	$(CC) -I./ $^ -o $@ 
	
client : $(c_objects) $(p_objects) $(headfiles)
	$(CC) -I./ $^ -o $@ 
	
%.o : %.c
	$(CC) -c $< 

clean:
	-rm -f $(targets) $(s_objects) $(c_objects) $(p_objects)
	-rm -rf txt
