OBJ = mpradio.o player.o fsrun.o settings_provider.o
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

