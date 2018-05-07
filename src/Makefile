OBJ = mpradio.o control_pipe.o player.o settings_provider.o files.o libs/inih/ini.o libs/inih/cpp/INIReader.cpp
CXXFLAGS=-Wall 

mpradio: $(OBJ)
	g++ -pthread -o mpradio $(OBJ) -lid3

-include dependencies

.PHONY: depend clean cleanall

depend:
	g++ -MM *.cc > dependencies
clean:
	rm -f *.o
cleanall:
	rm -f mpradio *.o *~

