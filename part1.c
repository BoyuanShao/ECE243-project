#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define LED ((volatile long *) 0xFF200000)
#define PS2_BASE              0xFF200100
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define FPGA_CHAR_BASE        0xC9000000
//constant expression
#define N 1
#define THIEF_WIDTH 17
#define THIEF_HEIGHT 22
#define POLICE_WIDTH 14
#define POLICE_HEIGHT 22
#define START_X 10
#define START_Y 54.8
#define END_X   70
#define END_Y   10

volatile bool correct = 0;
volatile int pixel_buffer_start; // global variable
short int const black = 0;
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_line(int x_1, int y_1, int x_2, int y_2, short int line_color);
void swap(int *a, int *b);
void plot_char(int x, int y, char* c);
void clear_char(int x_start, int y_start, int x_end, int y_end);
int string_length(char *c);
char key_to_ascii(char key);
void wait_for_vsync();
void draw_start();
void draw_loss();
void draw_win();

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
    pixel_buffer_start = *pixel_ctrl_ptr;
    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
    // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();

    char character = 0;
    char* display = "dr.tallman is a fantastic cleaner. he can clean the entire myhal centre within two minutes. dr.tallman is a fantastic cleaner. he can clean the entire myhal centre within two minutes.";
    volatile int * ps2_ptr = (int *)PS2_BASE;
    *ps2_ptr = 0xFF;       //drive the keyboard
    int ps2_data, RVALID;

    int thief_x = 0;
    int thief_y = 175.5;
    int thief_dx = 1;
    int thief_dy = 1;
    int police_dy = 1;
    int police_x = 0;
    int police_dx = 4;
    int police_y = 0;
    bool start = true;
    bool ingame = false;
    bool win = false;
    bool loss = false;

    if (start){
        draw_start();
        plot_char(30, 20,"Press s to start the game");
        while(1){
            char response = 0;
            ps2_data = *ps2_ptr;
            RVALID = ps2_data & 0x8000;
            response = key_to_ascii(ps2_data & 0xFF);
            if(RVALID && response == 's'){
                clear_screen();
                ingame = true;
                break;
            }
        }
    }
    
    while(ingame){
        
        if(police_x != 0 && thief_x != 0 && police_x == thief_x && police_y == thief_y){
            *LED = 0b111110000;
            clear_screen();
            draw_win();
            //Need to implement restart
            break;
        }
        if(thief_x >=148 && thief_x+THIEF_WIDTH <= 172 && thief_y <= 0-THIEF_HEIGHT){
            *LED = 0b000001111;
            clear_screen();
            draw_loss();
            //Need to implement restart
            break;
        }

        correct = 0;
        ps2_data = *ps2_ptr;
        *LED = 0;
        plot_char(START_X,START_Y, display);
        RVALID = ps2_data & 0x8000;
        if(RVALID && string_length(display) != 0){
            character = ps2_data & 0xFF;
            character = key_to_ascii(character);

            if (character == 'w'){
                thief_y -= 1;
            }
            if (character == 's'){
                thief_y += 1;
            }
            if (character == 'a'){
                thief_x -= 1;
            }
            if (character == 'd'){
                thief_x += 1;
            }

            if(character == *display){
                correct = 1;
                int length = string_length(display);
                plot_char(START_X + string_length(display) -1, START_Y, " ");
                display++;
            }
        }

        for (int x = 0; x <320; x++){
            for (int y = 0; y < 240; y++){

                if (x >= thief_x && x < thief_x + THIEF_WIDTH && y>= thief_y && y < thief_y + THIEF_HEIGHT && thief_dx > 0
                    && thief_right[(y-thief_y)*17 + (x-thief_x)]!= 0x1dcb){
                    plot_pixel(x, y, thief_right[(y-thief_y)*17 + (x-thief_x)]);
                }
                else if (x >= thief_x && x < thief_x + THIEF_WIDTH && y>= thief_y && y < thief_y + THIEF_HEIGHT && thief_dx < 0
                     && thief_right[(y-thief_y)*17 + (x-thief_x)]!= 0x1dcb){
                    plot_pixel(x, y, thief_left[(y-thief_y)*17 + (x-thief_x)]);
                }
                else if (x >= police_x && x < police_x + POLICE_WIDTH && y>= police_y && y < police_y + POLICE_HEIGHT && police_dx > 0
                     && police_right[(y-police_y)*14+(x-police_x)] != 0x0400){
                    plot_pixel(x, y, police_right[(y-police_y)*14+(x-police_x)]);
                }else if (x >= police_x && x < police_x + POLICE_WIDTH && y>= police_y && y < police_y + POLICE_HEIGHT && police_dx < 0
                    && police_right[(y-police_y)*14+(x-police_x)] != 0x0400){
                    plot_pixel(x, y, police_left[(y-police_y)*14+(x-police_x)]);
                }
                else{
                    plot_pixel(x, y, in_game[y*320 + x] );
                }
            }
        }      

        //thief_x+= thief_dx;

        if (correct == 1){
            police_x += police_dx;
        }

        if (thief_x >= 319-THIEF_WIDTH){
            thief_dx = -abs(thief_dx);
        }else if (thief_x <= 0 + THIEF_WIDTH){
            thief_dx = abs(thief_dx);
        }

        if (police_x >= 319-POLICE_WIDTH){
            police_dx = -abs(police_dx);
        }else if (police_x <= 0 + POLICE_WIDTH){
            police_dx = abs(police_dx);
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
   }

    return 0;
}


