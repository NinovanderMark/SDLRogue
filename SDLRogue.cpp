#include "SDL/include/SDL.h"
#include "SDL/include/SDL_ttf.h"

#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "sdrclass.h"

using namespace std;

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 16;

SDL_Surface *screen; //This pointer will reference the backbuffer
SDL_Surface *screenbuffer1 = SDL_CreateRGBSurface(SDL_HWSURFACE,SCREEN_WIDTH,SCREEN_HEIGHT,16,0,0,0,0);; //Screen buffer
SDL_Surface *hudbuffer = SDL_CreateRGBSurface(SDL_HWSURFACE,128,SCREEN_HEIGHT,16,0,0,0,0);; //Hud Buffer

SDL_Surface *objimg; //Character sheet
SDL_Surface *worldimg; //World sheet
SDL_Surface *guiimg; // GUI sheet
SDL_Surface *beastimg; // Monster sheet
SDL_Surface *text = NULL; // Text surface

//The event structure
SDL_Event event;

//The fonts to be used
TTF_Font *font1 = NULL;
TTF_Font *font2 = NULL;
TTF_Font *font3 = NULL;

//The portions of the sprite map to be blitted
const int maxclipnum = 64;
SDL_Rect clip[ maxclipnum ];
SDL_Rect miniclip[ 8 ]; // For tiny stuff (keys, some HUD components)

//The camera 
SDL_Rect camera = { 0, 0, SCREEN_WIDTH - 128, SCREEN_HEIGHT};
SDL_Rect camerasize = camera;

int tmpa;
int tmpb;

char cchar[32];

// ********************************
// Program Related Variables
// ********************************

// Game version
const int VERREL = 0; // Release #
const int VERMA = 0;  // Major version
const int VERMI = 7;  // Minor version

const int FILEVER = 1; // File version

// [ Program Sequence ]
bool quit = false; // Quit Variable
bool draws = false; // Should the screen be redrawn?
bool fullscreen = false; // Should the screen be drawn in Fullscreen mode?
bool lastfull = false; // Was the last screen drawn in Fullscreen mode?
bool poscheck = false; // Should the game check if the players current position has any events on it?
bool drawqui = false; // Should the Quick UI be drawn?
bool ignoreESC = false; // Set this when ESCing out of the QUI (To prevent immediate shutdown)
bool ignoreCTRL = false; // Set this when switching to/from UIs to prevent immediatly selecting the next menu option
int drawui = 0; // Should the UI be redrawn? (and if so, which element(s)

ifstream infile;
ofstream destfil;

char* modpath = "base/";

// [ Game Variables ]
const int maxrooms = 256; // Maximum amount of rooms per level allowed
const int maxroomx = 40; // Maximum room width
const int maxroomy = 40; // Maximum room height
const int maxenca = 8; // Maximum amount of encounter areas per level allowed

bool collidable[2][512]; // List of all the tiles, and whether they are collidable or not (0 = worldtiles, 1 = objs)
int roomset[maxrooms][maxroomx][maxroomy]; // Tiles, 256 Rooms capable, each with a maximum size of 40x40
int objset[maxrooms][maxroomx][maxroomy][2]; // Objects, every location can hold an object defined by objecttype and objectid
int fgset[maxrooms][maxroomx][maxroomy]; // Foreground layer
int encset[maxrooms][maxenca][9]; // Encounter layer (used to see which areas the player can encounter hostiles in)
int roomenc[maxroomx][maxroomy]; // Current room's encounter tile list

const int maxarenasize = 12; // Maximum height & width of the arena
bool battlezone[maxroomx][maxroomy]; // Keeps track of which tiles are in the combat zone

int enccnt; // Random Encounter counter. When this reaches a certain threshold it will cause an encounter

int curroom_x; // Variable in which the X length of the current room is stored (used for camera bounds)
int curroom_y; // Variable in which the Y length of the current room is stored (used for camera bounds)

const int maxobjs = 5000;  // Maximum amount of objects
bool objectused[maxobjs];  // Is this object currently used?
int object[maxobjs][32];   // Unique object array. First value represents the objectid, the others several
					  	   // variables this object may have. This will differ per object.
int curobjs;			   // Current amount of Objects
int nextobject;		  	   // Variable used to indicate the next usable object number

Player P1(0,5,5,"MANDUDE GUY"); // The Player Object

// Game Entities (Monsters, weapons etc.)
string monstername[64]; 	// Holds the monster prototype's basenames
int ptmonster[64][4][11]; 	// Holds the monster prototypes
string aiattackname[256];	// Holds the ai attack's names
int aiattack[256][3][9];	// Holds the ai attacks

// Combat Related
int battlebeasts[16][3]; // Array of all non-player battle participants
int lastbeast = 0; // The amount of beasts set in 'battlebeasts'

// [ Visuals Variables ]
bool newrender = true; // Use the new rendering method rather then the original?
int pstep = 2; // # of pixels to move character every cycle when moving in between frames
int stepint = 0; // Interval between moving the character #'pstep' of pixels
int stepcount = 0; // Used to count up to stepint

int frmint = 8; // Interval between drawing the next animation frame
int crfrm = 0; // Used to count up to frmint
int FRAMES_PER_SECOND = 80; // Cap to 80 (Game frames, not rendered frames)

// Set up some colors
SDL_Color rrWhite		= { 255,	255,	255 };
SDL_Color rrLightGrey	= { 192,	192,	192 };
SDL_Color rrGrey		= { 128,	128,	128 };
SDL_Color rrDarkGrey	= { 64,		64,		64  };
SDL_Color rrBlack		= { 0,		0,		0	};
SDL_Color rrLightRed	= { 255,	128,	128 };
SDL_Color rrRed			= { 255,	0,		0 	};
SDL_Color rrDarkRed		= { 128,	0,		0 	};
SDL_Color rrYellow		= { 255,	255,	0	};
SDL_Color rrGreen		= { 0,		255,	0 	};
SDL_Color rrDarkGreen	= { 0,		128,	0 	};
SDL_Color rrCyan		= { 0,		255,	255 };
SDL_Color rrLightBlue	= { 64,		150,	255 };
SDL_Color rrBlue		= { 0,		0,		255 };
SDL_Color rrDarkBlue	= { 0,		0,		128 };
SDL_Color rrBrown		= { 112,	76,		32	};

SDL_Color rrMain = rrBlack; // The 'Main' Color (Now Black)
SDL_Color rrAlt	 = rrDarkRed; // The "Secondary" Color (Now Dark Red)

const int fontsz1 = 10; // Font Size 1 (Used for small things, like the stats)
const int fontsz2 = 12; // Font Size 2 (Used for the Quick GUI and other GUI components)
const int fontsz3 = 14; // Font Size 3 (Used to display Endurance during combat and such)

/*====================================================1 ROUTINES 1===================================================*/

// ========================
// Dependancy prototypes
// ========================

void inttoerr(int integer);
void inttoout(int integer);
void outvideoset();
void error(int typ, int styp, int vara, int varb , int varc , int vard , const char* vare);
void querender(bool inbattle);

// External

int roundn(double a); 								// Procedure to round numbers
int randnum(int min, int max); 						// Procedure to generate random numbers
SDL_Surface *load_image( std::string filename );	// Load image file to a surface

// Initialize SDL_VIDEO
int InitVideo(Uint32 flags = SDL_DOUBLEBUF | SDL_FULLSCREEN) 
{
	// Load SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) 
	{
		fprintf(stderr, "ERROR: Unable to initialize SDL: %s!!\n", SDL_GetError());
		return false;
	}
	atexit(SDL_Quit); // Clean it up nicely

	//set the main screen to SCREEN_WIDTHxSCREEN_HEIGHT with a colour depth of 16:
	outvideoset();
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_DOUBLEBUF | SDL_HWSURFACE );
	if (screen == NULL) 
	{
		fprintf(stderr, "ERROR: Unable to set video mode: %s!!\n", SDL_GetError());
		return false;
	}
	return true;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;
    
    //Get offsets
    offset.x = x;
    offset.y = y;
    
    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

// ========================
// Program System Routines
// ========================

void error(int typ, int styp, int vara, int varb = 0, int varc = 0, int vard = 0, const char* vare = "")
{
	if ( typ == 0 ) // Error type 0 (General Errors)
	{
		 if ( styp == 0 ) { 
			  fprintf(stdout, "[E] Required variable out of range!!\n"); fprintf(stderr, "Error: Variable out of range!!\n - Range: (");
			  inttoerr(varb); fprintf(stderr, " - "); inttoerr(varc); fprintf(stderr, "). Read: ("); inttoerr(vara); fprintf(stderr, ") on Line: "); 
			  inttoerr(vard); fprintf(stderr, ".\n"); }
		 else if ( styp == 1 ) { 
			  fprintf(stdout, "[E] Room width to small!!\n"); fprintf(stderr, "Error: Room X for Room ("); inttoerr(vara);
			  fprintf(stderr, ") too small!!\n - Rooms must be 3x3 or larger.\n"); }
		 else if ( styp == 2 ) {  
			  fprintf(stdout, "[E] Room height to small!!\n"); fprintf(stderr, "Error: Room Y for Room ("); inttoerr(vara);
			  fprintf(stderr, ") too small!!\n - Rooms must be 3x3 or larger.\n"); }
		 else { fprintf(stdout, "[W] Unhandled Exception!\n"); fprintf(stderr, "Warning: An error was thrown, of type 0, but not defined!"); }
		 
	}
	else if ( typ == 1 ) // Error type 1 (File loading Errors)
	{
		 if ( styp == 0 ) { 
		      fprintf(stdout, "[E] Can't open requested file!!\n"); fprintf(stderr, "Error: Can't find or open file '"); fprintf(stderr, vare);
			  fprintf(stderr, "'!!\n"); }
		 else if ( styp == 1 ) { 
				   fprintf(stdout, "[W] Incorrect file version!\n"); fprintf(stderr, "Warning: Incompatible file version for file '"); fprintf(stderr, vare);
				   fprintf(stderr,"'!\n - Read: "); inttoerr(vara); fprintf(stderr, " - Expected: "); inttoerr(varb); fprintf(stderr, ".\n"); }
		 else if ( styp == 2 ) {
				   fprintf(stdout, "[E] Variable read doesn't match required type!!\n"); fprintf(stderr, "Error: Type of Variable Read doesn't match Type Required!!\n"); }
		 else if ( styp == 3 ) {
				   fprintf(stdout, "[E] Maximum # of entities for file exceeded!!\n"); fprintf(stderr, "Error: Number of entities exceeds limit!!\n"); }
		 else if ( styp == 4 ) {
				   fprintf(stdout, "[E] Maximum # of objects for file exceeded!!\n"); fprintf(stderr, "Error: Number of objects exceeds limit!!\n"); }
		 else if ( styp == 5 ) {
				   fprintf(stdout, "[E] Maximum # of tilezones for file exceeded!!\n"); fprintf(stderr, "Error: Number of tilezones exceeds limit!!\n"); }
		 else if ( styp == 6 ) {
				   fprintf(stdout, "[E] Maximum # of secondary tilezones for file exceeded!!\n"); fprintf(stderr, "Error: Number of secondary tilezones exceeds limit!!\n"); }
		 else if ( styp == 7 ) {
				   fprintf(stdout, "[E] Maximum # of encounter zones for file exceeded!!\n"); fprintf(stderr, "Error: Number of encounter zones exceeds limit!!\n"); }
		 else if ( styp == 8 ) {
				   fprintf(stdout, "[E] Maximum # of spawn locations for file exceeded!!\n"); fprintf(stderr, "Error: Number of spawn locations exceeds limit!!\n"); }
		 else if ( styp == 9 ) {
				   fprintf(stdout, "[N] No items found in current section.\n"); }
		 else { fprintf(stdout, "[W] Unhandled Exception!\n"); fprintf(stderr, "Warning: An error was thrown, of type 1, but not defined!"); }		   
	}
	else if ( typ == 2 ) // Error type 2 (Tilesetting Errors)
	{
		 if ( styp == 0 ) {
			  fprintf(stdout, "[E] Maximum # of tiles for collidability exceeded!!\n"); fprintf(stderr, "Error: Number of tiles exceeds limit of 512!!\n"); }
		 else if ( styp == 1 ) {
			  fprintf(stdout, "[E] Maximum # of objects for collidability exceeded!!\n"); fprintf(stderr, "Error: Number of objects exceeds limit of 512!!\n"); }
		 else { fprintf(stdout, "[W] Unhandled Exception!\n"); fprintf(stderr, "Warning: An error was thrown, of type 2, but not defined!"); }
	}
	else if ( typ == 3 ) // Error type 3 (AI related Errors)
	{
		 if ( styp == 0 ) {
			  fprintf(stdout, "[E] Maximum # of attacks for monster exceeded!!\n"); fprintf(stderr, "Error: Monster on line "); inttoerr(vara); fprintf(stderr, " has more then 10 attacks!!\n"); }
		 else if ( styp == 1 ) {
			  fprintf(stdout, "[E] Maximum # of loot types for monster exceeded!!\n"); fprintf(stderr, "Error: Monster on line "); inttoerr(vara); fprintf(stderr, " has more then 10 loot types!!\n"); }
		 else if ( styp == 2 ) {
			  fprintf(stdout, "[E] Maximum # of special loot types for monster exceeded!!\n"); fprintf(stderr, "Error: Monster on line "); inttoerr(vara); fprintf(stderr, " has more then 10 special loot types!!\n"); }
		 else if ( styp == 3 ) {
			  fprintf(stdout, "[E] Maximum # of monsters exceeded!!\n"); fprintf(stderr, "Error: Number of monsters exceeds limit of 64!!\n"); }
		 else if ( styp == 4 ) {
			  fprintf(stdout, "[E] Maximum # of attacks exceeded!!\n"); fprintf(stderr, "Error: Number of attacks exceeds limit of 256!!\n"); }
		 else { fprintf(stdout, "[W] Unhandled Exception!\n"); fprintf(stderr, "Warning: An error was thrown, of type 3, but not defined!"); }
	}
	else { fprintf(stdout, "[W] Non-existant Error type submitted by program!\n"); }
}

