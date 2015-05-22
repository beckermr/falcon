
CC          =  gcc
OPTIMIZE    =  -g -O3 #-Wall -wd981 #-wd1419 -wd810
CLINK=$(CC)
CFLAGS=$(OPTIMIZE)
CLIB=

EXEC = test_falcon
check: $(EXEC) 

OBJS1= main.o falcon.o
$(EXEC): $(OBJS1)
	$(CLINK) $(CFLAGS) -o $@ $(OBJS1) $(CLIB)
	./test_falcon

.PHONY : clean
clean: 
	rm -f *.o

.PHONY : spotless
spotless: 
	rm -f *.o $(EXEC)

.PHONY : pristine
pristine:
	rm -f *.o $(EXEC) *~

