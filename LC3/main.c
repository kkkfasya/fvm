/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Kamal Fasya
 * fvm - fasya's virtual machines: Various virtual machines developed by fasya for learning purpose
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
// NOTE: why are we using bitshift op?
#define MEMORY_MAX (1 << 16) // LC-3 has 65,536 memory locations a.k.a 16-bit uint

// each register is 16 bit (2 bytes)
typedef enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,       // program counter
    R_COND,     // conditional flag register
    R_COUNT,    // store amount of register (minus 1 ofc)
} Register;
// XXX: do you name enum plural or singular?
typedef enum {
    OP_ADD,     // add
    OP_LD,      // load
    OP_ST,      // store
    OP_JSR,     // jump register
    OP_AND,     // bitwise and
    OP_LDR,     // load register
    OP_STR,     // store register
    OP_RTI,     // unused
    OP_NOT,     // bitwise not 
    OP_LDI,     // load indirect
    OP_STI,     // store indirect
    OP_JMP,     // jump
    OP_RES,     // reserved (unused)
    OP_LEA,     // load effective address
    OP_TRAP,    // execute trap
} OPCode;

/* according to spec:
 * The condition codes are set, based on whether the result is 
 * negative, zero, or positive. (Pg. 526) 
 * so each time any value is written to a register, we update the condition flag
 * using the value of enums below
*/
typedef enum {
    FL_POS = 1 << 0, // P
    FL_ZRO = 1 << 1, // Z
    FL_NEG = 1 << 2, // N
} Cond_Flag;

uint16_t memory[MEMORY_MAX]; // can store total of 128KB
uint16_t reg[R_COUNT]; // GLOBAL MUTABLE STATE!!! functional programmer bros are doomed

// defining the function here
// i will split it later to separate file if i am in the mood to do so
int read_image(const char *img);
uint16_t mem_read(uint16_t mem);
uint16_t sign_extend(uint16_t x, int bit_count);
void update_flags(uint16_t r);
uint16_t swap16(uint16_t x);


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("[INFO]: fvm-lc3 <image-file>\n");
        exit(2);
    }

    for (int i; i < argc; i++) {
        if (read_image(argv[i])) {
            printf("failed to load image: %s\n", argv[i]);
            exit(1);
        }
    }
    
    // memory start for PC
    // 0x3000 is the default according to spec(?)
    enum {PC_START = 0x3000};
    reg[R_PC] = PC_START;

    int running = 1;
    while (running) {
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op) {
            case OP_ADD: { // must wrap within brace  
                // Destination Register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;

                // First Operand (SR1)
                uint16_t r1 = (instr >> 6) & 0x7;

                // check wheter in immediate mode
                // we take one bit from the left-most side, 1 means negative value, 0 means positive value
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag) {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                } else {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }

                update_flags(r0);
                
                break;
            }
            case OP_AND: {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;
                if (imm_flag) {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] & imm5;
                } else {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(r0);
                break;
            }

            default:
                break;

        }
    }
    
    return 0;
}

uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

void read_image_file(FILE* f) {
    // origin tells us where in memory to place the image
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, f);
    origin = swap16(origin);
}


int read_image(const char *img_path) {
    FILE *f = fopen(img_path, "rb");
    if (!f) {return 0;} // shouldn't it return 1 to be more consistent with unix shits?
    read_image_file(f);
    fclose(f);
    return 1;
}

void update_flags(uint16_t r) {
    uint16_t reg_value = reg[r];
    if (reg_value == 0) reg[R_COND] = FL_ZRO;
    /* a 1 in the left-most bit indicates negative */
    else if (reg_value >> 15) reg[R_COND] = FL_NEG; 
    else reg[R_COND] = FL_POS;
}


uint16_t mem_read(uint16_t mem) {
    return 0;
}

/* useful for immediate mode where we can only have 5 bit value
 * but we need to extend it to 16 bit
 * for positive num, we simply add 0
 * for negative num, we simply add 0 AND 1 at the very left value (e.g 10010 (-2), the one indicate negative value)
 * this is called Sign and Magnitude encoding or... Two's Complement? what's the difference?
 * */
uint16_t sign_extend(uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