// Log function when Video mode is changed during runtime
void outvideoset()
{
	fprintf(stdout, "[I] Setting Video mode: "); inttoout(SCREEN_WIDTH); fprintf(stdout, "x"); inttoout(SCREEN_HEIGHT);
	fprintf(stdout, ", "); inttoout(SCREEN_BPP); fprintf(stdout, " bpp");
	if ( fullscreen == true ) { fprintf(stdout, ", fullscreen...\n"); }
	else { fprintf(stdout, "...\n"); }
}

// Reads an integer from the currently opened file
int readintfromfile(char endline)
{
string tempstr; int tempint;
	getline (infile,tempstr,endline);
	istringstream buffer(tempstr); buffer >> tempint;
return tempint;
}

// Reads a string from the currently opened file
string readstrfromfile(char endline)
{
string tempstr;
	getline (infile,tempstr,endline);
return tempstr;
}

// Outputs an integer to the stderr logfile
void inttoerr(int integer)
{	
	sprintf(cchar, "%d", integer); fprintf(stderr,cchar);
}

// Outputs an integer to the stdout logfile
void inttoout(int integer)
{
	sprintf(cchar, "%d", integer); fprintf(stdout,cchar);
}

// Checks to see if a file exists (actually, checks if a file's size is > 0)
bool filexist(const char* thefilename)
{
	long begin,end;
	infile.open(thefilename, ios::binary);
	
	begin = infile.tellg();
	infile.seekg (0, ios::end);
	end = infile.tellg();
	infile.seekg (0, ios::beg);
	
	infile.close();
	if ( (end-begin) == 0 ) 
	{
		 return false;
	}
	else
	{
		 return true;
	}
}

// Simple file version check
bool vercheck(const char* filnam, const int reqver, int readver)
{
  if ( readver != reqver ) 
  {
	   error(1,1,readver,reqver,0,0,filnam);
	   if ( readver > 24 && readver < 50 ) { fprintf(stderr, " - You didn't really believe a Xubutor file would work here did you?\n"); }
	   return false;
  }
  else { return true; }
}

// Loads all data files
bool load_files()
{
    fprintf(stdout, "[I] Loading tilesets...\n");
	//Load the tiles
	objimg = load_image( "base/bmp/objs.bmp" );
	worldimg = load_image( "base/bmp/world.bmp" );
	guiimg = load_image( "base/bmp/gui.bmp" );
	beastimg = load_image( "base/bmp/beasts.bmp" );
	
    //If there was an problem loading the object map
    if( objimg == NULL )
    {
	 	error(1,0,0,0,0,0,"base/bmp/objs.bmp");
        return false;    
    }

	//Or loading the world map
	if ( worldimg == NULL )
	{
	 	error(1,0,0,0,0,0,"base/bmp/world.bmp");
		return false;
	}

	//Or the gui map
	if ( guiimg == NULL )
	{
	 	error(1,0,0,0,0,0,"base/bmp/gui.bmp");
		return false;
	}
	
	//Or even the beast map
	if ( beastimg == NULL )
	{
	 	error(1,0,0,0,0,0,"base/bmp/beasts.bmp");
		return false;
	}
	
	fprintf(stdout, "[I] Loading fonts...\n");
	//Load the Fonts
	font1 = TTF_OpenFont( "base/ttf/font1.ttf", fontsz1 );
	font2 = TTF_OpenFont( "base/ttf/font1.ttf", fontsz2 );
	font3 = TTF_OpenFont( "base/ttf/font1.ttf", fontsz3 );
	
	//If there was an error in loading the fonts
	if( font1 == NULL ) 
	{ 
	  	error(1,0,0,0,0,0,"base/ttf/font1.ttf");
		return false; 
	}
	
	//
	//Load the Config file
    //
	if ( filexist("sdlrogue.rcf") == true ) // If the file exists, load it's settings otherwise pick the defaults
	{  
		 infile.open ("sdlrogue.rcf", fstream::in); fprintf(stdout, "[I] Reading 'sdlrogue.rcf'...\n");
		 string tmpstr; int line = 0; getline (infile,tmpstr,','); // First line is to be discarded
	     while ( line < 6 )
		 {
				 if ( line == 0 ) // File Version Checking
				 { 
					  int intvar = readintfromfile(',');
					  if ( vercheck("sdlrogue.rcf",FILEVER,intvar) == false ) { fprintf(stderr, " - Loading defaults...\n"); line = 99; }
				  }
				  if ( line == 1 ) 
				  {
					   int intvar = readintfromfile(',');
					   if ( intvar == 1 ) { newrender = true; } else { newrender = false; }
				  }
				  if ( line == 2 ) { pstep = readintfromfile(','); }
				  if ( line == 3 ) { frmint = readintfromfile(','); }
				  if ( line == 4 ) { FRAMES_PER_SECOND = readintfromfile(','); }
				  if ( line == 5 ) 
				  { 
					   int intvar = readintfromfile(',');
					   if ( intvar == 1 ) { fullscreen = true; lastfull = false; }
				  }
				  line++;
		 }
		 infile.close(); 
	}
	else
	{
		 //If no config file was found, make a new one with the standard settings
		 destfil.open("sdlrogue.rcf",ios::trunc);
		 destfil << "** SDLRogue config file **\n," << FILEVER << ",1,2,8,80,0,";
		 destfil.close();
	}
	
	//
	// Loads the tileset settings from a file (like collidability)
	//
	if ( filexist("base/tileset.rcf") == true ) // If the file exists, load it's settings
	{  
		 infile.open ("base/tileset.rcf", fstream::in); fprintf(stdout, "[I] Reading 'base/tileset.rcf'...\n");
		 string tmpstr; int intvar; int line = 1; getline (infile,tmpstr,','); // First line is to be discarded
		 
		 intvar = readintfromfile(',');
		 if ( vercheck("base/tileset.rcf",FILEVER,intvar) == true ) 
		 {
			  while ( intvar >= 0 )
			  {
					  intvar = readintfromfile(',');
					  if 	 ( intvar == 1 ) { collidable[0][line] = true;  } 
					  else if ( intvar == 0 ) { collidable[0][line] = false; }
					  line++;
				 
					  if ( line > 512 ) { error(2,0,0); infile.close(); return false; }
			  }
			  if ( line == 2 ) { error(1,9,0); }
			  else { fprintf(stdout, " |- Tileset entries read: "); inttoout(line-2); fprintf(stdout, ".\n"); }
			  
			  intvar = 0; line = 1;
			  while ( intvar >= 0 )
			  {
					  intvar = readintfromfile(',');
					  if 	  ( intvar == 1 ) { collidable[1][line] = true;  } 
					  else if ( intvar == 0 ) { collidable[1][line] = false; }
					  line++;
				 
					  if ( line > 512 ) { error(2,1,0); infile.close(); return false; }
			  }
			  if ( line == 2 ) { error(1,9,0); }
			  else { fprintf(stdout, " |- Objectset entries read: "); inttoout(line-2); fprintf(stdout, ".\n"); }
		 }
		 else { fprintf(stderr," - No Collidability has been set!\n"); }
		 
		 infile.close();
	}
	else
	{
		 error(1,0,0,0,0,0,"base/tileset.rcf");
		 fprintf(stderr, " - No Collidability has been set!\n");
	}
	
	///
	/// Loads the monster & attack database
	///
	if ( filexist("base/monsters.rcf") == true ) // If the file exists, load it's settings
	{
		 infile.open ("base/monsters.rcf", fstream::in); fprintf(stdout, "[I] Reading 'base/monsters.rcf'...\n");
		 string tmpstr; bool doneload = false; int line = 0; int intvar = 0;
		 getline (infile,tmpstr,','); // First line is to be discarded
		 
		 intvar = readintfromfile(',');
		 if ( vercheck("base/monsters.rcf",FILEVER,intvar) == true )
		 {  
			  while ( doneload == false ) // This section loads the monsters from the file
			  { 
					  intvar = readintfromfile(',');
					  if ( intvar >= 0 )
					  {
						  ptmonster[line][0][0] = intvar; ptmonster[line][0][1] = readintfromfile(',');
						  ptmonster[line][0][2] = readintfromfile(','); ptmonster[line][0][3] = readintfromfile(',');
						  ptmonster[line][0][4] = readintfromfile(','); ptmonster[line][0][5] = readintfromfile(',');
						  ptmonster[line][0][6] = readintfromfile(','); ptmonster[line][0][7] = readintfromfile(',');
					  
						  ptmonster[line][0][8] = readintfromfile(','); ptmonster[line][0][9] = readintfromfile(',');
						  ptmonster[line][0][10] = readintfromfile(',');
					  
						  int tmpc = 0;
						  intvar = readintfromfile(',');
						  while ( intvar >= 0 ) // Read all the attacks for this monster
						  {
								  if ( tmpc > 9 ) { 
									   error(3,0,line+1);
									   infile.close(); return false; }
								  ptmonster[line][1][tmpc] = intvar;
								  tmpc++;
								  intvar = readintfromfile(',');
						  }
						  tmpc = 0; intvar = readintfromfile(',');
						  while ( intvar >= 0 ) // Read all the normal loot for this monster
						  {
								  if ( tmpc > 9 ) { 
									   error(3,1,line+1);
									   infile.close(); return false; }
								  ptmonster[line][2][tmpc] = intvar;
								  tmpc++;
								  ptmonster[line][2][tmpc] = readintfromfile(',');
								  intvar = readintfromfile(',');
								  tmpc++;
						  }
						  tmpc = 0; intvar = readintfromfile(',');
						  while ( intvar >= 0 ) // Read all the special loot for this monster
						  {
								  if ( tmpc > 9 ) { 
									   error(3,2,line+1);
									   infile.close(); return false; }
								  ptmonster[line][3][tmpc] = intvar;
								  tmpc++;
								  ptmonster[line][3][tmpc] = readintfromfile(',');
								  intvar = readintfromfile(',');
								  tmpc++;
						  }
						  monstername[line] = readstrfromfile(',');
					  }
					  else
					  {
						  doneload = true;
						  if ( line == 0 ) { error(1,9,0); fprintf(stderr, " - No Monsters where added to the database!\n"); }
						  else { fprintf(stdout, " |- Monsters read: "); inttoout(line); fprintf(stdout, ".\n"); }
					  }
					  line++;
				 
					  if ( line >= 64 ) { error(3,3,0); infile.close(); return false; }
			  }
			  doneload = false; line = 0;
			  while ( doneload == false ) // This section loads the AI Attacks from the file
			  {
				      intvar = readintfromfile(',');
					  if ( intvar >= 0 )
					  {
						   aiattack[line][0][0] = intvar;
						   
						   int tmpc = 1; 
						   while ( tmpc < 9 ) { aiattack[line][0][tmpc] = readintfromfile(','); tmpc++; }
						   
						   aiattack[line][1][0] = readintfromfile(','); aiattack[line][1][1] = readintfromfile(',');
						   aiattack[line][2][0] = readintfromfile(','); aiattack[line][2][1] = readintfromfile(',');
						   aiattack[line][2][2] = readintfromfile(','); 
						   
						   aiattackname[line] = readstrfromfile(',');
					  }
					  else
					  {
						   doneload = true;
						   if ( line == 0 ) { error(1,9,0); fprintf(stderr, " - No AI Attacks where added to the database!\n"); }
						   else { fprintf(stdout, " |- AI Attacks read: "); inttoout(line); fprintf(stdout, ".\n"); }
					  }
					  line++;
					  
					  if ( line >= 256 ) { error(3,0,0); infile.close(); return false; }
			  }
		 }
		 else { fprintf(stderr," - No Monsters or AI Attacks where added to the database!\n"); }
		 
		 infile.close();
	}
	else
	{
		 error(1,0,0,0,0,0,"base/monsters.rcf"); fprintf(stderr, "- No Monsters or AI Attacks where added to the database!\n");
	}
    //If eveything loaded fine
	return true;    
}

