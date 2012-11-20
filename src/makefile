OPTS  = -Wall -Wextra -g -l glpk
OPTI  = -O3 -pipe
HEADS = uls/*.cpp cls/*.cpp mcls/*.cpp *.cpp
OBJS  = *.o
NAME  = ../bin/solveLotSizing

launch : head
	g++ $(OBJS) $(OPTS) -o $(NAME)

head : $(HEADS)
	g++ $(HEADS) $(OPTS) -c 

opti : header
	g++  $(OBJS) $(OPTS) $(OPTI) -o $(NAME)

header : $(HEADS)
	g++ $(OPTS) $(OPTI) -c $(HEADS)

clean :
	rm -rf $(OBJS) $(NAME)

cleaner :
	rm -rf $(OBJS) $(NAME) *~ uls/*~ cls/*~ mcls/*~
