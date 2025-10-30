#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include "SDL.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;
typedef struct {
    uint32_t window_width; //SDL window width
    uint32_t window_height; //SDL window height
    uint32_t fg_color; //Foreground color
    uint32_t bg_color; //Background color
    uint32_t scale_factor; // Amount to scale a chip8 pixel by e.g. 20x will be 20x larger window



}config_t;

//initialize SDL
bool init_sdl(sdl_t *sdl, const config_t config) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) !=0){
        SDL_Log("Could not init SDL subsystems! %s\n", SDL_GetError());
        return false;
    }
    sdl->window = SDL_CreateWindow("CHIP8 Emulator", SDL_WINDOWPOS_CENTERED, 
                                                     SDL_WINDOWPOS_CENTERED, 
                                                     config.window_width * config.scale_factor, 
                                                     config.window_height * config.scale_factor, 
                                                     0);
    if (!sdl->window) {
        SDL_Log("could not create SDL window %s\n", SDL_GetError());
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);

    if(!sdl->renderer){
        SDL_Log("could not create SDL window %s\n", SDL_GetError());
        return false;
    }

    return true; //success
}



//Initize the initial emulator config from passed in args
bool set_config_from_args(config_t *config, int argc, char **argv){
    //Set default
    *config = (config_t){
        .window_width = 64, //64 by 32 original default original window 
        .window_height = 32,
        .fg_color = 0xFFFFFFFF, //white
        .bg_color = 0xFFFF00FF, //yellow
        .scale_factor = 20 //default will be 1280x640
    };
    //rewrite default from passed in args
    for (int i = 1; i < argc; i++){
        //...
        (void)argv[i]; //prevent error from unused var arc arrv
    }

    return true;
}

//final Cleanup
void final_cleanup(const sdl_t sdl) {
    SDL_DestroyRenderer(sdl.renderer);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit(); //shut down SDL subsystems
}

// clear screen / sdl window to background color
void clear_screen(const sdl_t sdl, const config_t config){
    const uint8_t r = (config.bg_color >> 24) & 0xFF;
    const uint8_t g = (config.bg_color >> 16) & 0xFF;
    const uint8_t b = (config.bg_color >> 8) & 0xFF;
    const uint8_t a = (config.bg_color >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r , g , b, a);
    SDL_RenderClear(sdl.renderer);
}

//updates window with any changes
void update_screen(const sdl_t sdl) {
    SDL_RenderPresent(sdl.renderer);

}
//da main
#undef main


int main(int argc, char **argv) {

    // initialize emulator config/options
    config_t config = {0};
    if( !set_config_from_args(&config, argc, argv)) exit(EXIT_FAILURE);

    // initialize SDL
    sdl_t sdl = {0};
    if (!init_sdl(&sdl, config)) exit(EXIT_FAILURE);

    //initial screen clear to backroud color
    clear_screen(sdl, config);

    // Main emulator loop 
    bool running = true;
     while (running) {
        // Handle events - required for macOS
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        //get_time()
        //emulate chip8 instructions
        //get_time() elapsed since last get_time

        //maybe delay approx 60hz/60fps (16.67 ms)
        SDL_Delay(16);

        //update window with changes
        clear_screen(sdl, config);
        update_screen(sdl);
    } 

    //FINAL Clean up functions 
    final_cleanup(sdl);

    exit(EXIT_SUCCESS);
    puts("TESTING ON MAC");
}



/*
int main(int argc, char **argv) {
    // This is critical for macOS
    SDL_SetMainReady();
    
    printf("Program started\n");
    fflush(stdout);

    // initialize emulator config/options
    config_t config = {0};
    if( !set_config_from_args(&config, argc, argv)) exit(EXIT_FAILURE);
    printf("Config set\n");
    fflush(stdout);

    // initialize SDL - use SDL_INIT_VIDEO only for now
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Log("Could not init SDL subsystems! %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    sdl_t sdl = {0};
    sdl.window = SDL_CreateWindow("CHIP8 Emulator", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 
                                  config.window_width * config.scale_factor, 
                                  config.window_height * config.scale_factor, 
                                  SDL_WINDOW_SHOWN);  // Add SDL_WINDOW_SHOWN flag
    
    if (!sdl.window) {
        SDL_Log("could not create SDL window %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    printf("Window created\n");
    fflush(stdout);

    sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_ACCELERATED);
    if(!sdl.renderer){
        SDL_Log("could not create renderer %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    printf("Renderer created\n");
    fflush(stdout);

    // Main emulator loop 
    bool running = true;
    printf("Entering main loop\n");
    fflush(stdout);
    
    while (running) {
        // MUST process events on macOS!
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                printf("Quit event received\n");
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                printf("Escape pressed\n");
                running = false;
            }
        }

        clear_screen(sdl, config);
        update_screen(sdl);
        SDL_Delay(16);
    } 

    printf("Cleaning up\n");
    final_cleanup(sdl);
    printf("Done\n");

    return 0;
}*/