void assign_tiles(); //Prototype for "assign_tiles" located at the bottom of this source file

// ========================
// Game World Routines
// ========================

// Get the tile # of the next square
int getnexttile( int tehroom, int x, int y, int facing)
{
   if ( facing == 1 ) { return roomset[tehroom][x][y - 1]; }			
   else if ( facing == 2 ) { return roomset[tehroom][x + 1][y]; }
   else if ( facing == 3 ) { return roomset[tehroom][x][y + 1]; }
   else if ( facing == 4 ) { return roomset[tehroom][x - 1][y]; }
}

// Get the object # of the next square
int getnextobj( int tehroom, int x, int y, int facing, int element )
{
   if ( facing == 1 ) { return objset[tehroom][x][y - 1][element]; }			
   else if ( facing == 2 ) { return objset[tehroom][x + 1][y][element]; }
   else if ( facing == 3 ) { return objset[tehroom][x][y + 1][element]; }
   else if ( facing == 4 ) { return objset[tehroom][x - 1][y][element]; }
}

// Changes the properties of the next object
void setnextobj( int tehroom, int x, int y, int facing, int element, int newval )
{
   if ( facing == 1 ) { objset[tehroom][x][y - 1][element] = newval; }			
   else if ( facing == 2 ) { objset[tehroom][x + 1][y][element] = newval; }
   else if ( facing == 3 ) { objset[tehroom][x][y + 1][element] = newval; }
   else if ( facing == 4 ) { objset[tehroom][x - 1][y][element] = newval; }
}

/*==============================================2 LEVEL SPECIFIC 2===================================================*/

