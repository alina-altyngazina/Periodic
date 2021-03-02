CC=gcc
CFLAGS=-fPIC
LDFLAGS=-shared
SRC=./src
LIBS=$(shell pwd) 

all:libmessage.so libproject.so launch_daemon periodic period

libmessage.so: $(SRC)/message.c 
	$(CC) $(LDFLAGS) -o libmessage.so $(CFLAGS) $(SRC)/message.c
# sudo cp libmessage.so /usr/lib
# sudo chmod 0755 /usr/lib/libmessage.so
# sudo ldconfig

libproject.so: $(SRC)/libproject.c 
	$(CC) $(LDFLAGS) -o libproject.so $(CFLAGS) $(SRC)/libproject.c
# sudo cp libproject.so /usr/lib
# sudo chmod 0755 /usr/lib/libproject.so
# sudo ldconfig

launch_daemon: $(SRC)/launch_daemon.c 
	$(CC) $(SRC)/launch_daemon.c -o launch_daemon

periodic: $(SRC)/periodic.c 
	$(CC) $(SRC)/periodic.c -o periodic	-L$(LIBS) -Wl,-rpath=$(LIBS) -lproject -lmessage
# $(CC) $(SRC)/periodic.c -o periodic	-lproject -lmessage

period: $(SRC)/period.c
	$(CC) $(SRC)/period.c -o period -L$(LIBS) -Wl,-rpath=$(LIBS) -lproject -lmessage
# $(CC) $(SRC)/period.c -o period -lproject -lmessage

clean:
	rm libmessage.so libproject.so launch_daemon periodic period