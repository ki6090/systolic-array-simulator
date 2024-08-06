CC = g++
CXXFLAGS = -Wall -std=c++17
OBJS = SystolicWS.o Config.o main.o

SystolicWS.o: SystolicWS.h SystolicWS.cpp 
			   $(CC) $(CXXFLAGS) -c SystolicWS.cpp 

Config.o: Config.h Config.cpp 
			$(CC) $(CXXFLAGS) -c Config.cpp 

main.o: main.cpp Config.h SystolicWS.h 
			$(CC) $(CXXFLAGS) -c main.cpp 

sim: $(OBJS)
		$(CC) $(CXXFLAGS) $(OBJS) -o sim

clean: 
	rm -f $(OBJS) sim