bool loadroom( const char* filetoload )
{
	curobjs = 0;
	if ( filexist(filetoload) == true ) // If the file exists, load it's settings
	{  
		 infile.open (filetoload, fstream::in); fprintf(stdout, "[I] Loading level '"); fprintf(stdout, filetoload); fprintf(stdout,"'...\n");
		 string tmpstr; bool donecycle = false; int line = 0; getline (infile,tmpstr,','); // First line is to be discarded
		 int intvar = readintfromfile(',');
		 if ( vercheck(filetoload,FILEVER,intvar) == true )
		 {
			  while ( donecycle == false && line <= 409600 ) // Read the Tiles
			  {
					  int tehroom = readintfromfile(',');
					  if ( tehroom >= 0 )
					  {
						  int mrx = readintfromfile(',');
						  if ( mrx >= 40 ) 
						  {
							  mrx = readintfromfile(',');
							  int mry = readintfromfile(',');
							  roomset[tehroom][mrx][mry] = readintfromfile(',');
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
										  roomset[tehroom][tmpa][tmpb] = intvar;
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
			  if ( donecycle == false ) { error(1,6,0); infile.close(); return false;}
			  
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

bool setbounds(int tehroom) // Set the room's largest X and Y
{
int tmpa = 0; int tmpb = 0; int tmpc = 0; curroom_x = 0; curroom_y = 0;

// Procedure that calculates the tile bounds and sets the encounter tiles
while ( roomset[tehroom][tmpa][tmpb] > 0 ) // While this tile is something
{
	  int doedel = 0;
	  while ( doedel < 8 ) // Do as long as 'doedel' (indicating current area handled) < 8
	  {
			if ( tmpa >= encset[tehroom][doedel][0] ) {
				 if ( tmpa <= encset[tehroom][doedel][2] ) {
					  if ( tmpb >= encset[tehroom][doedel][1] ) {
						   if ( tmpb <= encset[tehroom][doedel][3] ) { roomenc[tmpa][tmpb] = doedel + 1; }
					  }
				 }
		    }
			doedel++;
	  }
 	  tmpa += 1;
 	  tmpc = roomset[tehroom][tmpa][tmpb];
	  if ( tmpc == 0 && tmpa >= curroom_x ) { curroom_x = tmpa - 1; tmpa = 0; tmpb += 1; } 
}

curroom_y = tmpb - 1;

if ( curroom_x < 2 )
{
   error(0,1,tehroom);
   return false;
}
else if ( curroom_y < 2 )
{
   error(0,2,tehroom);
   return false;
}
else { return true; }

}
			  
/*====================================================3 RENDERING 3==================================================*/

// Simple procedure to easily render any variable (int or string) to any Surface
void text2surf(int var, string tmpstr, SDL_Color color, TTF_Font *fontname, int x, int y, SDL_Surface *surfacepntr)
{
 	 if ( var >= 0 ){ 
	   	 sprintf(cchar, "%d", var);
		 const char* tchar = cchar;
		 text = TTF_RenderText_Solid( fontname, tchar, color );
	}
	else {
		 strcpy(cchar, tmpstr.c_str());
		 const char* tchar = cchar;
		 text = TTF_RenderText_Solid( fontname, tchar, color );
	} 
	apply_surface( x, y, text, surfacepntr );
}

// Procedure to automate the drawing of QUI boxes.
int quibox(int boxx, int length, bool separators, int width = 32)
{
	int tmpc = 0;
	int tmpd = (tmpc + 1)*32;
	
	if ( length <= 0 ) 
	{ 
		error(0,0,0); // Out of Range
		fprintf(stdout, "[E] Quibox called with a length of '%i'\n", length);
		return 1; 
	}
	
	apply_surface( boxx, 0, guiimg, screen, &clip[1] );
	apply_surface( boxx+width, 0, guiimg, screen, &clip[7] );
	apply_surface( boxx+(width*2), 0, guiimg, screen, &clip[2] );
	
	if ( length > 2 )
	{
		while ( tmpc < length-2 ) 
		{   						
			tmpd = (tmpc + 1)*32;
		
			apply_surface( boxx, tmpd, guiimg, screen, &clip[3] );
			apply_surface( boxx+width, tmpd, guiimg, screen, &clip[11] );
			apply_surface( boxx+(width*2), tmpd, guiimg, screen, &clip[4] );
		
			if ( separators == true )
			{
				apply_surface( boxx+width, tmpd-16, guiimg, screen, &clip[12] ); // Separator
				apply_surface( boxx+width, tmpd+16, guiimg, screen, &clip[12] ); // Separator
			}
		
			tmpc++;
		}
	}
	tmpd = (tmpc + 1)*32;
	
	if ( length > 1 ) 
	{
		apply_surface( boxx, tmpd, guiimg, screen, &clip[5] );
		apply_surface( boxx+width, tmpd, guiimg, screen, &clip[8] );
		apply_surface( boxx+(width*2), tmpd, guiimg, screen, &clip[6] );
	}
	else
	{
		apply_surface( boxx, 6, guiimg, screen, &clip[5] );
		apply_surface( boxx+width, 6, guiimg, screen, &clip[8] );
		apply_surface( boxx+(width*2), 6, guiimg, screen, &clip[6] );
		
		separators = false;
	}
	
	if ( separators == true ) 
	{ 
		tmpd = (tmpc + 1)*32;
		
		apply_surface( boxx+width, 16, guiimg, screen, &clip[12] ); 
		apply_surface( boxx+width, tmpd-16, guiimg, screen, &clip[12] );
	}
	
	return 0;
	
}

// ===================== KEY QUI
// The mini "select key" menu

int keyqui( int keynum )
{
 	 int quisel = 0;
	 drawqui = true;
	 
	 while ( drawqui == true )
	 {
	 	   	 apply_surface( 228, 212, guiimg, screen, &clip[1]);
	 		 apply_surface( 228, 236, guiimg, screen, &clip[5]);
	 		 apply_surface( 256, 212, guiimg, screen, &clip[7]);
	 		 apply_surface( 256, 236, guiimg, screen, &clip[8]);
	 		 apply_surface( 284, 212, guiimg, screen, &clip[2]);
	 		 apply_surface( 284, 236, guiimg, screen, &clip[6]);
			 
			 apply_surface( 240+(quisel*32), 224, guiimg, screen, &clip[13]);
			 
			 apply_surface( 280, 236, objimg, screen, &miniclip[0]);
			 apply_surface( 248, 236, objimg, screen, &miniclip[keynum]); 
			 
	 		 SDL_Flip(screen);
			 
			 Uint8 *keystate = SDL_GetKeyState(NULL);
			 if ( keystate[SDLK_LEFT] )  { if ( quisel == 1 ) { quisel--; } }
			 else if ( keystate[SDLK_RIGHT] ) { if ( quisel == 0 ) { quisel++; } }
			 else if ( keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL] ) 
			 { 
			   	  if ( ignoreCTRL == false ) { drawqui = false; return quisel; }
			 }
			 else if ( keystate[SDLK_ESCAPE] ) { drawqui = false; ignoreESC = true;	return 42; }
			 else { ignoreCTRL = false; ignoreESC = false; }
			 
			 while( SDL_PollEvent( &event ) )
        	 {
				   //If the user has Xed out the window
            	   if( event.type == SDL_QUIT )
            	   {
                	   //Quit the program
                	   quit = true;
					   drawqui = false;
            	   }		
            }
	 }
}

// ===================== GENERAL QUI
// Used for all out of combat short menu functionality
			  
int quimode( int quix, int nextobj, int objvar, int pvar )
{
 	 // Set up the variables used in this context
	 int tmpa = 0; int tmpb = 0; int tmpc = 0; int tmpd = 0;
	
	 int quilist[30]; // list of QUI menu items
	 int quinum = 0; // Will contain total QUI menu items
	 int quioff = 0; // Will contain the 'offset' used for scrolling through the QUI list
	 int quisel = 0; // Current Item selected
				   
	 // Start adding menu items taking into account whatever the player's looking at
	 if ( nextobj == 0 ) { drawqui = false; }  		  		   	          		  	  // Nothing
	 else if ( nextobj == 1 ) { drawqui = false; } 			   	           		  	  // Teleporter
	 else if ( nextobj == 2 )   		   	 					   	  	   			  // Closed Door
	 {
	 	  if ( objvar == 0 ) { quilist[quinum] = 1; quinum++; }
  		  else if ( objvar > 0 && pvar == 0 ) { quilist[quinum] = 4; quinum++; }
		  else if ( objvar > 0 && pvar > 0 ) { quilist[quinum] = 3; quinum++; }
	 }
	 else if ( nextobj == 3 ) { quilist[quinum] = 2; quinum++; } 		   		  	  // Opened Door
	 else if ( nextobj == 4 ) { quilist[quinum] = 5; quinum++; }					  // A Key
	 else { drawqui = false; }			   	 			  						  	  // None of the above
				   
	 quilist[quinum] = 99; // Add "close menu" to the end of the list
	 quinum++;
				   				   
	 while ( drawqui == true ) // Menu loop
	 {	   
		     quibox(quix,quinum,true);
			 						   
			 tmpa = 0;
						   
			 while ( tmpa < quinum )
			 {
				     bool Altcol = false;
					 if ( quisel == tmpa ) { rrMain = rrWhite; rrAlt = rrLightRed; }
					 else { rrMain = rrBlack; rrAlt = rrDarkRed; }
					 // Draw the Menu items				 
					 if ( quilist[tmpa] == 1 )   	 { text = TTF_RenderText_Solid( font2, "Open Door", rrMain ); }
					 else if ( quilist[tmpa] == 2 )  { text = TTF_RenderText_Solid( font2, "Close Door", rrMain ); }
					 else if ( quilist[tmpa] == 3 )  { text = TTF_RenderText_Solid( font2, "Unlock Door", rrMain ); }
					 else if ( quilist[tmpa] == 4 )  { text = TTF_RenderText_Solid( font2, "Unlock Door", rrAlt ); }
					 else if ( quilist[tmpa] == 5 )  { text = TTF_RenderText_Solid( font2, "Pickup Key", rrMain ); }
					 else if ( quilist[tmpa] == 99 ) { text = TTF_RenderText_Solid( font2, "Close Menu", rrAlt ); }
					 
					 apply_surface( quix+9, (tmpa * 32)+8, text, screen );
					 tmpa++;
			 }
				   
			 // Draw the selector
			 apply_surface( quix, (quisel - quioff)*32, guiimg, screen, &clip[9] ); // Left part
			 apply_surface( quix+64, (quisel - quioff)*32, guiimg, screen, &clip[10] ); // Right part
			 SDL_Flip(screen);
						   
			 // What's the keyboard doing?
			 Uint8 *keystate = SDL_GetKeyState(NULL);
			 if ( keystate[SDLK_UP] ) { 
				  if ( ( quisel - 1 ) >= 0 ) { 
				   	   quisel--;
					   if ( quisel < quioff ) {
							quioff--; }
					   }
				  }
			else if ( keystate[SDLK_DOWN] ) {
				 if ( ( quisel + 1 ) < quinum ) {
					  quisel++;
					  if ( quisel > ( quioff+4 ) ) {
						   quioff++; }
					  }
				 }
			else if ( keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL] )
			{
			 	 if ( ignoreCTRL == false ) 
				 {
				 	  if ( quilist[quisel] == 4 ) { /*Door locked, be we don't have the required key, so do nothing*/ }
				 	  else if ( quilist[quisel] == 99 ) { drawqui = false; ignoreCTRL = true; return 0; }
					  else { drawqui = false; ignoreCTRL = true; return quilist[quisel]; }
				 }
			}
			else if ( keystate[SDLK_ESCAPE] ) { drawqui = false; ignoreESC = true; }
			else
			{
			 	 ignoreESC = false;
				 ignoreCTRL = false;
		    }
				   
			while( SDL_PollEvent( &event ) )
        	{
				   //If the user has Xed out the window
            	   if( event.type == SDL_QUIT )
            	   {
                	   //Quit the program
                	   quit = true;
					   drawqui = false;
            	   }		
            }
  
	 } // End of Menu Loop
}

// ===================== COMBAT QUI

int combqui(int mode, int P1endur, int P1maxendur, int Monendur) // HIERO
{
// SET-UP

	int quilist[30]; // list of QUI menu items
	int quinum = 0; // Will contain total QUI menu items
	int quisel = 0; // Current Item selected
	bool qdraw = true; // Whether or not the QUI should be drawn
	
	int quix = 0; // X Offset for the QUI
	if ( (P1.x*32)-camera.x > 255 ) { quix = 0; } else { quix = 416; } // Offset to the side the player's furthest from

// PROCEDURE

if ( mode == 0 ) // Barebones mode
{
	 // The GUI Frame
	quibox(quix,1,false);
			 
	// Put the stats on the screen
	text2surf(P1endur, "", rrWhite, font3, quix+12, 12, screen); 
	text2surf(-1, "/", rrWhite, font3, quix+44, 12, screen); 
	text2surf(P1maxendur, "", rrBlue, font3, quix+54, 12, screen);
	 
	text2surf(Monendur, "", rrWhite, font2, 128, 128, screen); /* REMOVE ME */
	SDL_Flip(screen);
	
	Uint8 *keystate = SDL_GetKeyState(NULL); // Check for input
	if ( keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL] ) 
	{ 
		if (!ignoreCTRL)
		{
			ignoreCTRL = true; 
			return 1;
		}
	}
	else if ( keystate[SDLK_ESCAPE] ) // Temporary SPECIAL ABORT
	{
		if (!ignoreESC)
		{
			ignoreESC = true;
			return -2;
		}
	}
	else { ignoreCTRL = false; ignoreESC = false; }
	
	return 0;
}
else if ( mode == 2 ) // Action mode
{
	quinum = 3;
	quisel = 0;
	bool ignoreDOWN = false;
	bool ignoreUP = false;
	
	if ( quix > 0 ) { quix += 32; }
	
	while (true)
	{
		int tmpc = 0;
		
		if (qdraw)
		{
			quibox(quix,4,true,16); // Add GUI Frame
	
			// Add all Menu text sequentially
			while ( tmpc <= quinum )
			{
				if ( tmpc == quisel ) { rrMain = rrWhite; } else { rrMain = rrBlack; } // Make it white when selected
				if ( tmpc == 0 ) { text2surf(-1, "Attack", rrMain, font2, quix+11, (tmpc*32)+8, screen); }
				else if ( tmpc == 1 ) { text2surf(-1, "Magic", rrMain, font2, quix+14, (tmpc*32)+8, screen); }
				else if ( tmpc == 2 ) { text2surf(-1, "Item", rrMain, font2, quix+17, (tmpc*32)+8, screen); }
				else if ( tmpc == 3 ) { text2surf(-1, "Move", rrMain, font2, quix+15, (tmpc*32)+8, screen); }
				tmpc++;
			}
			// Selector
			apply_surface( quix, (quisel*32), guiimg, screen, &clip[9] ); // Left part
			apply_surface( quix+32, (quisel*32), guiimg, screen, &clip[10] ); // Right part
	
			SDL_Flip(screen);
			qdraw = false;
		}
		
		// Check and parse input
		Uint8 *keystate = SDL_GetKeyState(NULL);
		if ( keystate[SDLK_UP] ) 
		{ 
			if (!ignoreUP)
			{
				quisel--;
				if ( quisel < 0 ) { quisel = 3; }
				ignoreUP = true;
				qdraw = true;
			}
		}
		else if ( keystate[SDLK_DOWN] ) 
		{
			if (!ignoreDOWN)
			{
				quisel++;
				if ( quisel > 3 ) { quisel = 0; }
				ignoreDOWN = true;
				qdraw = true;
			}
		}
		else if ( keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL] )
		{
			if (!ignoreCTRL) 
			{
				if ( quisel == 0 ) // Attack Menu
				{
					ignoreCTRL = true;
					return 1;
				}
				else if ( quisel == 1 ) // Magic Menu
				{
					ignoreCTRL = true;
					return 2;
				}
				else if ( quisel == 2 ) // Item Menu
				{
					ignoreCTRL = true;
					return 3;
				}
				else if ( quisel == 3 ) // Move Menu
				{
					ignoreCTRL = true;
					return 4;
				}
			}
		}
		else if ( keystate[SDLK_ESCAPE] ) 
		{ 
			if (!ignoreESC)
			{
				ignoreESC = true; 
				return 0; 
			}
		}
		else
		{
			ignoreESC = false;
			ignoreCTRL = false;
			ignoreDOWN = false;
			ignoreUP = false;
		}
		
		// Poll for events in case the user pressed the 'X' or something
		while ( SDL_PollEvent( &event ) )
		{
			//If the user has Xed out the window
			if ( event.type == SDL_QUIT )
			{
				//Quit the cycle
				return -1;
			}		
		}
	}
}
else if ( mode == 3 ) // Attack mode
{
	quinum = 3;
	quisel = 0;
	bool ignoreDOWN = false;
	bool ignoreUP = false;
	
	if ( quix > 0 ) { quix += 32; }
	
	// Draw the original 'fake' menu
	quibox(quix,4,true,16);
	text2surf(-1, "Attack", rrCyan, font2, quix+11, 8, screen); 
	text2surf(-1, "Magic", rrBlack, font2, quix+14, 40, screen); 
	text2surf(-1, "Item", rrBlack, font2, quix+17, 72, screen); 
	text2surf(-1, "Move", rrBlack, font2, quix+15, 104, screen); 
	
	if ( quix > 0 ) { quix = 352; } else { quix = 64; }
	
	while (true)
	{	
		int tmpc = 0;
		
		if (qdraw)
		{
			quibox(quix,2,true);
		
			if (true)
			{
				if ( quix > 64 ) { tmpc = quix+50; } else { tmpc = 0; }
				SDL_Rect UIRECT = { tmpc, 380, 110, 100 }; // Cyan Background
				SDL_FillRect( screen, &UIRECT, SDL_MapRGB( screen->format, 0x00, 0xFF, 0xFF ) );
				SDL_Rect UIRECT2 = { tmpc+1, 381, 108, 16}; // Dark Titlebar
				SDL_FillRect( screen, &UIRECT2, SDL_MapRGB( screen->format, 0x42, 0x80, 0xFF ) );
				SDL_Rect UIRECT3 = { tmpc+1, 398, 108, 81}; // Mid content section
				SDL_FillRect( screen, &UIRECT3, SDL_MapRGB( screen->format, 0x64, 0xA2, 0xFF ) );
			}
			
			text2surf(-1,"Attack Name", rrBlack, font2, tmpc+2, 382, screen); text2surf(-1,"Type:", rrWhite, font1, tmpc+2, 400, screen);
			text2surf(-1,"Skill:", rrWhite, font1, tmpc+2, 416, screen); text2surf(-1,"Endurance:", rrWhite, font1, tmpc+2, 432, screen);
			text2surf(-1,"Mana:", rrWhite, font1, tmpc+2, 448, screen); text2surf(-1,"Movement:", rrWhite, font1, tmpc+2, 464, screen);
			
			tmpc = 0;
		
			while ( tmpc <= quinum )
			{
				if ( tmpc == quisel ) { rrMain = rrWhite; } else { rrMain = rrBlack; } // Make it white when selected
				if ( tmpc == 0 ) { text2surf(-1, "Blade Cutter", rrMain, font1, quix+8, (tmpc*32)+8, screen); }
				else if ( tmpc == 1 ) { text2surf(-1, "Circle Strafe", rrMain, font1, quix+8, (tmpc*32)+8, screen); }

				tmpc++;
			}
			// Selector
			apply_surface( quix, (quisel*32), guiimg, screen, &clip[9] ); // Left part
			apply_surface( quix+64, (quisel*32), guiimg, screen, &clip[10] ); // Right part
		
			SDL_Flip(screen);
			qdraw = false;
		}
		
		// Check and parse input
		Uint8 *keystate = SDL_GetKeyState(NULL);
		if ( keystate[SDLK_UP] ) 
		{ 
			if (!ignoreUP)
			{
				quisel--;
				if ( quisel < 0 ) { quisel = 3; }
				ignoreUP = true;
				qdraw = true;
			}
		}
		else if ( keystate[SDLK_DOWN] ) 
		{
			if (!ignoreDOWN)
			{
				quisel++;
				if ( quisel > 3 ) { quisel = 0; }
				ignoreDOWN = true;
				qdraw = true;
			}
		}
		else if ( keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL] )
		{
			if (!ignoreCTRL) 
			{
				return quisel+1;
			}
		}
		else if ( keystate[SDLK_ESCAPE] ) 
		{ 
			if (!ignoreESC)
			{
				ignoreESC = true; 
				return 0; 
			}
		}
		else
		{
			ignoreESC = false;
			ignoreCTRL = false;
			ignoreDOWN = false;
			ignoreUP = false;
		}
		
		// Poll for events in case the user pressed the 'X' or something
		while ( SDL_PollEvent( &event ) )
		{
			//If the user has Xed out the window
			if ( event.type == SDL_QUIT )
			{
				//Quit the cycle
				return -1;
			}		
		}
	}
}
else if ( mode == 4 ) // Magic mode
{
}
else if ( mode == 5 ) // Item mode
{
}
else if ( mode == 6 ) // Move mode
{
}
else { return 0; }

}

// ===================== SIDEBAR GUI

void hudhdevide(int a, int b)
{
	apply_surface( 0, a, guiimg, hudbuffer, &clip[b]);
	apply_surface( 32, a, guiimg, hudbuffer, &clip[b]);
	apply_surface( 64, a, guiimg, hudbuffer, &clip[b]);
	apply_surface( 96, a, guiimg, hudbuffer, &clip[b]);
}

void sidebar( int element, int var1, int var2, int var3, int var4, int var5, int var6, int var7, int var8, string var9 )
{
	 if ( element == 0 ) {
		  int tmpc = 1;
		  SDL_Rect CLEANHUDRECT = { 7, 55, 114, 51 };
		  SDL_FillRect( hudbuffer, &CLEANHUDRECT, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) ); // Clear the region that'll be redrawn

		  //Top Row
		  apply_surface( 0, 0, guiimg, hudbuffer, &clip[26] );
	      apply_surface( 32, 0, guiimg, hudbuffer, &clip[27] );
	      apply_surface( 64, 0, guiimg, hudbuffer, &clip[27] );
		  apply_surface( 96, 0, guiimg, hudbuffer, &clip[28] );
		  text2surf( -1, var9, rrYellow, font2, 10, 8, hudbuffer ); //Name
		  
		  //HP
		  text2surf( -1, "Health:", rrGreen, font2, 8, 54, hudbuffer );
		  //Bar Display
		  SDL_Rect TEMPRECTA = { 8, 67, 112, 12};
		  SDL_FillRect( hudbuffer, &TEMPRECTA, SDL_MapRGB( screen->format, 0x88, 0x88, 0x88 ) );
		  SDL_Rect TEMPRECTB = { 9, 68, 110, 10 };
		  SDL_FillRect( hudbuffer, &TEMPRECTB, SDL_MapRGB( screen->format, 0x77, 0x00, 0x00 ) );
		  
		  float tmpf = ( float (var1) / float (var2) )* 110;
		  
		  tmpc = static_cast<int>(tmpf);
		  SDL_Rect TEMPRECTC = { 9, 68, tmpc, 10 };
 		  SDL_FillRect( hudbuffer, &TEMPRECTC, SDL_MapRGB( screen->format, 0x00, 0xFF, 0x88 ) );
		  
  		  tmpc = 28; int tmpd = 58; int tmpe = 66;
		  if ( var2 > 999 && var2 <= 999999 ) { tmpc = 12; tmpd = 58; tmpe = 66; }
		  else if ( var2 > 999999 ) { tmpc = 128; tmpd = 128; tmpe = 128; }
		  
		  //Numeric Display
		  text2surf( var1, "", rrDarkGreen, font2, tmpc, 66, hudbuffer );
		  text = TTF_RenderText_Solid( font2,"/",rrDarkGreen);
		  apply_surface( tmpd, 66, text, hudbuffer );
		  text2surf( var2, "", rrDarkGreen, font2, tmpe, 66, hudbuffer );
		  
		  //MP
		  text2surf( -1, "Mana:", rrLightBlue, font2, 8, 78, hudbuffer );
		  //Bar Display
		  SDL_Rect TEMPRECTD = { 8, 91, 112, 12};
		  SDL_FillRect( hudbuffer, &TEMPRECTD, SDL_MapRGB( screen->format, 0x88, 0x88, 0x88 ) );
		  SDL_Rect TEMPRECTE = { 9, 92, 110, 10 };
		  SDL_FillRect( hudbuffer, &TEMPRECTE, SDL_MapRGB( screen->format, 0x77, 0x00, 0x00 ) );
		  
		  tmpf = ( float (var3) / float (var4) )* 110;
		  
		  tmpc = static_cast<int>(tmpf);
		  SDL_Rect TEMPRECTF = { 9, 92, tmpc, 10 };
 		  SDL_FillRect( hudbuffer, &TEMPRECTF, SDL_MapRGB( screen->format, 0x22, 0x66, 0xFF ) );
		  
		  tmpc = 28; tmpd = 58; tmpe = 66;
		  if ( var4 > 999 && var2 <= 999999 ) { tmpc = 12; tmpd = 58; tmpe = 66; }
		  else if ( var4 > 999999 ) { tmpc = 128; tmpd = 128; tmpe = 128; }
		  
		  //Numeric Display
		  text2surf( var3, "", rrDarkBlue, font2, tmpc, 90, hudbuffer );
  		  text = TTF_RenderText_Solid( font2,"/",rrDarkBlue);
		  apply_surface( tmpd, 90, text, screen );
		  text2surf( var4, "", rrDarkBlue, font2, tmpe, 90, hudbuffer );
	 }
	 else if ( element == 1 ) {
	 	  SDL_Rect CLEANHUDRECT = { 7, 30, 114, 23 };
		  SDL_FillRect( hudbuffer, &CLEANHUDRECT, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) ); // Clear the region that'll be redrawn
		  
		  //Major Level
		  text2surf( -1, "Lvl:", rrWhite, font2, 8, 30, hudbuffer);
		  text2surf( var1, "", rrWhite, font2, 30, 30, hudbuffer);
		  //Minor Level
		  if ( var2 == 0 ) { text2surf( -1, "Ap", rrBrown, font2, 100, 29, hudbuffer ); }
		  else if ( var2 == 1 ) { text2surf( -1, "Sk", rrGrey, font2, 100, 29, hudbuffer ); }
		  else if ( var2 == 2 ) { text2surf( -1, "Mr", rrGreen, font2, 100, 29, hudbuffer ); }
		  
		  //Experience bar
		  SDL_Rect TEMPRECTA = { 8, 45, 110, 6 };
		  SDL_FillRect( hudbuffer, &TEMPRECTA, SDL_MapRGB( screen->format, 0x88, 0x88, 0x88 ) );
		  SDL_Rect TEMPRECTB = { 9, 46, 108, 4 };
		  SDL_FillRect( hudbuffer, &TEMPRECTB, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) );
		  
		  float tmpe = ( float (var3) / float (var4) )* 110;
		  
		  int tmpc = static_cast<int>(tmpe);
		  SDL_Rect TEMPRECTC = { 9, 46, tmpc, 4 };
		  SDL_FillRect( hudbuffer, &TEMPRECTC, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0x00 ) );
		 
	}
	else if ( element == 2 ) { // Stats
		 SDL_Rect CLEANHUDRECT = { 7, 108, 114, 51 };
		 SDL_FillRect( hudbuffer, &CLEANHUDRECT, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) ); // Clear the region that'll be redrawn
	
		 text2surf(-1,"STR",rrWhite, font1, 8, 108, hudbuffer); text2surf(var1,"",rrWhite, font1, 36, 108, hudbuffer);
		 text2surf(-1,"INT",rrGrey, font1, 68, 108, hudbuffer); text2surf(var2,"",rrGrey, font1, 96, 108, hudbuffer);
		 text2surf(-1,"DEX",rrGrey, font1, 8, 126, hudbuffer); text2surf(var3,"",rrGrey, font1, 36, 126, hudbuffer);
		 text2surf(-1,"CON",rrWhite, font1, 68, 126, hudbuffer); text2surf(var4,"",rrWhite, font1, 96, 126, hudbuffer);
		 text2surf(-1,"CHA",rrWhite, font1, 8, 144, hudbuffer); text2surf(var5,"",rrWhite, font1, 36, 144, hudbuffer);
		 text2surf(-1,"LUC",rrGrey, font1, 68, 144, hudbuffer); text2surf(var6,"",rrGrey, font1, 96, 144, hudbuffer);
	}
	
	else if ( element == 3 ) { // Weapon/Spell Skills
		 SDL_Rect CLEANHUDRECT = { 7, 161, 114, 100 };
		 SDL_FillRect( hudbuffer, &CLEANHUDRECT, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) ); // Clear the region that'll be redrawn
		 
		 apply_surface( 10, 164, guiimg, hudbuffer, &miniclip[0] ); text2surf(var1,"",rrRed, font2, 36, 161, hudbuffer);
		 apply_surface( 10, 176, guiimg, hudbuffer, &miniclip[1] ); text2surf(var2,"",rrLightBlue, font2, 36, 173, hudbuffer);
		 apply_surface( 10, 188, guiimg, hudbuffer, &miniclip[2] ); text2surf(var3,"",rrGreen, font2, 36, 185, hudbuffer);
		 apply_surface( 10, 200, guiimg, hudbuffer, &miniclip[3] ); text2surf(var4,"",rrWhite, font2, 36, 197, hudbuffer);
		
		 apply_surface( 10, 214, guiimg, hudbuffer, &miniclip[4] ); text2surf(var5,"",rrGrey, font2, 36, 211, hudbuffer);
		 apply_surface( 10, 226, guiimg, hudbuffer, &miniclip[5] ); text2surf(var6,"",rrBrown, font2, 36, 223, hudbuffer);
		 apply_surface( 10, 238, guiimg, hudbuffer, &miniclip[6] ); text2surf(var7,"",rrWhite, font2, 36, 235, hudbuffer);
		 apply_surface( 10, 250, guiimg, hudbuffer, &miniclip[7] ); text2surf(var8,"",rrYellow, font2, 36, 247, hudbuffer);
	}
	
	else if ( element == 4 ) { // Keys
		 SDL_Rect CLEANHUDRECT = { 7, 263, 114, 45 };
		 SDL_FillRect( hudbuffer, &CLEANHUDRECT, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) ); // Clear the region that'll be redrawn
	
		 apply_surface( 8, 264, objimg, hudbuffer, &miniclip[1] ); text2surf(var2,"",rrWhite,font1,28,261, hudbuffer);
		 apply_surface( 44, 264, objimg, hudbuffer, &miniclip[2] ); text2surf(var3,"",rrWhite,font1,64,261, hudbuffer);
		 apply_surface( 82, 264, objimg, hudbuffer, &miniclip[3] ); text2surf(var4,"",rrWhite,font1,102,261, hudbuffer);
		 
		 apply_surface( 8, 280, objimg, hudbuffer, &miniclip[4] ); text2surf(var5,"",rrLightBlue,font1,28,277, hudbuffer);
		 apply_surface( 44, 280, objimg, hudbuffer, &miniclip[5] ); text2surf(var6,"",rrGreen,font1,64,277, hudbuffer); 
		 apply_surface( 8, 296, objimg, hudbuffer, &miniclip[6] ); text2surf(var7,"",rrYellow,font1,28,293, hudbuffer);
		 apply_surface( 44, 296, objimg, hudbuffer, &miniclip[7] ); text2surf(var8,"",rrRed,font1,64,293, hudbuffer);
		 
		 apply_surface( 82, 288, objimg, hudbuffer, &miniclip[0] ); text2surf(var1,"",rrWhite,font1,102,285, hudbuffer);
	}
	
	else if ( element == 5 ) {
		 apply_surface( 8, 454, guiimg, hudbuffer, &clip[29] ); text2surf(var1,"",rrLightGrey,font2,28,458, hudbuffer);
	}
	
	int tmpc = 1;
	//Draw the Sidebar borders
	  
	hudhdevide(53,14); //Second Separator (Between Level and Health/mana)
	hudhdevide(106,14); //Third Separator (Between Health/Mana and basestats)
		  
	//Fourth Separator(s) (Basestats grid, and underlining)
	hudhdevide(123,14);
	hudhdevide(141,14);
	hudhdevide(159,14);  
	apply_surface( 32, 106, guiimg, hudbuffer, &clip[15] );
	apply_surface( 32, 127, guiimg, hudbuffer, &clip[15] );
	apply_surface( 64, 106, guiimg, hudbuffer, &clip[15] );
	apply_surface( 64, 127, guiimg, hudbuffer, &clip[15] );
	apply_surface( 92, 106, guiimg, hudbuffer, &clip[15] );
	apply_surface( 92, 127, guiimg, hudbuffer, &clip[15] );
		  
	hudhdevide(210,14); //Fifth Separator (Between Magic Skills and Weapon Skills)
	hudhdevide(261,14); //Sixth Separator (Between Weapon Skills and Keys)
	  
	//Vertical Separator (For the Fifth and the Sixth)
	apply_surface( 32, 159, guiimg, hudbuffer, &clip[15] );
	apply_surface( 32, 191, guiimg, hudbuffer, &clip[15] );
	apply_surface( 32, 220, guiimg, hudbuffer, &clip[15] );
	apply_surface( 32, 230, guiimg, hudbuffer, &clip[15] );
	  
	hudhdevide(274,14); //Seventh Seperator (Between Keys and Keycards)
	hudhdevide(308,14); //Eighth Seperator (Between Keycards and Inventory)
	  
	apply_surface( 78, 276, guiimg, hudbuffer, &clip[15] ); //Semi-Seperator between keycards and MasterKey
	
	hudhdevide(454,14); //Ninth Seperator (Between Inventory and Cash)
		  
	//The sides
	while ( tmpc < 14 ) 
	{
	  	   	apply_surface( 0, tmpc*32, guiimg, hudbuffer, &clip[21] );
	 	   	apply_surface( 96, tmpc*32, guiimg, hudbuffer, &clip[22] );
	 	   	tmpc++; 
	}
		  
	//Bottom row
	apply_surface( 0, 448, guiimg, hudbuffer, &clip[19] );
	apply_surface( 32, 448, guiimg, hudbuffer, &clip[23] );
	apply_surface( 64, 448, guiimg, hudbuffer, &clip[23] );
	apply_surface( 96, 448, guiimg, hudbuffer, &clip[20] );

	draws = true;
}