void draw_start(){
    
    for (int x = 0; x <320; x++){
        for (int y = 0; y < 240; y++){
           plot_pixel(x, y, start_interface[(y)*320 + (x)]);
        }
    }
    wait_for_vsync();
}

void draw_win(){
    
    for (int x = 0; x <320; x++){
        for (int y = 0; y < 240; y++){
           plot_pixel(x, y, win_interface[(y)*320 + (x)]);
        }
    }
    wait_for_vsync();
}


void draw_loss(){
    
    for (int x = 0; x <320; x++){
        for (int y = 0; y < 240; y++){
           plot_pixel(x, y, loss_interface[(y)*320 + (x)]);
        }
    }
    wait_for_vsync();
}

// code not shown for clear_screen() and draw_line() subroutines

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void plot_char(int x, int y, char* c){
    volatile char * character_buffer = (char*) FPGA_CHAR_BASE;

    while(*c){
        if(x == END_X) {
            return;
        }
        int offset = (y<<7) + x;
        *(character_buffer + offset) = *c;
        c++;

        offset = (((y) << 7 ) + (x++));

    }
}
void clear_char(int x_start, int y_start, int x_end, int y_end){
    int i;
    int j;
    for(i = x_start; i <= x_end; i++){
        for(j = y_start; j <= y_end; j++){
            plot_char(i,j, " ");
        }
    }
}

void clear_screen(){
    int i;
    int j;
    for( i=0; i<320; i++){
        for( j=0; j<240;j++){
            plot_pixel(i,j,black);
        }
    }
    for( i=0; i<80; i++){
        for( j=0; j<60; j++){
            plot_char(i,j," ");
        }
    }
}

void draw_line(int x_1, int y_1, int x_2, int y_2, short int line_color){
    bool is_steep = fabs((double) y_2 - y_1) > fabs((double) x_2 - x_1);
    if(is_steep){
        swap(&x_1,&y_1);
        swap(&x_2,&y_2);
    }
    if(x_1 > x_2){
        swap(&x_1,&x_2);
        swap(&y_1,&y_2);
    }
    assert(x_1<x_2);
    int x_diff = x_2 - x_1;
    int y_diff = fabs((double) y_2 - y_1);
    int error = -(x_diff / 2);
    int y = y_1;
    int y_step = (y_1 < y_2) ? 1 : (-1);
    int x;
    for(x = x_1; x <= x_2; x++){
        if(is_steep){
            plot_pixel(y,x,line_color);
        }
        else{
            plot_pixel(x,y,line_color);
        }
        error += y_diff;
        if(error >= 0){
            y += y_step;
            error -= x_diff;
        }
    }

}

void swap(int *a, int *b){
    int temp = 0;
    temp = *a;
    *a = *b;
    *b = temp;
}

int string_length(char *c){
    int counter = 0;
    while(*c) {
        counter++;
        c++;
    }
    return counter;
}

char key_to_ascii(char key){

    switch(key){
        case 0x1C:
            return 'a';
        case 0x32:
            return 'b';
        case 0x21:
            return 'c';
        case 0x23:
            return 'd';
        case 0x24:
            return 'e';
        case 0x2B:
            return 'f';
        case 0x34:
            return 'g';
        case 0x33:
            return 'h';
        case 0x43:
            return 'i';
        case 0x3B:
            return 'j';
        case 0x42:
            return 'k';
        case 0x4B:
            return 'l';
        case 0x3A:
            return 'm';
        case 0x31:
            return 'n';
        case 0x44:
            return 'o';
        case 0x4D:
            return 'p';
        case 0x15:
            return 'q';
        case 0x2D:
            return 'r';
        case 0x1B:
            return 's';
        case 0x2C:
            return 't';
        case 0x3C:
            return 'u';
        case 0x2A:
            return 'v';
        case 0x1D:
            return 'w';
        case 0x22:
            return 'x';
        case 0x35:
            return 'y';
        case 0x1A:
            return 'z';
        case 0x52:
            return '\'';
        case 0x4E:
            return '-';
        case 0x55:
            return '=';
        case 0x29:
            return ' ';
        case 0x41:
            return ',';
        case 0x49:
            return '.';
        default:
            return 0;
    }

}

void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    register int status;
    //start the synchronous
    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);
    while ((status & 0x0001) != 0){
        status = *(pixel_ctrl_ptr + 3);
    }
}