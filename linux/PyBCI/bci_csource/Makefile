CC = g++
CFLAGS= -Wall
LFLAGS= -lpthread -lglut
INCLUDE = -I/usr/include/GL
IFLAGS = -I/usr/include/python2.5

_bci_source.so: bci_source.o sign.o bitmap_loader.o bci_source_wrap.o 
	$(CC) -shared bci_source.o sign.o bitmap_loader.o bci_source_wrap.o -o _bci_source.so $(LFLAGS)

bci_source_wrap.o: bci_source_wrap.cxx
	$(CC) -c bci_source_wrap.cxx $(IFLAGS)

sign.o:  sign.h sign.cpp  BitmapLoader.h
	$(CC) -c $(CFLAGS) sign.cpp $(LFLAGS) $(INCLUDE)

bitmap_loader.o: BitmapLoader.h BitmapLoader.cpp
	$(CC) -c $(CFLAGS) BitmapLoader.cpp -o bitmap_loader.o

bci_source.o: bci_source.cpp bci_source.h RecorderRDA.h sign.h
	$(CC) -c $(CFLAGS) bci_source.cpp $(LFLAGS)