//  =============
//  RENDERER
//  =============

// Que Renderer, Places all the tiles inside the camera view into an array and then blits all the items in the que to the screen directly
void querender(bool inbattle)
{
int tmpa = 0; int tmpb = 0; int tmpc = 0; int tmpd = 0; int tmpe = 0;
int rendque[1024][3][4]; int curnorque = 0; int curobjque = 0; int curforque = 0;

// Some temporary variables to make the following read more easily
int thisx; int lowestx; int highestx; int truex;
int thisy; int lowesty; int highesty; int truey;

SDL_FillRect( screenbuffer1, &screen->clip_rect, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) ); // Blackened!!
SDL_FillRect( screen, &camerasize, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) );

while ( roomset[P1.roomnum][tmpa][tmpb] > 0 ) // Draw the world tiles
{
	 thisx = tmpa*32; truex = (tmpa*32) - camera.x; highestx = camera.x + SCREEN_WIDTH; lowestx = camera.x - 32;
	 thisy = tmpb*32; truey = (tmpb*32) - camera.y; highesty = camera.y + SCREEN_HEIGHT; lowesty = camera.y - 32;
	 
	 // Add the current tile to the render que, if it's inside the camera view
	 if ( thisx >= lowestx && thisx <= highestx ) 
	 { 
	   	  if ( thisy >= lowesty && thisy <= highesty ) 
		  { 
		   	  	// The normal tile
				rendque[curnorque][0][0] = truex; 
				rendque[curnorque][0][1] = truey; 
				rendque[curnorque][0][2] = roomset[P1.roomnum][tmpa][tmpb];
				curnorque++;
				
 
	 			// If object type isn't 0 (i.e. it's SOMETHING) add it to the que
				if ( objset[P1.roomnum][tmpa][tmpb][0] != 0 )
	 			{
					 tmpc = objset[P1.roomnum][tmpa][tmpb][1];
					 rendque[curobjque][1][0] = truex; rendque[curobjque][1][1] = truey; // Set the X and Y for this entry in the que
					 rendque[curobjque][1][2] = 0; rendque[curobjque][1][3] = 0;
		  
	  	  			 if ( objset[P1.roomnum][tmpa][tmpb][0] == 1 ) // Teleporter
		  			 {
		   	   		  	  tmpd = object[tmpc][0]; 
			   			  if ( tmpd > 0 ) { rendque[curobjque][1][2] = 12 + tmpd; }
		  			 }
		  			 else if ( objset[P1.roomnum][tmpa][tmpb][0] == 2 ) // Closed Door
		  			 {
			   		  	  rendque[curobjque][1][2] = 17 + object[tmpc][1] + (object[tmpc][0]*4);
		  			 }
		  			 else if ( objset[P1.roomnum][tmpa][tmpb][0] == 3 ) // Opened Door
		  			 {
			   		  	  rendque[curobjque][1][2] = 19 + object[tmpc][1] + (object[tmpc][0]*4);
		  			 }
		  			 else if ( objset[P1.roomnum][tmpa][tmpb][0] == 4 ) // Key
		  			 {
			  		  	  rendque[curobjque][1][0] = truex+8; // Offset the X because they key isn't 32^2
						  rendque[curobjque][1][1] = truey+16; // Offset the Y also
						  rendque[curobjque][1][2] = tmpc;
						  rendque[curobjque][1][3] = 1; // Also set the miniclip 'true' var
		  			 }
					 else if ( objset[P1.roomnum][tmpa][tmpb][0] == 5 ) // Monster
					 {
						  int tmpq = object[tmpc][0];
						  rendque[curobjque][1][2] = ( ( ptmonster[tmpq][0][0] + object[tmpc][4] ) * object[tmpc][5] ) + 1;
						  rendque[curobjque][1][3] = 2; // Use Alternative Tileset
					 }
					 curobjque++;
	 			}
				
				
				
	 			// Get the Foreground Tile if there's something there
	 			if ( fgset[P1.roomnum][tmpa][tmpb] > 0 )
				{
				   	 rendque[curforque][2][0] = truex; 
					 rendque[curforque][2][1] = truey;
	 				 rendque[curforque][2][2] = fgset[P1.roomnum][tmpa][tmpb];
					 curforque++;
				}
	 	  }
	 }
	 tmpa += 1;
	 if ( roomset[P1.roomnum][tmpa][tmpb] == 0) { tmpa = 0; tmpb += 1; }
}

