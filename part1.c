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
#define THIEF_WIDTH 17
#define THIEF_HEIGHT 22
#define POLICE_WIDTH 14
#define POLICE_HEIGHT 22
#define START_X 10
#define START_Y 55
#define END_X   70
#define END_Y   10

volatile bool correct = 0;
volatile int pixel_buffer_start; // global variable
short int const black = 0;
int thief_x = 30;
int thief_y = 175;
int thief_dx = 1;
int thief_dy = 1;
int police_dy = 4;
int police_x = 0;
int police_dx = 4;
int police_y = 175;
bool start = true;
bool ingame = false;
bool win = false;
bool loss = false;
bool left = false;
bool p_left = false;
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
void thief_motion();
void police_motion();

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

    //typing paragraph
    char character = 0;
    char* display = "although dr.tallman appears to be a communication instructor, his real job is actually chief cleaner of university of toronto. he can clean the entire campus within two hours - with just a "
                    "mop in hand. in his free time, dr.tallman becomes a delivery guy in uber eats. the competiton between he and olivier trescases is fierce - olivier has a yellow electric car but dr.tallman doesn't."
                    "so what does dr.tallman do? he asks his friend, professor micheal seica for help."
                    " ";
    // char* display = "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
    // "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
    // "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    volatile int * ps2_ptr = (int *)PS2_BASE;
    *ps2_ptr = 0xFF;       //drive the keyboard
    int ps2_data, RVALID;


    //display the start interface every time the game is compiled and loaded onto the board
    if (start){
        //draw start interface
        draw_start();
        //when the user press enter, start the game otherwise no further motion
        while(1){
            char response = 0;
            ps2_data = *ps2_ptr;
            RVALID = ps2_data & 0x8000;
            response = ps2_data & 0xFF;
            if(RVALID && response == 0x5A){
                clear_screen();
                ingame = true;
                break;
            }
        }
    }
    
    while(ingame){

        //if the user wins the game
        if(win){

            win = false;

            //reset character position and paragraph inputs
            thief_x = 30;
            thief_y = 175;
            police_x = 0;
            police_y = 175;
            display = "although dr.tallman appears to be a communication instructor, his real job is actually chief cleaner of university of toronto. he can clean the entire campus within two hours - with just a "
                            "mop in hand. in his free time, dr.tallman becomes a delivery guy in uber eats. the competiton between he and olivier trescases is fierce - olivier has a yellow electric car but dr.tallman doesn't."
                            "so what does dr.tallman do? he asks his friend, professor micheal seica for help."
                            " ";
    //          char* display = "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
    // "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
    // "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
             //clear in game background
            clear_screen();

            //plot win interface
            draw_win(); 
            
            //when the user press enter, restart the game otherwise no further motion
            while(1){
                char response = 0;
                ps2_data = *ps2_ptr;
                RVALID = ps2_data & 0x8000;
                response =ps2_data & 0xFF;
                if(RVALID && response == 0x5A){
                    clear_screen();
                    break;
                }
            }
        }

        //if the user losses the game
        if(loss){

            loss = false;
            //reset character position and paragraph inputs
            thief_x = 30;
            thief_y = 175;
            police_x = 0;
            police_y = 175;
            display = "although dr.tallman appears to be a communication instructor, his real job is actually chief cleaner of university of toronto. he can clean the entire campus within two hours - with just a "
                            "mop in hand. in his free time, dr.tallman becomes a delivery guy in uber eats. the competiton between he and olivier trescases is fierce - olivier has a yellow electric car but dr.tallman doesn't."
                            "so what does dr.tallman do? he asks his friend, professor micheal seica for help."
                            " ";
//  char* display = "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
//     "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
//     "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
             //clear in game background
            clear_screen();

            //plot loss interface
            draw_loss(); 
            
            //when the user press enter, restart the game otherwise no further motion
            while(1){
                char response = 0;
                ps2_data = *ps2_ptr;
                RVALID = ps2_data & 0x8000;
                response =ps2_data & 0xFF;
                if(RVALID && response == 0x5A){
                    clear_screen();
                    break;
                }
            }
        }

        //in game logics
        correct = 0;
        ps2_data = *ps2_ptr;
        *LED = 0;
        plot_char(START_X,START_Y, display);
        RVALID = ps2_data & 0x8000;
        if(RVALID && string_length(display) != 0){
            character = ps2_data & 0xFF;
            character = key_to_ascii(character);

            if(character == *display){
                correct = 1;
                plot_char(START_X + string_length(display) -1, START_Y, " ");
                display++;
            }
        }

        for (int x = 0; x <320; x++){
            for (int y = 0; y < 240; y++){

                if (x >= thief_x && x < thief_x + THIEF_WIDTH && y>= thief_y && y < thief_y + THIEF_HEIGHT && !left
                    && thief_right[(y-thief_y)*17 + (x-thief_x)]!= 0x1dcb){
                    plot_pixel(x, y, thief_right[(y-thief_y)*17 + (x-thief_x)]);
                }
                else if (x >= thief_x && x < thief_x + THIEF_WIDTH && y>= thief_y && y < thief_y + THIEF_HEIGHT && left
                     && thief_left[(y-thief_y)*17 + (x-thief_x)]!= 0x1dcb){
                    plot_pixel(x, y, thief_left[(y-thief_y)*17 + (x-thief_x)]);
                }
                else if (x >= police_x && x < police_x + POLICE_WIDTH && y>= police_y && y < police_y + POLICE_HEIGHT && !p_left
                     && police_right[(y-police_y)*14+(x-police_x)] != 0x0400){
                    plot_pixel(x, y, police_right[(y-police_y)*14+(x-police_x)]);
                }else if (x >= police_x && x < police_x + POLICE_WIDTH && y>= police_y && y < police_y + POLICE_HEIGHT && p_left
                    && police_left[(y-police_y)*14+(x-police_x)] != 0x0400){
                    plot_pixel(x, y, police_left[(y-police_y)*14+(x-police_x)]);
                }
                else{
                    plot_pixel(x, y, in_game[y*320 + x] );
                }
            }
        }

 /*------------------------------------Police Motion------------------------------*/

        if (correct == 1){
           police_motion();
        }
/*------------------------------------Police Motion------------------------------*/

/*------------------------------------Thief Motion------------------------------*/
        thief_motion();
/*------------------------------------Thief Motion------------------------------*/
        wait_for_vsync();


        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
   }

    return 0;
}


