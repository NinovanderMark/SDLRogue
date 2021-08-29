#include "SDL/include/SDL.h"
#include <iostream>
#include <string>
#include "sdrclass.h"

//Simple number rounding routine
int roundn(double a) 
{
	return int(a + 0.5);
}

// Procedure to generate random numbers
int randnum(int min, int max)
{		
	int tmp=(max-min)+1;
	return min+int(tmp*rand()/(RAND_MAX + 1.0));
}


SDL_Surface *load_image( std::string filename ) 
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;
    
    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;
    
    //Load the image
    loadedImage = SDL_LoadBMP( filename.c_str() );
    
    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );
        
        //Free the old surface
        SDL_FreeSurface( loadedImage );
        
        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_RLEACCEL | SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0xFF, 0, 0xFF ) );
        }
    }
    
    //Return the optimized surface
    return optimizedImage;
}

Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;    
}

void Timer::start()
{
    //Start the timer
    started = true;
    
    //Unpause the timer
    paused = false;
    
    //Get the current clock time
    startTicks = SDL_GetTicks();    
}

void Timer::stop()
{
    //Stop the timer
    started = false;
    
    //Unpause the timer
    paused = false;    
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;
    
        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;
    
        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;
        
        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }    
    }
    
    //If the timer isn't running
    return 0;    
}

bool Timer::is_started()
{
    return started;    
}

bool Timer::is_paused()
{
    return paused;    
}


// Player Class
Player::Player( int a, int c, int d, std::string setname ) {

	 width = 32;
	 height = 32;

	 moving = false;	 
	 
	 movstep = 0;
	 facing = 1;
	 frame = 0;
	 pong = false;

	 roomnum = a;
	 x = c;
	 y = d;

	name = setname;
	HP = 30; MAXHP = 30;
	MP = 5; MAXMP = 15;

	level[0] = 1;
	level[1] = 0;
	nextexp = 200;
	exp = 0;

	cash = 5;
	
	int tmpa = 0;
	int tmpb = 0;
	while ( tmpa < 6 ){
		stat[tmpa] = 5;
		tmpa++;
	}
	
	stat[6] = 25;
	stat[7] = 1;

	tmpa = 0;
	tmpb = 0;
	while ( tmpa < 4 ){
	while ( tmpb < 2 ){
		MagicSkill[tmpa][tmpb] = 1;
		WeaponSkill[tmpa][tmpb] = 5;
		tmpb++;
	}
	tmpa++;
	tmpb = 0;
	}

	keys[0] = 0;
	keys[1] = 0;
	keys[2] = 0;
	keys[3] = 0;
	keys[4] = 0;
	keys[5] = 0;
	keys[6] = 0;
	keys[7] = 0;
}

void Player::addhealth(int amount)
{
	HP += amount;
	if ( HP > MAXHP ) { HP = MAXHP; }
}

/*// Standard Level Class
GameLevel::GameLevel
{

	// Set all the places in all the arrays to 0
	int tmpa = 0; int tmpb = 0;
	while ( tmpb < maxroomd )
	{
		while ( tmpa < maxroomd )
		{
			
		}
	}

}*/
