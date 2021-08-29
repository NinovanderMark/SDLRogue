# What is SDLRogue
SDLRogue is a roguelike RPG built on SDL 1.2, written in C++. It was a hobby project that ran until 2009, at which point it  was abandoned.

# Why is it on GitHub
This project is on GitHub mostly for archival purposes. I figured that it would be a more natural fit than keeping a ZIP file somewhere on OneDrive or the like.

# What is the future of this project
There is none, I do not intend to develop it further.

# How to build and run it
My main development platform was Windows at the time, though the code should probably build and run on Linux or MacOs with minimal changes, though I offer no guarantees of that.

## Dependencies
In order to build the project on Windows, you are required to have SDL 1.2 and a compatible version of SDL_ttf available. These should be available through http://www.libsdl.org

I'm aware that anything prior to SDL2 has been deprecated, and don't intend to do anything about that. The API looks sufficiently different that a simple find+replace won't get this to run, which is about as much trouble as I was willing to go to.

If you really intend to get this running, I wish you the best of luck.

## Building
Building the project using MinGW can be done with the following script
```
g++ -c SDLRogue.cpp
g++ -c sdrclass.cpp
g++ -o SDLRogue SDLRogue.o sdrclass.o -I SDL/include -L SDL/lib -lmingw32 -lSDLmain -lSDL -lSDL_ttf -mwindows
```