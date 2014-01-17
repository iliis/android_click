#include <cstdlib>
#include <SDL.h>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>


using namespace std;

// this program sends mouse presses and movements to android via adb
// press 'U' or SPACE to load screen
// press ESCAPE to quit
//
// use this command to get a view of the screen:
// adb shell screencap -p | sed 's/\r$//' > screen.png


//------------------------------------------------------------------------------------------------
template <typename Typ>
string ToString(Typ bla)
{
	ostringstream tmp;
	tmp << bla;
	return tmp.str();
}
//------------------------------------------------------------------------------
string inline const operator+(string const& a, double b) {return a+ToString(b);};
string inline const operator+(string const& a, float  b) {return a+ToString(b);};
string inline const operator+(string const& a, int    b) {return a+ToString(b);};
string inline const operator+(string const& a, unsigned int         b) {return a+ToString(b);};
string inline const operator+(string const& a, long int             b) {return a+ToString(b);};
string inline const operator+(string const& a, unsigned long int    b) {return a+ToString(b);};


int main ( int argc, char** argv )
{
	if (argc != 3) {
		printf("usage: actrl width height\n");
		return 1;
	}

	int width = atoi(argv[1]);
	int height = atoi(argv[2]);

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(width, height, 16,
                                           SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set %dx%d video: %s\n", width, height, SDL_GetError());
        return 1;
    }

	SDL_Surface* screenshot = NULL;

	// previous mousepress:
	int x = 0, y = 0;
	unsigned int last_time = 0;

    // program main loop
    bool done = false;
    while (!done)
    {
        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
                {
                    // exit if ESCAPE is pressed
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = true;

					if (event.key.keysym.sym == SDLK_u || event.key.keysym.sym == SDLK_SPACE) {
						printf("getting screenshot\n");

						if (screenshot)
							SDL_FreeSurface(screenshot);

						// clear screen to indicate loading
						SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 200, 0));
						SDL_Flip(screen);

						// get screenshot
						system("adb shell screencap -p | sed 's/\r$//' > screen.png; convert screen.png screen.bmp");
						screenshot = SDL_LoadBMP("screen.bmp");
						system("rm screen.png screen.bmp");

						if (!screenshot)
							printf("failed to load image\n");
					}
                    break;
                }

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					// printf("%d %d\n", event.button.x, event.button.y);
					// fflush(stdout);

					x = event.button.x;
					y = event.button.y;
					last_time = SDL_GetTicks();
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT) {
					if (abs(x-event.button.x) > 0 || abs(y-event.button.y) > 0 || SDL_GetTicks() - last_time > 100) {
						// did the mouse move during press?
						// if so, send swipe instead of touch
						string tmp = string("adb shell input touchscreen swipe ") + x + " " + y
							+ " " + event.button.x + " " + event.button.y
							+ " " + (SDL_GetTicks() - last_time);
						system(tmp.c_str());
						cout << tmp << endl;
					} else {
						// otherwise, send normal click
						string tmp = string("adb shell input tap ") + event.button.x + " " + event.button.y;
						system(tmp.c_str());
						cout << tmp << endl;
					}
				}
				break;
            } // end switch
        } // end of message processing

        // DRAWING STARTS HERE

        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

		if (screenshot)
			SDL_BlitSurface(screenshot, 0, screen, 0);

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);

		SDL_Delay(10); // keep cpu usage down
    } // end main loop

	if (screenshot)
		SDL_FreeSurface(screenshot);

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}