void thief_motion(){
	//switch to go upward
	//(50, 175)
    const int corner_x_1 = 67 - THIEF_WIDTH;
    const int corner_y_1 = 175;

	//switch to go right
	//(50, 30)
    const int corner_x_2 = corner_x_1;
    const int corner_y_2 = 30;

	//switch to go downward
	//(100, 30)
    const int corner_x_3 = corner_x_2 + 50;
    const int corner_y_3 = corner_y_2;

	//switch to go right
	//(100, 101)
    const int corner_x_4 = corner_x_3;
    const int corner_y_4 = corner_y_3 + 71;

	//switch to go downward
	//(171, 101)
    const int corner_x_5 = corner_x_4 + 71;
    const int corner_y_5 = corner_y_4;

	//switch to go left
	//(171, 139)
    const int corner_x_6 = corner_x_5;
    const int corner_y_6 = corner_y_5 + 38;

	//switch to go down
	//(104, 139)
    const int corner_x_7 = corner_x_6 - 67;
    const int corner_y_7 = corner_y_6;

	//switch to go right
	//(104, 174)
    const int corner_x_8 = corner_x_7;
    const int corner_y_8 = corner_y_7 + 35;

	//switch to go up (left)
	//(286, 174)
    const int corner_x_9 = corner_x_8 + 181;
    const int corner_y_9 = corner_y_8;

	//siwch to left
	//(286, 128)
    const int corner_x_10 = corner_x_9;
    const int corner_y_10 = corner_y_9 - 46;

	//switch to go up (right)
	//(222, 128)
    const int corner_x_11 = corner_x_10 - 64;
    const int corner_y_11 = corner_y_10;

	//swicth to go right
	//(222, 83)
    const int corner_x_12 = corner_x_11;
    const int corner_y_12 = corner_y_11 - 45;

	//switch to go up (left)
	//(286, 83)
    const int corner_x_13 = corner_x_12 + 64;
    const int corner_y_13 = corner_y_12;

	//switch to go left
	//(286, 37)
    const int corner_x_14 = corner_x_13;
    const int corner_y_14 = corner_y_12 - 46;
	
	//switch to go up
	//(153, 37)
    const int corner_x_15 = corner_x_14 - 134;
    const int corner_y_15 = corner_y_14;

	//reach final destination
    const int destination_x = corner_x_15;
    const int destination_y = 0;


    if(thief_x <= corner_x_1 && thief_y == corner_y_1){
        left = false;
        thief_x += thief_dx;
    }

    else if(thief_x <= corner_x_2 + thief_dx && thief_y >= corner_y_2){
        left = false;
        thief_y -= thief_dy;
    }

    else if(thief_x <= corner_x_3 && thief_y <= corner_y_3){
        left = false;
        thief_x += thief_dx;
    }

    else if(thief_x <= corner_x_4 + thief_dx && thief_y <= corner_y_4){
        left = false;
        thief_y += thief_dy;
    }

    else if(thief_x <= corner_x_5 && thief_y < corner_y_6 && thief_y > corner_y_15){
        left = false;
        thief_x += thief_dx;
    }

    else if(thief_x <= corner_x_6 + thief_dx && thief_y <= corner_y_6 && thief_y >= corner_y_5){
        left = true;
        thief_y += thief_dy;
    }

    else if(thief_x <= corner_x_6 + thief_dx && thief_x >= corner_x_7 && thief_y >= corner_y_7 && thief_y < corner_y_8){
        left = true;
        thief_x -= thief_dx;
    }

    else if(thief_x <= corner_x_8 && thief_y <= corner_y_8){
        left = false;
        thief_y += thief_dy;
    }

    else if(thief_x <= corner_x_9 && thief_y >= corner_y_9){
        left = false;
        thief_x += thief_dx;
    }

    else if(thief_x >= corner_x_10 && thief_y >= corner_y_10){
        left = true;
        thief_y -= thief_dy;
    }

    else if(thief_x >= corner_x_11 && thief_y >= corner_y_11 - thief_dy){
        left = true;
        thief_x -= thief_dx;
    }

    else if(thief_x >= corner_x_12 - thief_dx && thief_y >= corner_y_12){
        left = false;
        thief_y -= thief_dy;
    }

    else if(thief_x <= corner_x_13 && thief_y >= corner_y_13 - thief_dy){
        left = false;
        thief_x += thief_dx;
    }

    else if(thief_x >= corner_x_14 && thief_y >= corner_y_14){
        left = true;
        thief_y -= thief_dy;
    }

    else if(thief_x >= corner_x_15 && thief_y >= corner_y_15 - thief_dy){
        left = true;
        thief_x -= thief_dx;
    }

    else if(thief_x >= destination_x - thief_dx && thief_y + THIEF_HEIGHT >= destination_y){
        left = false;
        thief_y -= thief_dy;
    }
    //game over -- loss
    else if(thief_x >= destination_x - thief_dx && thief_y + THIEF_HEIGHT <= destination_y){
        left = false;
        loss = 1;
    }
}

