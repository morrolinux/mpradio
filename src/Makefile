OBJ = mpradio.o player.o settings_provider.o libs/inih/ini.o libs/inih/cpp/INIReader.cpp
CXXFLAGS=-Wall	

mpradio: $(OBJ)
	g++ -o mpradio $(OBJ)

-include dependencies

.PHONY: depend clean cleanall

depend:
	g++ -MM *.cc > dependencies
clean:
	rm -f *.o
cleanall:
	rm -f mpradio *.o *~

