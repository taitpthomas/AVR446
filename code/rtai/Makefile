all: scope rt_process.o

LINUX_HOME = /usr/src/linux
RTAI_HOME = /home/rtai-1.4
INCLUDE = -I/include -I/include
MODFLAGS = -D__KERNEL__ -DMODULE -O2 -Wall

scope: scope.c
    gcc -o $@ $<

rt_process.o: rt_process.c
    gcc -c -o $@ $<

clean:
    rm -f rt_process.o scope