void police_motion(){

    const int corner_x_1 = 65 - POLICE_WIDTH;
    const int corner_y_1 = 175;

    const int corner_x_2 = corner_x_1;
    const int corner_y_2 = 31;

    const int corner_x_3 = corner_x_2 + 50;
    const int corner_y_3 = corner_y_2;

    const int corner_x_4 = corner_x_3;
    const int corner_y_4 = corner_y_3 + 70;

    const int corner_x_5 = corner_x_4 + 70;
    const int corner_y_5 = corner_y_4;

    const int corner_x_6 = corner_x_5;
    const int corner_y_6 = corner_y_5 + 36;

    const int corner_x_7 = corner_x_6 - 65;
    const int corner_y_7 = corner_y_6;

    const int corner_x_8 = corner_x_7;
    const int corner_y_8 = corner_y_7 + 37;

    const int corner_x_9 = corner_x_8 + 181;
    const int corner_y_9 = corner_y_8;

    const int corner_x_10 = corner_x_9;
    const int corner_y_10 = corner_y_9 - 45;

    const int corner_x_11 = corner_x_10 - 61;
    const int corner_y_11 = corner_y_10;

    const int corner_x_12 = corner_x_11;
    const int corner_y_12 = corner_y_11 - 45;

    const int corner_x_13 = corner_x_12 + 61;
    const int corner_y_13 = corner_y_12;

    const int corner_x_14 = corner_x_13;
    const int corner_y_14 = corner_y_12 - 48;

    const int corner_x_15 = corner_x_14 - 129;
    const int corner_y_15 = corner_y_14;

    const int destination_x = corner_x_15;
    const int destination_y = 0;

    if(police_x <= corner_x_1 && police_y == corner_y_1){
        p_left = false;
        police_x += police_dx;
    }

    else if(police_x <= corner_x_2 + police_dx && police_y >= corner_y_2){
        p_left = false;
        police_y -= police_dy;
    }

    else if(police_x <= corner_x_3 && police_y <= corner_y_3){
        p_left = false;
        police_x += police_dx;
    }

    else if(police_x <= corner_x_4 + police_dx && police_y <= corner_y_4){
        p_left = false;
        police_y += police_dy;
    }

    else if(police_x <= corner_x_5 && police_y < corner_y_6 && police_y > corner_y_15){
        p_left = false;
        police_x += police_dx;
    }

    else if(police_x <= corner_x_6 + police_dx && police_y <= corner_y_6 && police_y >= corner_y_5){
        p_left = true;
        police_y += police_dy;
    }

    else if(police_x <= corner_x_6 + police_dx && police_x >= corner_x_7 && police_y >= corner_y_7 && police_y < corner_y_8){
        p_left = true;
        police_x -= police_dx;
    }

    else if(police_x <= corner_x_8 && police_y <= corner_y_8){
        p_left = false;
        police_y += police_dy;
    }

    else if(police_x <= corner_x_9 && police_y >= corner_y_9){
        p_left = false;
        police_x += police_dx;
    }

    else if(police_x >= corner_x_10 && police_y >= corner_y_10){
        p_left = true;
        police_y -= police_dy;
    }

    else if(police_x >= corner_x_11 && police_y >= corner_y_11 - police_dy){
        p_left = true;
        police_x -= police_dx;
    }

    else if(police_x >= corner_x_12 - police_dx && police_y >= corner_y_12){
        p_left = false;
        police_y -= police_dy;
    }

    else if(police_x <= corner_x_13 && police_y >= corner_y_13 - police_dy){
        p_left = false;
        police_x += police_dx;
    }

    else if(police_x >= corner_x_14 && police_y >= corner_y_14){
        p_left = true;
        police_y -= police_dy;
    }

    else if(police_x >= corner_x_15 && police_y >= corner_y_15 - police_dy){
        p_left = true;
        police_x -= police_dx;
    }

    else if(police_x >= destination_x - police_dx && police_y + POLICE_HEIGHT >= destination_y){
        p_left = false;
        police_y -= police_dy;
    }

    //check win condtion
    for(int i=0; i<=POLICE_WIDTH-6; i++){
        for(int j=0; j<=POLICE_HEIGHT-6; j++){
            for(int k=0; k<=THIEF_WIDTH-6; k++){
                for(int l=0; l<=THIEF_HEIGHT-6;l++){
                    if((police_x + i == thief_x + k) && (police_y + j == thief_y + l)){
                        win = 1;
                        return;
                    }
                }
            }
        }
    }
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
        for(j=0; j<60; j++){
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