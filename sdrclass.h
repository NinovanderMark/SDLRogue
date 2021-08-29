// Class definitions
// -- Part of SDLRogue

#ifndef SDRCLASS_H
#define SDRCLASS_H

int roundn(double a); 
int randnum(int min, int max);
SDL_Surface *load_image( std::string filename );
bool filexist(const char* thefilename);

class Timer 
{

	private:
	int startTicks;
	int pausedTicks;

	bool paused;
	bool started;

	public:
	Timer();
    
	void start();
	void stop();
	void pause();
	void unpause();
    
	int get_ticks();
	bool is_started();
	bool is_paused();
};

class Player 
{

	int width;
	int height;
	  
	public:
	Player(int a, int c, int d, std::string setname);
	int roomnum;
	int x;
	int y;
    
	std::string name;

	int HP; int MAXHP;
	int MP; int MAXMP;
	float stat[8]; // Respectively: STR, INT, DEX, CON, CHA, LUC, End. Delay, End. Gain
	int level[2];
	int nextexp;
	int exp;

	int MagicSkill[4][2];
	int WeaponSkill[4][2];

	int cash;
	int keys[8];

	bool moving;	
	
	int facing;
	int movstep;
	int frame;
	bool pong;

	void addhealth(int);
};

/*class GameLevel
{

	private:
	int maxrooms = 256;
	int maxroomd = 40;
	int maxenca = 9;
	int maxobjs = 5000;

	public:
	int tiles[maxroomd][maxroomd];
	int fgtiles[maxroomd][maxroomd];
	int entities[maxroomd][maxroomd][2];
	int objects[maxobjs][31];
	int enczones[maxrooms][maxenca];

	// Which object # is the next empty slot
	int nextobject = 0;

	GameLevel();
	bool LoadFromFile(char* filename);

}

class EditorLevel
{

	private:
	int maxint = 32767;

	public:
	int tiles[maxint][7];
	int fgtiles[maxint][7];
	int entities[maxint][5];
	int objects[maxint][3];
	int enczones[maxint][11];

	// Which object # is the next empty slot
	int nextobject = 0;

	EditorLevel();
	bool LoadFromFile(char* filename);
	bool SaveToFile(char* filename);

}*/

#endif
