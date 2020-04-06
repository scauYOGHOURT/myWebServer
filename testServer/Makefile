target := SERVER

sources := ${wildcard *.cpp}
headfiles := ${wildcard *.h}
objects := ${sources:.cpp=.o}

CC := g++ -std=c++11 -g
RM := rm -rf 
LIB := -lpthread

${target}: ${objects}
	${CC} -o $@ $^ ${LIB}

${objects}: %.o: %.cpp ${headfiles}
	${CC} -o $@ -c $< 

.PHONY: clean cleanall
cleanall:
	${RM} ${target} ${objects}

clean:
	${RM} ${objects}