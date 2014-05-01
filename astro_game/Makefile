SOURCES=$(wildcard src/*.cpp)
OBJS=$(SOURCES:.cpp=.o)
 
# compiler options : add debug information in debug mode
# optimize speed and size in release mode
ifneq (,$(findstring debug,$(MAKECMDGOALS)))
  CFLAGS=-g
else
  CFLAGS=-O2 -s
endif

# linker options : OS dependant
ifeq ($(shell sh -c 'uname -s'),Linux)
  LIBFLAGS=-L. -ltcod -ltcodxx -Wl,-rpath=. -lSDL
else
  LIBFLAGS=-Llib -ltcod-mingw -static-libgcc -static-libstdc++ -mwindows -lmingw32 -lSDLMain -lSDL
endif
 
debug : tuto
release : tuto
 
tuto : $(OBJS)
	g++ $(OBJS) -o tuto -Wall $(LIBFLAGS) $(CFLAGS) -g
 
src/main.hpp.gch : src/*.hpp
	g++ src/main.hpp -Iinclude -Wall
 
src/%.o : src/%.cpp
	g++ $< -c -o $@ -Iinclude -Wall $(CFLAGS)
 
clean :
	rm -f src/main.hpp.gch $(OBJS)
