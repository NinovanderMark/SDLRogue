#include "SDL/include/SDL.h"
#include "SDL/include/SDL_ttf.h"

#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../rrlib/rrclass.h"
#include "sdrclass.h"

using namespace std;

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 16;

const int FILEVER = 001;

SDL_Surface *screen; //This pointer will reference the backbuffer

// These are the World Variables, they contain all aspects of the game world
const int maxrooms = 256; 	// Maximum amount of rooms per level allowed
const int maxroomd = 40; 	// Maximum room dimension (either width or height)
const int maxenca = 8; 		// Maximum amount of encounter areas per level allowed

int roomset[maxroomd][maxroomd]; 	// Tiles, 256 Rooms capable, each with a maximum size of 40x40
int fgset[maxroomd][maxroomd]; 		// Foreground layer
int objset[maxroomd][maxroomd][2]; 	// Entities, every location can hold an object defined by objecttype and objectid
int objects[5000][31];				// Unique object array. First value represents the objectid, the others several
									// variables this object may have. This will differ per object.
int encset[maxenca][9]; 			// Encounter layer (used to see which areas the player can encounter hostiles in)

int tileentry[32767][7];		// Tile entries. As the file would have. The last of the 7 positions is editor-only
int fgentry[32767][7];			// Foreground entries. As the file would have. The last of the 7 positions is editor-only
int objectentry[32767][5];		// Entity entries. As the file would have.
int objectientry[32767][3];		// Object index entries. As the file would have.
int encentry[32767][11];		// Encounter set entries. As the file would have.

int curte; int curfe; int curoe; int curie; int curee; // Current # entry for all 

bool loadroom( const char* filetoload ); // This loads the room into the world variables

// =========
// INT MAIN
// =========

int main()
{

}

// This checks to see if the file version matches the expected value
bool vercheck(const int reqver, int readver)
{
	if ( readver != reqver ) 
	{
		fprintf(stderr, "Incompatible file versions\n");
		if ( readver > 24 && readver < 50 ) { fprintf(stderr, " - You didn't really believe a Xubutor file would work here did you?\n"); }
		return false;
	}
	else { return true; }
}

// Reads an integer from the currently opened file
int readintfromfile(char endline)
{
	string tempstr; int tempint;
	getline (infile,tempstr,endline);
	istringstream buffer(tempstr); buffer >> tempint;
	return tempint;
}

