#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "SDL.h"

typedef struct {
    SDL_Window *window;
} sdl_t;
typedef struct {
    uint32_t window_width; //SDL window width
    uint32_t window_height; //SDL window height
}config_t;

//initialize SDL
bool init_sdl(sdl_t *sdl, const config_t config) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) !=0){
        SDL_Log("Could not init SDL subsystems! %s\n", SDL_GetError());
        return false;
    }
    sdl->window = SDL_CreateWindow("CHIP8 Emulator", SDL_WINDOWPOS_CENTERED, 
                                                     SDL_WINDOWPOS_CENTERED, 
                                                     config.window_width, 
                                                     config.window_height, 
                                                     0);
    if (!sdl->window) {
        SDL_Log("could not create SDL window %s\n", SDL_GetError());
    }
    return true; //success
}

//final Cleanup
void final_cleanup(const sdl_t sdl) {
    SDL_DestroyWindow(sdl.window);
    SDL_Quit(); //shut down SDL subsystems
}

//Initize the initial emulator config from passed in args
bool set_config_from_args(config_t *config, int argc, char **argv){
    //Set default
    *config = (config_t){
        .window_width = 64, //64 by 32 original default
        .window_height = 32,
    };
    //rewrite default from passed in args
    for (int i = 1; i < argc; i++){
        //...
        (void)argv[i]; //prevent error from unused var arc arrv
    }

    return true;
}

//da main
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;


    // initialize emulator config/options
    config_t config = {0};
    if( !set_config_from_args(&config, argc, argv)) exit(EXIT_FAILURE);

    // initialize SDL
    sdl_t sdl = {0};
    if (!init_sdl(&sdl, config)) exit(EXIT_FAILURE);


    //FINAL Clean up functions 
    final_cleanup(sdl);

    exit(EXIT_SUCCESS);
    puts("TESTING ON MAC");
}