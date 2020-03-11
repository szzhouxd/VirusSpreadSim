CXX = $(CROSSPREFIX)g++

PROGRAM = Virus.exe
OBJECTS = Virus.o
SOURCE = Virus.cpp

INCLUDEDIR = -I..\SDL2-2.0.10\include
LIBDIR = -L..\SDL2-2.0.10\lib
LIBS = -lmingw32 -lSDL2main -lSDL2_ttf -lSDL2 -lfreetype
CXXFLAGS = -O2 -DFONTPATH=\"C:/Windows/Fonts/msyh.ttc\"
LDFLAGS = -s -mwindows

$(PROGRAM) : $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(LIBDIR) $(LIBS)

$(OBJECTS) : $(SOURCE)
	$(CXX) -c $(SOURCE) -o $(OBJECTS) $(CXXFLAGS) $(INCLUDEDIR)

.PHONY : clean

clean :
	-DEL $(PROGRAM) $(OBJECTS)