if ( inbattle == true )
{
   	 int tmpa = 0;
	 while ( tmpa < lastbeast )
	 {
	  	   	 int tmpc = battlebeasts[tmpa][2]; int tmpq = object[tmpc][0];
			 rendque[curobjque][1][0] = ( battlebeasts[tmpa][0]*32 ) - camera.x;
			 rendque[curobjque][1][1] = ( battlebeasts[tmpa][1]*32 ) - camera.y;
			 rendque[curobjque][1][2] = ( ( ptmonster[tmpq][0][0] + object[tmpc][4] ) * object[tmpc][5] ) + 1;
			 rendque[curobjque][1][3] = 2;
			 curobjque++;
			 tmpa++;
	 }
}   

// ---------------------------------------
// Start rendering items from the que here
// ---------------------------------------

bool doneque = false;
tmpa = 0;
while ( doneque == false ) // Draw the Base tiles
{	 
	 tmpb = rendque[tmpa][0][0]; tmpc = rendque[tmpa][0][1];
	 tmpd = rendque[tmpa][0][2]; 
	 apply_surface( tmpb, tmpc, worldimg, screen, &clip[tmpd] );
	 tmpa++;
	 
	 if ( tmpa == curnorque ) { doneque = true; }
} 		  

doneque = false;
tmpa = 0;
if ( curobjque != 0 ) // Are there any Object tiles at all?
{
   	 while ( doneque == false ) // Draw the Object tiles
	 {
          tmpb = rendque[tmpa][1][0]; tmpc = rendque[tmpa][1][1];
	 	  tmpd = rendque[tmpa][1][2];
	 	  
		  // Is this a standard object or does it follow special rules (Like miniclip or alt. tilesets)?
		  if ( rendque[tmpa][1][3] == 0 ) { apply_surface( tmpb, tmpc, objimg, screen, &clip[tmpd] ); }
	 	  else if ( rendque[tmpa][1][3] == 1 ) { apply_surface( tmpb, tmpc, objimg, screen, &miniclip[tmpd] ); }
		  else if ( rendque[tmpa][1][3] == 2 ) { apply_surface( tmpb, tmpc, beastimg, screen, &clip[tmpd] ); }
	 	  tmpa++;
	 
	 	  if ( tmpa == curobjque ) { doneque = true; }
	 } 		  
}
// Put the player on the screen here
if ( P1.moving == false ) 
{ 
   	 int player1x = (P1.x*32) - camera.x; int player1y = (P1.y*32) - camera.y;
   	 apply_surface( player1x, player1y, objimg, screen, &clip[P1.facing + 4] );
}
else
{
	// When the player IS moving, add # pixels to offset him in the direction he's moving to have him move smoothly
	int player1x = (P1.x*32) - camera.x; int player1y = (P1.y*32) - camera.y;
	tmpc = P1.facing + (P1.frame * 4);
	if ( P1.facing == 1 ) { apply_surface( player1x, player1y - P1.movstep, objimg, screen, &clip[tmpc] ); }
	if ( P1.facing == 2 ) { apply_surface( player1x + P1.movstep, player1y, objimg, screen, &clip[tmpc] ); }
	if ( P1.facing == 3 ) { apply_surface( player1x, player1y + P1.movstep, objimg, screen, &clip[tmpc] ); }
	if ( P1.facing == 4 ) { apply_surface( player1x - P1.movstep, player1y, objimg, screen, &clip[tmpc] ); }
}

doneque = false;
tmpa = 0;
if ( curforque != 0 ) // Are there any Foreground tiles at all?
{ 
     while ( doneque == false ) // Draw the Foreground tiles
	 {
          tmpb = rendque[tmpa][2][0]; tmpc = rendque[tmpa][2][1];
	 	  tmpd = rendque[tmpa][2][2]; 
	 	  apply_surface( tmpb, tmpc, worldimg, screen, &clip[tmpd] );
	 	  tmpa++;
	 
	 	  if ( tmpa == curforque ) { doneque = true; }
	 } 
}


if ( inbattle == true ) // Draw the combat borders on the screen
{
   	 tmpa = 0; tmpb = 0;
   	 while ( tmpb < maxroomy ) 
	 {
		  while ( tmpa < maxroomx ) 
		  {
			   bool inscreen = false;
			   // Check to see if the tile is on the screen and NOT part of the combat area
			   int truex = (tmpa*32) - camera.x;
			   int truey = (tmpb*32) - camera.y;
			   if ( truex > -32 && truex < SCREEN_WIDTH && truey > -32 && truey < SCREEN_HEIGHT ) { inscreen = true; }
			   if ( battlezone[tmpa][tmpb] == false && inscreen == true ) 
			   { 
					apply_surface( truex, truey, guiimg, screen, &clip[30] ); // If so, draw the 'not combat area' tile here.
			   }
			   tmpa++;
		  }
		  tmpa = 0;
		  tmpb++;
	 }
}

apply_surface(512, 0, hudbuffer, screen);
if ( inbattle == false ) { SDL_Flip(screen); } // Flip the backbuffer to the screen 
draws = false;

}

/*==============================================4 MAIN ROUTINES 4====================================================*/

// ================
// COMBAT ROUTINE
// ================

int combat(float P1STR, float P1INT, float P1DEX, float P1CON, float P1CHA, float P1LUC, float P1ENDDel, float P1ENDGain)
{
int tmpb = 0; int tmpc = 0; int tmpd = 0; int tmpe = 0; int P1CounterDelay = 0; int MonEndDelay = 0; // Temporary variables

int P1_STR = static_cast<int>(P1STR); int P1_INT = static_cast<int>(P1INT); int P1_DEX = static_cast<int>(P1DEX); // Cast STR, INT, DEX...
int P1_CON = static_cast<int>(P1CON); int P1_CHA = static_cast<int>(P1CHA); int P1_LUC = static_cast<int>(P1LUC); // ...CON, CHA and LUC to ints.
int P1EndDelay = static_cast<int>(P1.stat[6]); int P1EndGain = static_cast<int>(P1.stat[7]); // Cast the Floats representing Endurance gain and -Delay to ints

int P1Endurance = 0; int P1MaxEndurance = roundn((P1_STR*P1_DEX*P1_CON)*P1EndGain/(P1EndDelay/7))/P1.level[0]; // Calculate the Max amount of Endurance

Timer cfps;

tmpd = P1.x - roundn(maxarenasize/2); 
if ( tmpd < 0 ) { tmpd = 0; } if ( tmpd >= maxroomx - maxarenasize ) { tmpd = maxroomx - maxarenasize + 1; } // Set starting X for arena
tmpe = P1.y - roundn(maxarenasize/2);
if ( tmpe < 0 ) { tmpe = 0; } if ( tmpe >= maxroomy - maxarenasize ) { tmpe = maxroomy - maxarenasize + 1; } // Set starting Y for arena

// Mark the arena, gather all monsters in the battle area and put them into the 'battlebeasts' array
while ( tmpc < maxarenasize ) {
	while ( tmpb < maxarenasize ) {
		battlezone[tmpd+tmpb][tmpe+tmpc] = true;
		if ( objset[P1.roomnum][tmpd+tmpb][tmpe+tmpc][0] == 5 ) {
			battlebeasts[lastbeast][0] = tmpd+tmpb;
			battlebeasts[lastbeast][1] = tmpe+tmpc;
			battlebeasts[lastbeast][2] = objset[P1.roomnum][tmpd+tmpb][tmpe+tmpc][1];
			objset[P1.roomnum][tmpd+tmpb][tmpe+tmpc][0] = 0; // As battlebeasts takes care of the monster now, remove it from the room
			lastbeast++; 
		}
		tmpb++;
	}
	tmpc++;
	tmpb = 0;
}

bool incombat = true; // Are we in combat? We sure are!
int battlephase = 0; // Which battle phase are we in?
int actionmode = 0; // Which part of the action mode are we in?
cfps.start();

// ---------- COMBAT LOOP --------
while ( incombat == true ) 
{	
	// Reset some of the 'session' variables
	tmpb = 0;
	int cqout = 0;
	
	querender(true); // Render the scene
	 
	if ( actionmode == 0 ) // When no action has been chosen yet
	{	  
		cqout = combqui(battlephase,P1Endurance,P1MaxEndurance,object[battlebeasts[0][2]][14]); // Render the Combat QUI
		
		if ( cqout == -1 ) { fprintf(stderr, "HARD KILL by COMBQUI\n"); return 0; }
		
		if ( battlephase == 0 )
		{
			if ( cqout == 1 ) { battlephase = 2; /* Change to 1 when COUNT UP is written */ }
			else if ( cqout == -2 ) { fprintf(stderr, "COMBAT ABORT\n"); incombat = false; }
		}
		else if ( battlephase == 1 )
		{
		}
		else if ( battlephase == 2 )
		{
			if ( cqout == 0 ) { battlephase = 0; } // Go back to initial state
			else if ( cqout == 1 ) { battlephase = 3; } // Go to the Attack menu
		}
		else if ( battlephase == 3 )
		{
			if (cqout == 0 ) { battlephase = 2; } // Go back to the Action Menu
			else if ( cqout > 0 ) { actionmode = 1; battlephase = 0; } // Make it happen!
		}
		
		/* AI Decision goes here */
	}
	 
	// ACTION Processing
	else if ( actionmode == 1 ) // The Player takes action
	{
		fprintf(stderr, "The Player did a basic attack");
		actionmode = 0;
	}
	else if ( actionmode == 2 ) // The Monster takes action
	{
		fprintf(stderr, "The Monster did a basic attack");
		actionmode = 0;
	}
	 
	// ENDURANCE Processing
	if ( actionmode == 0 && battlephase == 0 )
	{
		// Increase the Player's Endurance at set intervals
		P1CounterDelay++;
		if ( P1CounterDelay == P1EndDelay && P1Endurance < P1MaxEndurance ) { P1Endurance += P1EndGain; P1CounterDelay = 0; }
		if ( P1CounterDelay == P1EndDelay && P1Endurance >= P1MaxEndurance ) { P1.addhealth(P1_CON); P1CounterDelay = 0; }
		// Increase each monster's endurance at set intervals
		while ( tmpb < lastbeast )
		{
			tmpc = battlebeasts[tmpb][2];
			object[tmpc][15]++;
			if ( object[tmpc][15] >= ptmonster[object[tmpc][0]][0][7] ) { object[tmpc][14] += ptmonster[object[tmpc][0]][0][8]; object[tmpc][15] = 0; }
			tmpb++;
		}
	}
	// Cap the framerate
	if ( cfps.get_ticks() < 1000 / FRAMES_PER_SECOND )
	{
		SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - cfps.get_ticks() );
	}

	// TO BE WRITTEN: AI Decision
	 
	// And finally, poll for events in case the user pressed the 'X' or something
	while ( SDL_PollEvent( &event ) )
	{
		//If the user has Xed out the window
		if ( event.type == SDL_QUIT )
		{
			//Quit the cycle
			return 0;
		}		
	}
}

