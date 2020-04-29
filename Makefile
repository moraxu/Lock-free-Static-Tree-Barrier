CPPFLAGS := -I../include -g -L.. -lmodel -rdynamic

test1: test1.o
	g++ -o test1 test1.o $(CPPFLAGS)

test2: test2.o
	g++ -o test2 test2.o $(CPPFLAGS)

test1.o: test1.cpp STBarrier.h
	g++ -c test1.cpp $(CPPFLAGS)

test2.o: test2.cpp STBarrier.h
	g++ -c test2.cpp $(CPPFLAGS)

clean:
	rm -f test1 test2 *.o