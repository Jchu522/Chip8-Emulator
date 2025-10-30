#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <string.h> 
#include "SDL.h"

// Container
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;
typedef struct {
    uint16_t opcode; 
    uint16_t NNN;    //12 bit address/constant
    uint8_t NN;      //8 bit constant
    uint8_t N;       //4 bit constant
    uint8_t X;       //4 bit register identifier
    uint8_t Y;       //4 bit register identifier
}instruction_t;
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
    uint16_t *stack_ptr;  // stack pointer
    uint8_t V[16];       // Data registars from V0 to VF
    uint16_t I;          // index registar 
    uint16_t PC;         // Program Counter
    uint8_t delay_timer; //decrements at 60hz when >0
    uint8_t sound_timer; //decrements at 60hz and play tone when >0 and will play tone
    bool keypad[16];     //Hex keypad 0x0-0xF
    const char *rom_name;      //Currently running Rom
    instruction_t inst; // currently exe instruction
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
    const uint32_t entry_point = 0x200; //it will be loaded at x200
    const uint8_t font[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    //Load Font 
    memcpy(&chip8->ram[0], font, sizeof(font));

    //open ROM file
    FILE *rom = fopen(rom_name, "rb"); 
    if(!rom){
        SDL_Log("Rom file %s is invalid or does not exist \n", rom_name);
        return false;
    }

    //get and check rom size
    fseek(rom,0,SEEK_END);
    const size_t rom_size = ftell(rom);
    const size_t max_size = sizeof chip8->ram - entry_point;
    rewind(rom);

    if (rom_size > max_size){
        SDL_Log("Rom file %s is too big! Rom size:%zu, Max size allowed: %zu\n", rom_name, rom_size, max_size);
        return false;
    }

    if (fread(&chip8->ram[entry_point], rom_size, 1, rom) !=1){
        SDL_Log("could not read Rom file %s into chip8 memory\n", rom_name);
        return false;
    }

    fclose(rom);
    //Set chip8 machine defaults
    chip8->state = RUNNING; //default is running
    chip8->PC = entry_point; //start at the entry 
    chip8->rom_name = rom_name;
    chip8->stack_ptr = &chip8->stack[0];
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
void update_screen(const sdl_t sdl, config_t config, const chip8_t chip8) {
    SDL_Rect rect = {.x = 0,.y = 0 ,.w = config.scale_factor,.h = config.scale_factor};
    // grab color values to draw
    const uint8_t fg_r = (config.fg_color >> 24) & 0xFF;
    const uint8_t fg_g = (config.fg_color >> 16) & 0xFF;
    const uint8_t fg_b = (config.fg_color >> 8) & 0xFF;
    const uint8_t fg_a = (config.fg_color >> 0) & 0xFF;
    
    const uint8_t bg_r = (config.bg_color >> 24) & 0xFF;
    const uint8_t bg_g = (config.bg_color >> 16) & 0xFF;
    const uint8_t bg_b = (config.bg_color >> 8) & 0xFF;
    const uint8_t bg_a = (config.bg_color >> 0) & 0xFF;
    for (uint32_t i = 0; i <sizeof chip8.display; i++){
        //translate ID index i value to 2D x/Y coords
        // X = i % window width
        // Y = i / window width
        rect.x = (i % config.window_width) * config.scale_factor;
        rect.y = (i / config.window_width) * config.scale_factor;
        if (chip8.display[i]){
            //if pixel is on draw foreground
            SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }else{
            //draw background
            SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }
 
    }
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
                        case SDLK_SPACE:
                            //Space bar pauses
                            if (chip8->state == RUNNING){
                                chip8->state = PAUSED; //pause
                            }
                            else{
                                chip8->state = RUNNING; //resume
                                puts("==== PAUSED ====");
                            }
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

#ifdef DEBUG
void print_debug_info(chip8_t *chip8){
    printf("Address: 0x%04x, Opcode: 0x%04X Description:", chip8->PC-2, chip8->inst.opcode);
    switch ((chip8-> inst.opcode >> 12) & 0x0F){
        case 0x0:
            if  (chip8->inst.NN == 0xE0){
                //clear
                printf("clear screen\n");
                memset(&chip8->display[0], false, sizeof chip8->display);
            }else if (chip8->inst.NN == 0xEE){
                //return from subroutine
                //set program counter to popped stack from subroutine
                printf("return from subroutine to address 0x%04X\n"), chip8->PC = *--chip8->stack_ptr
                chip8->PC = *--chip8->stack_ptr;
            }else{
                printf("unimplemented opcode.\n")

            }
            break;
            
        case 0x01:
            // 0x1NNN: Jump to address NNN
            printf("Jump to address NNN (0x%04X)\n",
                   chip8->inst.NNN);   
            break;


        case 0x02:
        // call subroutine at NNN
            *chip8->stack_ptr++ = chip8->PC; // store current address to return to (push onto stack)
            chip8->PC = chip8->inst.NNN; // set pc to subroutine address
            break;
        
         case 0x06:
            // 0x6XNN: Set register VX to NN
            printf("Set register V%X = NN (0x%02X)\n",
                   chip8->inst.X, chip8->inst.NN);
            break;

        case 0x07:
            // 0x7XNN: Set register VX += NN
            printf("Set register V%X (0x%02X) += NN (0x%02X). Result: 0x%02X\n",
                   chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.NN,
                   chip8->V[chip8->inst.X] + chip8->inst.NN);
            break;

        case 0x0A:
            // 0xANNN: set index register I to NNN
            printf("set I to NNN (0x%04X)\n", 
                chip8 = chip8->inst.NNN);
            break;
        case 0x0D:
            // 0xDXYN: Draw N-height sprite at coords X,Y; Read from memory location I;
            //   Screen pixels are XOR'd with sprite bits, 
            //   VF (Carry flag) is set if any screen pixels are set off; This is useful
            //   for collision detection or other reasons.
            printf("Draw N (%u) height sprite at coords V%X (0x%02X), V%X (0x%02X) " "from memory location I (0x%04X). Set VF = 1 if any pixels are turned off.\n",
                   chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y, chip8->V[chip8->inst.Y], chip8->I);
            break;
        

        default:
            break; //unimplemented
    }
}
#endif

//emulate 1 chip8 instruction
void emulate_instruction(chip8_t *chip8, const config_t config){
    (void)config;
    //get opcode from ram
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC+1];
    chip8->PC += 2; //pre-increment PC for next opcode

    // fill out inst format
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x0FF;
    chip8->inst.N = chip8->inst.opcode & 0x0F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
    print_debug(info(chip8));
#endif


    //emulate opcode
    switch ((chip8->inst.opcode >> 12) & 0x0F){
        case 0x0:
            if  (chip8->inst.NN == 0xE0){
                //clear
                memset(&chip8->display[0], false, sizeof chip8->display);
            }else if (chip8->inst.NN == 0xEE){
                //return from subroutine
                //set program counter to popped stack from subroutine
                chip8->PC = *--chip8->stack_ptr;
            }
            break;
        case 0x01:
            // 0x1NNN: Jump to address NNN
            chip8->PC = chip8->inst.NNN;    // Set program counter so that next opcode is from NNN
            break;

        case 0x02:
            // 0x2NNN: Call subroutine at NNN
            // Store current address to return to on subroutine stack ("push" it on the stack)
            //   and set program counter to subroutine address so that the next opcode
            //   is gotten from there.
            *chip8->stack_ptr++ = chip8->PC;  
            chip8->PC = chip8->inst.NNN;
            break;
        
        case 0x03:
            // 0x3XNN: Check if VX == NN, if so, skip the next instruction
            if (chip8->V[chip8->inst.X] == chip8->inst.NN)
                chip8->PC += 2;       // Skip next opcode/instruction
            break;
        
        case 0x06:
            // 0x6XNN: Set register VX to NN
            chip8->V[chip8->inst.X] = chip8->inst.NN;
            break;

        case 0x07:
            // 0x7XNN: Set register VX += NN
            chip8->V[chip8->inst.X] += chip8->inst.NN;
            break;

        case 0x0A:
            // 0xANNN: Set index register I to NNN
            chip8->I = chip8->inst.NNN;
            break;

        case 0x0D: {
            // 0xDXYN: Draw N-height sprite at coords X,Y; Read from memory location I;
            //   Screen pixels are XOR'd with sprite bits, 
            //   VF (Carry flag) is set if any screen pixels are set off; This is useful
            //   for collision detection or other reasons.
            uint8_t X_coord = chip8->V[chip8->inst.X] % config.window_width;
            uint8_t Y_coord = chip8->V[chip8->inst.Y] % config.window_height;
            const uint8_t orig_X = X_coord; // Original X value

            chip8->V[0xF] = 0;  // Initialize carry flag to 0

            // Loop over all N rows of the sprite
            for (uint8_t i = 0; i < chip8->inst.N; i++) {
                // Get next byte/row of sprite data
                const uint8_t sprite_data = chip8->ram[chip8->I + i];
                X_coord = orig_X;   // Reset X for next row to draw

                for (int8_t j = 7; j >= 0; j--) {
                    // If sprite pixel/bit is on and display pixel is on, set carry flag
                    bool *pixel = &chip8->display[Y_coord * config.window_width + X_coord]; 
                    const bool sprite_bit = (sprite_data & (1 << j));

                    if (sprite_bit && *pixel) {
                        chip8->V[0xF] = 1;  
                    }

                    // XOR display pixel with sprite pixel/bit to set it on or off
                    *pixel ^= sprite_bit;

                    // Stop drawing this row if hit right edge of screen
                    if (++X_coord >= config.window_width) break;
                }

                // Stop drawing entire sprite if hit bottom edge of screen
                if (++Y_coord >= config.window_height) break;
            }
            break;
        }

        default:
            break; //unimplemented
    }
}

//da main
int main(int argc, char **argv) {
    //Default Usage
    if (argc < 2){
        fprintf(stderr, "Usage: %s <rom_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // initialize emulator config/options
    config_t config = {0};
    if( !set_config_from_args(&config, argc, argv)) exit(EXIT_FAILURE);

    // initialize SDL
    sdl_t sdl = {0};
    if (!init_sdl(&sdl, config)) exit(EXIT_FAILURE);

    //initialize chip8 machine
    chip8_t chip8 = {0};
    const char *rom_name = argv[1];
    if (!init_chip8(&chip8, rom_name)) exit(EXIT_FAILURE);
    //initial screen clear to backroud color
    clear_screen(sdl, config);

    // Main emulator loop 

     while (chip8.state != QUIT) {
        // Handle user inputs 
        // Handle events - required for macOS
        handle_input(&chip8);
        // if chip8 state paused continue;
        if (chip8.state == PAUSED) continue;
        //get_time()
        //emulate chip8 instructions
        emulate_instruction(&chip8, config);

        //get_time() elapsed since last get_time

        //maybe delay approx 60hz/60fps (16.67 ms)
        SDL_Delay(16);

        //update window with changes
        clear_screen(sdl, config);
        update_screen(sdl,config,chip8);
    } 

    //FINAL Clean up functions 
    final_cleanup(sdl);

    exit(EXIT_SUCCESS);
    puts("TESTING ON MAC");
}