// End of Combat
// Reset the Battlezone array, so it'll be clear the next time it's used
tmpc = 0; tmpb = 0;
while ( tmpc < maxroomx )
{
	while ( tmpb < maxroomy )
	{
		battlezone[tmpc][tmpb] = false;
		tmpb++;
	}
	tmpc++;
	tmpb = 0;
}

// Combat is over, but if there are monsters still alive, put them back in the level
tmpb = 0;
while ( tmpb < lastbeast )
{
	int tmpx = battlebeasts[tmpb][0]; battlebeasts[tmpb][0] = 0;
	int tmpy = battlebeasts[tmpb][1]; battlebeasts[tmpb][1] = 0;
	int tmpid = battlebeasts[tmpb][2]; battlebeasts[tmpb][2] = 0;
	objset[P1.roomnum][tmpx][tmpy][0] = 5;
	objset[P1.roomnum][tmpx][tmpy][1] = tmpid;
	tmpb++;
}
lastbeast = 0;

draws = true; // Render the screen again

}

// =================
// INT MAIN Routine
// =================

int main(int argc, char ** argv)
{
int oldencval = 0; oldencval -= 1;

fprintf(stdout, "SDLRogue, Version "); inttoout(VERREL); fprintf(stdout, "."); inttoout(VERMA); inttoout(VERMI);
fprintf(stdout, "\nCreated by: Nino v.d. Mark\n-------------------------------------------\n");

// Initialize Video & set the caption
InitVideo();
SDL_WM_SetCaption("SDL Rogue -- ALPHA", NULL);

srand((unsigned)time(0)); // Seed the random generator with the current system time

// Create and initialize required objects
Timer fps; // Timer
	
// Load the necessary files
if ( TTF_Init() == -1 ) { return 1; } // Initialize SDL_ttf 
if ( load_files() == false ) { return 1; } // Load all the files data/config files
if ( loadroom("base/levels/area1.rcf") == false ) { // Load the base level
	 fprintf(stderr,"ERROR: Level loading failure!! Exitting...\n"); return 1; }

// Assign the Clip values
assign_tiles(); 

draws = true;
drawui = 256;
if ( setbounds(P1.roomnum) == false ) { return 1; }

   //While the user hasn't quit
    while( quit == false )
	{
     	   //Start the frame timer
    	   fps.start();
		   
		if ( P1.moving == false ) // When the player isn't already moving
		{			 	 	 	
			Uint8 *keystate = SDL_GetKeyState(NULL);
			if ( keystate[SDLK_UP] ) // Holding the UP button
			{ 
			   	 int tmpa = roomset[P1.roomnum][P1.x][P1.y - 1];
			   	 int tmpb = objset[P1.roomnum][P1.x][P1.y - 1][0]; 
			   	 if ( collidable[0][tmpa] == false && collidable[1][tmpb] == false ) { P1.moving = true; }
   			   	 P1.facing = 1;
			   	 draws = true; 
			}
			else if ( keystate[SDLK_RIGHT] ) // Holding the Right button
			{ 
			   	 int tmpa = roomset[P1.roomnum][P1.x + 1][P1.y];
			   	 int tmpb = objset[P1.roomnum][P1.x + 1][P1.y][0];
			   	 if ( collidable[0][tmpa] == false && collidable[1][tmpb] == false ) { P1.moving = true; }
			   	 P1.facing = 2; 
			   	 draws = true;
			}
			else if ( keystate[SDLK_DOWN] ) // Holding the Down button
			{ 
			   	 int tmpa = roomset[P1.roomnum][P1.x][P1.y + 1];
			   	 int tmpb = objset[P1.roomnum][P1.x][P1.y + 1][0];
			   	 if ( collidable[0][tmpa] == false && collidable[1][tmpb] == false ) { P1.moving = true; }
			   	 P1.facing = 3;
			   	 draws = true;
			} 
			else if ( keystate[SDLK_LEFT] ) // Holding the Left button
			{ 
   			   	 int tmpa = roomset[P1.roomnum][P1.x - 1][P1.y];
			   	 int tmpb = objset[P1.roomnum][P1.x - 1][P1.y][0];
			   	 if ( collidable[0][tmpa] == false && collidable[1][tmpb] == false ) { P1.moving = true; }
			   	 P1.facing = 4;
			   	 draws = true;
			}
			else if ( keystate[SDLK_LCTRL] || keystate[SDLK_RCTRL]) // Holding the "Action Button"
			{
				 if ( ignoreCTRL == false ) { drawqui = true; ignoreCTRL = true; }
			}
			else if ( !keystate[SDLK_LCTRL] && !keystate[SDLK_RCTRL] )
			{
				 ignoreCTRL = false;
			}
		}

	Uint8 *keystate = SDL_GetKeyState(NULL); // Check for non-movement keys
	if ( keystate[SDLK_ESCAPE] )
	{
	   	 if ( ignoreESC == false ) { quit = true; fprintf(stdout, "[I] User pressed <ESCAPE>.\n"); }
	}
	else if ( keystate[SDLK_RETURN] && keystate[SDLK_LALT] )
	{
		if ( fullscreen == true ) { fullscreen = false; } else { fullscreen = true; }
		draws = true;
		drawui = 256;
	}
	else if ( !keystate[SDLK_ESCAPE] )
	{
	 	ignoreESC = false;
    }
        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
			//If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                fprintf(stdout, "[I] User closed the window.\n");
				quit = true;
            }		
        }
		// Actual Game Loop
		// When the player has just arrived on a new tile
		if ( poscheck == true )
		{  
		   if ( objset[P1.roomnum][P1.x][P1.y][0] != 0 )
		   {
			  if ( objset[P1.roomnum][P1.x][P1.y][0] == 1 ) // When you've stepped on a teleporter
			  {
			   	   int oldroom = P1.roomnum;
				   int tmpa = objset[P1.roomnum][P1.x][P1.y][1];
				   P1.roomnum = object[tmpa][1];
				   P1.x = object[tmpa][2];
				   P1.y = object[tmpa][3];
				   P1.facing = object[tmpa][4];
				 			 
				   P1.moving = false;
				   draws = true;
				   if ( P1.roomnum != oldroom ) { if ( setbounds(P1.roomnum) == false ) { return 1; } }
			  }
			  else if ( objset[P1.roomnum][P1.x][P1.y][0] == 3 ) // Standing on an "Open Door" tile
			  {			   	   
				   P1.moving = false;
				   poscheck = false;
				   draws = true;
			  }
			  else if ( objset[P1.roomnum][P1.x][P1.y][0] == 4 ) // Standing on a Key
			  {
			   	   int tmpa = objset[P1.roomnum][P1.x][P1.y][1];
				   P1.keys[tmpa]++;
				   objset[P1.roomnum][P1.x][P1.y][0] = 0;
				   drawui += 16;
			  }   	 
		   }
		   if ( roomenc[P1.x][P1.y] > 0 ) // Standing on an Encounter tile
		   {
				if ( randnum(0,20) < 11) { enccnt++; }
				//if ( enccnt > randnum(6,10) )
				if ( enccnt > 2 ) 
				{ 
					 // Player encounters monster
					 fprintf(stdout,"[N] Monster Encounter Ocurred.\n");

					 bool doneloc = false; bool failloccheck = false;
					 int tmpr = 0; int tmpb = 1; int tmpc = 1; int tmpd = 0; int tmpe = 0; int tmpface = 0;
					 while ( doneloc == false )
					 {
						  MakeMonsterLocationWork:
						  tmpb = randnum(0,2); tmpc = randnum(0,2);
						  if ( tmpb == tmpc && tmpb == 1 ) { goto MakeMonsterLocationWork; }
						  
						  if ( collidable[0][roomset[P1.roomnum][tmpb][tmpc]] == false && objset[P1.roomnum][tmpb][tmpc][0] == 0 )
						  { doneloc = true; }
						  else { tmpr++; }
						  
						  if ( tmpr == 12000 ) { doneloc = true; failloccheck = true; }
					 }
					 fprintf(stdout, "[N] Monster placement retried "); inttoout(tmpr); fprintf(stdout, " times!\n");
					 
					 tmpb -= 1; tmpc -= 1; tmpb = P1.x + tmpb; tmpc = P1.y + tmpc; // Changing tmpb and tmpc to absolute coordinates
					 
					 if ( tmpb < P1.x ) { tmpface = 3; } else if ( tmpb > P1.x ) { tmpface = 1; } // Set the monster's facing towards the player's X
					 if ( tmpc < P1.y ) { tmpface = 0; } else if ( tmpc > P1.y ) { tmpface = 2; } // Set the monster's facing towards the player's Y
					 
					 fprintf(stderr, "Creating Monster at: ");
					 inttoerr(P1.roomnum); fprintf(stderr, ", "); inttoerr(tmpb); fprintf(stderr, ", "); inttoerr(tmpc); fprintf(stderr, "...\n");
					 objset[P1.roomnum][tmpb][tmpc][0] = 5; objset[P1.roomnum][tmpb][tmpc][1] = nextobject;
					 tmpd = roomenc[tmpb][tmpc] - 1; tmpe = 4 + randnum(0,2);
					 
					 int prototyp = encset[P1.roomnum][tmpd][tmpe]; // Get the prototype number, we'll need this a few times so keep it someplace simple
					 object[nextobject][0] = prototyp; // Set the monster object to refer to the prototype
					 object[nextobject][1] = tmpb; object[nextobject][2] = tmpc; // Set the monster's X and Y to the previously determined values
					 object[nextobject][3] = 0; object[nextobject][4] = 0; // Movstep and Frame set to 0 
					 object[nextobject][5] = tmpface; // Set the monster's facing to the previously determined value
					 
					 // For now, assume monster is at level 1
					 // This should be altered in accordance with the leveling algorithm, which is yet to be written 
					 object[nextobject][6] = ptmonster[prototyp][0][1]; object[nextobject][7] = object[nextobject][6]; // Get HP & MaxHP
					 object[nextobject][8] = ptmonster[prototyp][0][2]; object[nextobject][9] = object[nextobject][8]; // Get MP & MaxMP
					 object[nextobject][10] = ptmonster[prototyp][0][3]; object[nextobject][11] = ptmonster[prototyp][0][4]; // STR, INT
					 object[nextobject][12] = ptmonster[prototyp][0][5]; object[nextobject][13] = ptmonster[prototyp][0][6]; // DEX, CON
					 object[nextobject][14] = 0;
					 
					 objectused[nextobject] = true;
					 nextobject++;
					 
					 int combres = combat(P1.stat[0], P1.stat[1], P1.stat[2], P1.stat[3], P1.stat[4], P1.stat[5], P1.stat[6], P1.stat[7]); // Start combat
					 if ( combres == 0 ) { fprintf(stderr, "HARD KILL\n"); return 0; }
					 SDL_Delay(2500);
					 
					 enccnt = 0;
				}
		   }
		   poscheck = false; P1.moving = false;
		}
		
		// When the player is 'moving'
		if ( P1.moving == true )
		{
	   	 	draws = true;
			P1.movstep += pstep;
			
			if ( crfrm == frmint ) // When Current Frame = Frame Interval
			{ 
			   	 if ( P1.pong == false ) // When animation is moving forward (Ping)
				 { 
				   	  P1.frame += 1; 
				   	  if ( P1.frame == 2 ) { P1.pong = true; } 
					  crfrm = 0;
				 }
				 else // When animation is moving backward (Pong)
				 { 	  
				 	  P1.frame -= 1;
					  if ( P1.frame == 0 ) { P1.pong = false; }
					  crfrm = 0;
				 }
			}
			else { crfrm += 1; } // Add 1 to Current Frame
			
			if ( P1.movstep >= 32 ) // When the player has arrived at the next tile
			{
			   if ( P1.facing == 1 ) { P1.y -= 1; P1.movstep = 0; poscheck = true; }
			   else if ( P1.facing == 2 ) { P1.x += 1; P1.movstep = 0; poscheck = true; }
			   else if ( P1.facing == 3 ) { P1.y += 1; P1.movstep = 0; poscheck = true; }
			   else if ( P1.facing == 4 ) { P1.x -= 1; P1.movstep = 0; poscheck = true; }
			}
		}
		
		if ( quit == false ) // Due to the process intensiveness, only execute when NOT already quitting
		{
		   	 if ( draws == true )
			 {
		   	  	  // Center the camera over the player without overshooting the level limits (where possible)
		   		  // Camera's X coordinates
		   		  if ( P1.x > 8 && P1.x < curroom_x - 7 ) { camera.x = ( P1.x * 32 + 64) - SCREEN_WIDTH / 2; } 
		   		  else if ( P1.x > 8 && curroom_x > 15) { camera.x = (( curroom_x - 5 ) * 32 ) - SCREEN_WIDTH /2; }
		   		  else { camera.x = 0; }
		    
		   		  // Camera's Y coordinates
		   		  if ( P1.y > 7 && P1.y < curroom_y - 7 ) { camera.y = ( P1.y * 32 + 16) - SCREEN_HEIGHT / 2; } 
		   		  else if ( P1.y > 7 && curroom_y > 14) { camera.y = (( curroom_y - 7 ) * 32 + 16 ) - SCREEN_HEIGHT / 2; }
		   		  else { camera.y = 0; }
		   
		   		  if ( P1.movstep > 0 ) // When the player's in between tiles
		   		  {
		   	  		   // Adjust the camera to the 'movstep' so the camera transitions smoothly between tiles as well
		   	  		   if ( P1.facing == 1 && P1.y > 7 && P1.y < (curroom_y - 6)) { camera.y -= P1.movstep; }
					   else if ( P1.facing == 2 && P1.x > 7 && P1.x < (curroom_x - 7)) { camera.x += P1.movstep; }
					   else if ( P1.facing == 3 && P1.y > 6 && P1.y < (curroom_y - 7)) { camera.y += P1.movstep; }
					   else if ( P1.facing == 4 && P1.x > 8 && P1.x < (curroom_x - 6)) { camera.x -= P1.movstep; }
		   		  }
		   		  if ( fullscreen != lastfull ) { 
					   if ( fullscreen == true ) { 
							outvideoset();
							SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_FULLSCREEN ); }
					   if ( fullscreen == false ) { 
							outvideoset();
							SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_DOUBLEBUF | SDL_HWSURFACE  ); } }
				  
				  // All set, let's render!
				  querender(false);

				  lastfull = fullscreen;
			 }
		   	 //Cap the frame rate
           	 if ( fps.get_ticks() < 1000 / FRAMES_PER_SECOND )
           	 {
           	   	  SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps.get_ticks() );
           	 }
			 
			 if ( drawui > 0 ) // ReDraw the UI (the right inventory bar)
			 {			 
				  if ( drawui >= 64 ) { 
					   /* Inventory */ 
					   drawui -= 64; 
				  }
				  if ( drawui >= 32 ) {
				  	   sidebar(5,P1.cash,0,0,0,0,0,0,0,""); // Cash
					   drawui -= 32;
				  }
				  if ( drawui >= 16 ) {
				       sidebar(4,P1.keys[0],P1.keys[1],P1.keys[2],P1.keys[3],P1.keys[4],P1.keys[5],P1.keys[6],P1.keys[7],""); // Keys
					   drawui -= 16;
				  }
				  if ( drawui >= 8 ) {
					   string tmpstr = "25";
					   sidebar(3,P1.MagicSkill[0][0],P1.MagicSkill[1][0],P1.MagicSkill[2][0],P1.MagicSkill[3][0], // Ability skills
								 P1.WeaponSkill[0][0],P1.WeaponSkill[1][0],P1.WeaponSkill[2][0],P1.WeaponSkill[3][0],
								 tmpstr
							   );
					   drawui -= 8;
				  }
				  if ( drawui >= 4 ) {
					   int tmpc = static_cast<int>(P1.stat[0]); int tmpd = static_cast<int>(P1.stat[1]);
					   int tmpe = static_cast<int>(P1.stat[2]); int tmpf = static_cast<int>(P1.stat[3]);
					   int tmpg = static_cast<int>(P1.stat[4]); int tmph = static_cast<int>(P1.stat[5]);
					   sidebar(2,tmpc,tmpd,tmpe,tmpf,tmpg,tmph,0,0,""); // Stats
					   drawui -= 4;
				  }
				  if ( drawui >= 2 ) {
				  	   sidebar(1,P1.level[0],P1.level[1],P1.exp,P1.nextexp,0,0,0,0,""); // Level & Experience
					   drawui -= 2;
				  }
				  if ( drawui >= 1 ) {
					   sidebar(0,P1.HP,P1.MAXHP,P1.MP,P1.MAXMP,0,0,0,0,P1.name); // Name, HP & MP
					   drawui = 0;
				  }
				  
			 }
			 
			 if ( drawqui == true ) // Draw the Quick UI (action menu)
			 {
			  	  int tmpc = 0; int tmpd = 0; int tmpe = 0;
			  	  // Gather the objecttype and objectid from the object the player's looking at		  
				  tmpc = getnextobj(P1.roomnum, P1.x, P1.y, P1.facing, 0); // Type
				  tmpd = getnextobj(P1.roomnum, P1.x, P1.y, P1.facing, 1); // ID		  
				  
				  int PLAYERVAR = 0;
				  
				  int quix = 0;
				  if ( (P1.x*32)-camera.x > 255 ) { quix = 0; }		// Draw top left
				  else { quix = 416; }								// Draw top right
				   
				  // Pass the relevant object property and Player Variable
				  if ( tmpc == 2 ) 
				  { 
				  	   tmpe = object[tmpd][2]; 
					   if 	   ( tmpe == 0 ) { PLAYERVAR = 1; } // If not locked, just pass 1 (variable isn't used)
   					   else if ( P1.keys[tmpe] > 0 && P1.keys[0] > 0 ) { PLAYERVAR = 2; } // Player has 2 usable keys
					   else if ( P1.keys[tmpe] > 0 ){ PLAYERVAR = 1; } // Just the required key
					   else if ( P1.keys[0] > 0 ) { PLAYERVAR = 3; } // Just the masterkey
				  }

				  int quiout = quimode( quix, tmpc, tmpe, PLAYERVAR );
				   
				  if ( quiout == 1 ) // Open the Door
				  {
				   	   setnextobj(P1.roomnum, P1.x, P1.y, P1.facing, 0, 3);
				  }
				  else if ( quiout == 2 ) // Close the Door
				  {
				   	   setnextobj(P1.roomnum, P1.x, P1.y, P1.facing, 0, 2);
				  }
				  else if ( quiout == 3 )  // Unlock the Door
				  {
				   	   if ( PLAYERVAR == 2 )
					   {
				   	   	  	ignoreCTRL = true;
							tmpa = keyqui(tmpe);
							if ( tmpa == 0 ) { object[tmpd][2] = 0; P1.keys[tmpe]--; drawqui = true; } //Use the normal key
					   		else if ( tmpa == 1 ) { object[tmpd][2] = 0; P1.keys[0]--; drawqui = true; } //Use the masterkey
							else if ( tmpa == 42 ) { drawqui = false; }
							else { drawqui = false; fprintf(stderr, "WARNING: Unknown exception thrown (Undefined keyQUI output)"); }
							drawui += 16;
					   }
					   else if ( PLAYERVAR == 3)
					   {
							object[tmpd][2] = 0;
							P1.keys[0]--;
							drawqui = true;
							drawui += 16;
					   }
					   else 
					   {
					   		object[tmpd][2] = 0;
							P1.keys[tmpe]--;
							drawqui = true;
							drawui += 16;
					   } 
				  }
				  else if ( quiout == 5 )
				  {
				   	   tmpa = getnextobj(P1.roomnum, P1.x, P1.y, P1.facing, 1);
					   P1.keys[tmpa]++;
					   setnextobj(P1.roomnum, P1.x, P1.y, P1.facing, 0, 0);
					   drawui += 16;
				  }
				   
				  ignoreCTRL = true;
				  draws = true;

			 } // End of QUI
			 
		} // End of if quit == false

	} // End of While quit == false

