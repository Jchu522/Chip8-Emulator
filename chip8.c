#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include "SDL.h"

// Container
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

//configuration object
typedef struct {
    uint32_t window_width; //SDL window width
    uint32_t window_height; //SDL window height
    uint32_t fg_color; //Foreground color
    uint32_t bg_color; //Background color
    uint32_t scale_factor; // Amount to scale a chip8 pixel by e.g. 20x will be 20x larger window
}config_t;

//Emulator states
typedef enum {
    QUIT  ,
    RUNNING ,
    PAUSED ,
}emulatior_state_t;

//CHIP8 Machine object
typedef struct{
    emulatior_state_t state;
    uint8_t ram[4096];   
    bool display[64*32]; //original chip8 resolution pixels
    uint16_t stack[12];  // Subroutine stack
    uint8_t V[16];       // Data registars from V0 to VF
    uint16_t I;          // index registar 
    uint8_t delay_timer; //decrements at 60hz when >0
    uint8_t sound_timer; //decrements at 60hz and play tone when >0 and will play tone
    bool keypad[16];     //Hex keypad 0x0-0xF
    char *rom_name;      //Currently running Rom
}chip8_t;



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
        .bg_color = 0x00000000, //black
        .scale_factor = 20 //default will be 1280x640
    };
    //rewrite default from passed in args
    for (int i = 1; i < argc; i++){
        //...
        (void)argv[i]; //prevent error from unused var arc arrv
    }

    return true;
}

// initialize chip8 machine
bool init_chip8(chip8_t *chip8, const char rom_name[]){
    chip8->state = RUNNING; //default on

    //Load Font

    //Load ROM into chip8 mem

    //Set chip8 machine defaults
    return true; //success
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

//handle inputs
void handle_input(chip8_t *chip8){
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
            switch (event.type){
                case SDL_QUIT:
                    //end program
                    chip8->state = QUIT; //exits the main loop
                    return;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            //when escape is pressed Exit
                            chip8->state = QUIT;
                            return;
                    }
                    break;

                case SDL_KEYUP:
                    break;

                default:
                    break;
                //exit window
                
            }
            // if (event.type == SDL_QUIT) {
            //     running = false;
            // }
        }
    // return running;
}

//da main
int main(int argc, char **argv) {

    // initialize emulator config/options
    config_t config = {0};
    if( !set_config_from_args(&config, argc, argv)) exit(EXIT_FAILURE);

    // initialize SDL
    sdl_t sdl = {0};
    if (!init_sdl(&sdl, config)) exit(EXIT_FAILURE);

    //initialize chip8 machine
    chip8_t chip8 = {0};
    if (!init_chip8(&chip8)) exit(EXIT_FAILURE);
    //initial screen clear to backroud color
    clear_screen(sdl, config);

    // Main emulator loop 

     while (chip8.state != QUIT) {
        // Handle user inputs 
        // Handle events - required for macOS
        handle_input(&chip8);
        // if chip8 state paused continue;

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

