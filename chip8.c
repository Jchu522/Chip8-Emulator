#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "SDL.h"

//initialize SDL
bool init_sdl(void) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) !=0){
        SDL_Log("Could not init SDL subsystems! %s\n", SDL_GetError());
        return false;
    }
    return true; //success
}
//final Cleanup
void final_cleanup(void) {
    SDL_Quit(); //shut down SDL subsystems
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // initialize SDL
    if (!init_sdl()) exit(EXIT_FAILURE);

    //FINAL Clean up functions 
    final_cleanup();

    exit(EXIT_SUCCESS);
    puts("TESTING ON MAC");
}