fprintf(stdout, "[I] Cleaning up Bitmaps...\n");
// Free Surfaces
SDL_FreeSurface(objimg);
SDL_FreeSurface(guiimg);
SDL_FreeSurface(worldimg);
SDL_FreeSurface(screen);
SDL_FreeSurface(text);

fprintf(stdout, "[I] Cleaning up Font Rendering...\n");
// Get rid of the SDL_TTF stuff
TTF_CloseFont( font2 ); 
TTF_Quit();

fprintf(stdout, "[I] Program terminated!\n");
return 0;
}

/*==================================================5 NECESSARY BULK 5===============================================*/

void assign_tiles() // Assigns the coordinates for each tile # for clipname
{
 	miniclip[ 0 ].x = 256;
	miniclip[ 0 ].y = 32;
	miniclip[ 0 ].w = 16;
	miniclip[ 0 ].h = 8;
 	
	miniclip[ 1 ].x = 272;
	miniclip[ 1 ].y = 32;
	miniclip[ 1 ].w = 16;
	miniclip[ 1 ].h = 8;
	
	miniclip[ 2 ].x = 256;
	miniclip[ 2 ].y = 40;
	miniclip[ 2 ].w = 16;
	miniclip[ 2 ].h = 8;
	
	miniclip[ 3 ].x = 272;
	miniclip[ 3 ].y = 40;
	miniclip[ 3 ].w = 16;
	miniclip[ 3 ].h = 8;
	
	miniclip[ 4 ].x = 256;
	miniclip[ 4 ].y = 48;
	miniclip[ 4 ].w = 16;
	miniclip[ 4 ].h = 8;
	
	miniclip[ 5 ].x = 272;
	miniclip[ 5 ].y = 48;
	miniclip[ 5 ].w = 16;
	miniclip[ 5 ].h = 8;
	
	miniclip[ 6 ].x = 256;
	miniclip[ 6 ].y = 56;
	miniclip[ 6 ].w = 16;
	miniclip[ 6 ].h = 8;
	
	miniclip[ 7 ].x = 272;
	miniclip[ 7 ].y = 56;
	miniclip[ 7 ].w = 16;
	miniclip[ 7 ].h = 8;
	
    int tmpq = 1;
	int tmpx = 0;
	int tmpy = 0;
	while ( tmpq < maxclipnum )
	{
	 	  clip[ tmpq ].x = tmpx;
		  clip[ tmpq ].y = tmpy;
		  clip[ tmpq ].w = 32;
		  clip[ tmpq ].h = 32;
		  
		  if ( tmpx == 480 ) { tmpx = 0; tmpy += 32; } else { tmpx += 32; }
		  tmpq++;
	}
}
