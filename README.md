# What is SDLRogue
SDLRogue is a roguelike RPG built on SDL, written in C++. It was a hobby project that ran until 2009, at which point it  was abandoned.

# Why is it on GitHub
This project is on GitHub mostly for archival purposes. I figured that it would be a more natural fit than keeping a ZIP file somewhere on OneDrive or the like.

# What is the future of this project
There is none, I do not intend to develop it further.

# How to build and run it
My main development platform was Windows at the time, though the code should probably build and run on Linux or MacOs with minimal changes, though I offer no guarantees of that.

## Dependencies
In order to build the project on Windows, the following libraries are required:
- SDL.dll
- SDL_ttf.dll
- zlib1.dll
- libfreetype-6.dll
These can be found on SDL's website: http://www.libsdl.org

Additionally the following SDL header files should be included in the PATH environment variable:
- SDL.h
- SDL_ttf.h

## Building
Building the project using MinGW can be done with the following script
```
g++ -c SDLRogue.cpp
g++ -c sdrclass.cpp
g++ -o SDLRogue SDLRogue.o sdrclass.o -I SDL/include -L SDL/lib -lmingw32 -lSDLmain -lSDL -lSDL_ttf -mwindows
```