// This loads the room into the world variables
bool loadroom( const char* filetoload )
{
	curobjs = 0;
	if ( filexist(filetoload) == true ) // If the file exists, load it's settings
	{  
		infile.open (filetoload, fstream::in); fprintf(stdout, "[I] Loading level '"); fprintf(stdout, filetoload); fprintf(stdout,"'...\n");
		string tmpstr; bool donecycle = false; int line = 0; getline (infile,tmpstr,','); // First line is to be discarded
		int intvar = readintfromfile(',');
		if ( vercheck(FILEVER,intvar) == true )
		{
			while ( donecycle == false && line <= 409600 ) // Read the Tiles
			{
				// Read next variable and see if it's a valid number or an 'end of section' character
				int tehroom = readintfromfile(',');
				if ( tehroom >= 0 )
				{
					// Read the next 6 variables from the file for tileentry
					int tmpa = 0;
					while ( tmpa < 6 )
					{
						tileentry[curte][tmpa] = readintfromfile(',');
						tmpa++;
					}
				}
				else 
				{ 
					donecycle = true;
					if ( line == 0 ) { return false; }
					else { fprintf(stdout, " |- Tile zones read: "); inttoout(line); fprintf(stdout, ".\n"); } 
				}
				line++;
			}
			if ( donecycle == false ) { error(1,5,0); infile.close(); return false; }
		 
			line = 0; donecycle = false;
			while ( donecycle == false && line <= 409600 ) // Read the foreground layer
			{
				int tehroom = readintfromfile(',');
				if ( tehroom >= 0 )
				{
					int mrx = readintfromfile(',');
					if ( mrx >= 40 ) 
					{
						mrx = readintfromfile(',');
						int mry = readintfromfile(',');
						fgset[tehroom][mrx][mry] = readintfromfile(',');
					}
					else
					{
						int mry = readintfromfile(',');
						int mrsx = readintfromfile(',');
						int mrsy = readintfromfile(',');
						int intvar = readintfromfile(',');
						   
						tmpa = mrx; tmpb = mry;
						while ( tmpb <= mrsy )
						{
							while ( tmpa <= mrsx )
							{
								fgset[tehroom][tmpa][tmpb] = intvar;
								tmpa++;
							}
							tmpb++;
							tmpa = mrx;
						}
					}
				}
				else 
				{ 
					donecycle = true; 
					if ( line == 0 ) { error(1,9,0); }
					else { fprintf(stdout, " |- Secondary tile zones read: "); inttoout(line); fprintf(stdout, ".\n"); } 
				}
				line++;
			}
			if ( donecycle == false ) { error(1,6,0); infile.close(); return false; }

			line = 0; donecycle = false;
			while ( donecycle == false ) // Read the Object layer
			{
				int tehroom = readintfromfile(',');
				if ( tehroom >= 0 ) 
				{
					int mrx = readintfromfile(',');
					int mry = readintfromfile(',');
					int datnum = readintfromfile(',');
					objset[tehroom][mrx][mry][datnum] =  readintfromfile(',');
				}
				else 
				{ 
					donecycle = true;
					if ( line == 0 ) { error(1,9,0); }
					else { fprintf(stdout, " |- Entities read: "); inttoout(line); fprintf(stdout, ".\n"); } 
				}
				line++;
			}
			if ( donecycle == false ) { error(1,3,0); infile.close(); return false; }

			line = 0; donecycle = false;
			while ( donecycle == false && line <= 160000 ) // Read the specific objects
			{
				int objnum = readintfromfile(',');
				if ( objnum >= 0 )
				{
					int objvar = readintfromfile(',');
					object[objnum][objvar] = readintfromfile(',');
					objectused[objnum] = true;
				}
				else 
				{ 
					donecycle = true;
					if ( line == 0 ) { error(1,9,0); }
					else { fprintf(stdout, " |- Objects read: "); inttoout(line); fprintf(stdout, ".\n"); }
				}
				if ( objnum > curobjs ) { curobjs = objnum; }
				line++;
			}
			if ( donecycle == false ) { error(1,4,0); infile.close(); return false; }

			line = 0; donecycle = false;
			while ( donecycle == false && line <= 2048 ) // Read the encounter areas
			{
				int tehroom = readintfromfile(',');
				if ( tehroom >= 0 )
				{
					int encareanum = readintfromfile(',');
					encset[tehroom][encareanum][0] = readintfromfile(',');
					encset[tehroom][encareanum][1] = readintfromfile(',');
					encset[tehroom][encareanum][2] = readintfromfile(',');
					encset[tehroom][encareanum][3] = readintfromfile(',');
					encset[tehroom][encareanum][4] = readintfromfile(',');
					encset[tehroom][encareanum][5] = readintfromfile(',');
					encset[tehroom][encareanum][6] = readintfromfile(',');
					encset[tehroom][encareanum][7] = readintfromfile(',');
					encset[tehroom][encareanum][8] = readintfromfile(',');
				}
				else 
				{ 
					donecycle = true; 
					if ( line == 0 ) { error(1,9,0); }
					else { fprintf(stdout, " |- Encounter zones read: "); inttoout(line); fprintf(stdout, ".\n"); }
				}
				line++;
			}
			if ( donecycle == false ) { error(1,7,0); infile.close(); return false; } 
		}
		else { fprintf(stderr, " -  Level could not be loaded!!\n"); infile.close(); return false; }
		infile.close();
	}
	else
	{
		error(1,0,0,0,0,0,filetoload);
		return false;
	}
	curobjs++;
	nextobject = curobjs + 1;
}
