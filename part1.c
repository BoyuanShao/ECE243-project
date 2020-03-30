#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#define LED ((volatile long *) 0xFF200000)
#define PS2_BASE              0xFF200100
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define FPGA_CHAR_BASE        0xC9000000
//constant expression
#define N 1
#define THIEF_WIDTH 17
#define THIEF_HEIGHT 22
#define POLICE_WIDTH 24
#define POLICE_HEIGHT 25
#define START_X 10
#define START_Y 10
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

int police_right[] = {
// 'output-onlinepngtools (2)', 24x25px
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0041, 0x0021, 0x0020, 0x0020, 0x0020, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6416, 0x4311, 0x3ad0, 0x3acf, 0x3aaf, 0x2a4d, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x08a3, 0x6416,
        0x5373, 0x4311, 0x4311, 0x73cf, 0x6bae, 0x2a4d, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x08a3, 0x6416, 0x5373, 0x4311, 0x9cd1, 0xdea5, 0xd664, 0x840e, 0x326e, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x29a9, 0x5373, 0x5394,
        0x4332, 0x94b1, 0xe6e3, 0xf6a0, 0xf6a0, 0xe6a2, 0x5b0b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1907, 0x320e, 0x29cd, 0x218c, 0x528b, 0x83a3, 0x8362, 0x8362, 0x7ba3, 0x39a8, 0x0003,
        0x0003, 0x0003, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0843, 0x1866, 0x1866,
        0x2907, 0x2907, 0x2907, 0x2907, 0x2907, 0x2907, 0x1865, 0x1045, 0x0805, 0x0005, 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4942, 0x9263, 0x8223, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf64c, 0x8223, 0x69a1,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5162, 0xa2a4, 0x9264,
        0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5162, 0xa2a4, 0x9264, 0xf66c, 0xf66c, 0x0840, 0xac68, 0xf66c, 0xee2b, 0x0000, 0xf66c,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7b26, 0xf64c, 0xf64c,
        0xf66c, 0xf66c, 0xee2c, 0xf66c, 0xf66c, 0xf66c, 0xee2c, 0xf66c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7b24, 0xf629, 0xf629, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5222, 0xb485, 0xf5e7,
        0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xac88, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x20c0, 0xdd65, 0xede8, 0xede8, 0xedc7, 0xede8, 0xede8, 0xede8, 0xe5a7, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x6416, 0x751a, 0x00a7, 0x4b94, 0x5c36, 0x2a4d, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x08a3, 0x6416, 0x5373, 0x751a, 0x218c, 0xbce3, 0xc544, 0x3acf, 0x326e, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x08a3, 0x6416,
        0x5373, 0x4311, 0x216b, 0x3a90, 0x42f1, 0x3acf, 0x326e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x08a3, 0x6416, 0x5373, 0x4311, 0x42d1, 0x4311, 0x42f1, 0x3acf, 0x326e, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x18c2, 0xad31,
        0x42d1, 0x322e, 0x322e, 0x322e, 0x2a0d, 0x29ec, 0x946d, 0x1082, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x20c1, 0xd58a, 0x29cd, 0x216b, 0x192a, 0x194b, 0x192a, 0x10e7, 0xd58a, 0x2124,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x216b, 0x0000, 0x10e8, 0x10e8, 0x0000, 0x0000, 0x2124, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

int police_left[] = {
        // 'police flip good', 24x25px
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0020, 0x0020, 0x0020, 0x0020, 0x0021, 0x0041, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x324d, 0x3aaf, 0x3acf, 0x3acf, 0x4311, 0x6416,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x324d, 0x6b8e, 0x73cf, 0x4311, 0x4311, 0x5373, 0x6416, 0x08a3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x326e, 0x7bee, 0xd644, 0xde85, 0x94d0, 0x4311, 0x5373,
        0x6416, 0x08a3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x5b0b, 0xe6a2, 0xf680, 0xf6a0, 0xe6e3, 0x9491, 0x4b32, 0x5393, 0x5373, 0x29a9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0823, 0x0823, 0x0823, 0x39c8, 0x83c3, 0x8383, 0x8383, 0x83c3, 0x52ab, 0x29ac,
        0x31ed, 0x3a2e, 0x1907, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0022, 0x0845, 0x0845,
        0x1886, 0x20a6, 0x3127, 0x3127, 0x3127, 0x3127, 0x3127, 0x3127, 0x20a6, 0x20a6, 0x1043, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x69c2, 0x8224, 0xf64c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c,
        0x8223, 0x9264, 0x4942, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0x9284, 0xa2a5, 0x5162, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xf66c, 0x0000, 0xee2b, 0xf66c, 0xac88, 0x0860, 0xf66c, 0xf66c,
        0x9264, 0xa2a5, 0x5162, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xf66c, 0xee2c, 0xf66c, 0xf66c, 0xf64c, 0xee2c, 0xf66c, 0xf66c, 0xf64c, 0xf64c, 0x7b26, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c, 0xf66c,
        0xf609, 0xf609, 0x7b04, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xac88, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf64b, 0xf5e7, 0xb465, 0x5222, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xe5a7, 0xedc8, 0xede8, 0xede8, 0xedc8, 0xede8, 0xede8,
        0xdd66, 0x20c0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x324d, 0x5c36, 0x4b94, 0x08c7, 0x751a, 0x6416, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x328e, 0x3acf, 0xc544, 0xbce3, 0x29ac, 0x751a, 0x5373,
        0x6416, 0x08a3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x328e, 0x3acf, 0x42f1, 0x3a8f, 0x218b, 0x4311, 0x5373, 0x6416, 0x08a3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x328e, 0x3acf, 0x42f1, 0x4311, 0x42f1, 0x4311, 0x5373,
        0x6416, 0x08a3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x10a2, 0x946d, 0x2a0c, 0x322d, 0x324e, 0x324e, 0x324e, 0x42d1, 0xad31, 0x18c2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2945, 0xd58a, 0x1907, 0x194a, 0x216b, 0x214a, 0x218b, 0x29cd,
        0xd58a, 0x20c1, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x2945, 0x0000, 0x0000, 0x1908, 0x1908, 0x0000, 0x218b, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

int thief_right[] = {
        // 'output-onlinepngtools (6)', 17x22px
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xabe6, 0x9385, 0x9345, 0x8b25, 0x7ac4, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xabc6, 0x9b86, 0x9b86, 0x9366, 0x9365, 0x8b45, 0x8b25, 0x8305, 0x7aa4, 0x7aa4, 0x1080,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0820, 0x3142, 0xa3a6, 0x9365, 0x9365, 0x9365, 0x9365, 0x8b45, 0x8b25, 0x8305, 0x7aa4, 0x72a4,
        0x1060, 0x0000, 0x0000, 0x0000, 0x0000, 0x18a1, 0xa3c6, 0x9b65, 0x9365, 0x9365, 0x8b45, 0x9345, 0x8b45, 0x8b25, 0x8305, 0x7aa4,
        0x72a4, 0x1060, 0x0000, 0x0000, 0x0000, 0x0000, 0x2020, 0xb0c3, 0x98c3, 0x98c3, 0x98c3, 0x98e3, 0x90c3, 0x90c3, 0x88c3, 0x88a3,
        0x78a2, 0x7882, 0x1000, 0x0000, 0x0000, 0xabe6, 0x9b66, 0x9366, 0x9365, 0x9345, 0x8b45, 0x8b25, 0x8325, 0x8305, 0x8305, 0x82e5,
        0x7ae5, 0x7ac4, 0x72a4, 0x7aa4, 0x7aa5, 0x7aa4, 0x18a1, 0x1881, 0x20e1, 0x5a45, 0x5225, 0xe5eb, 0xdd89, 0xdd26, 0xd526, 0xd526,
        0xd526, 0xd526, 0xd526, 0xd526, 0x3141, 0x1060, 0x1060, 0x0000, 0x0000, 0x0861, 0x5205, 0x49e5, 0xf66c, 0xee0a, 0xeda6, 0xdd26,
        0xeda6, 0xeda6, 0xeda6, 0xdd26, 0xeda6, 0x2901, 0x0000, 0x0000, 0x0000, 0x0000, 0x0841, 0x49e5, 0x41a4, 0xf66c, 0xf64c, 0xf66c,
        0x0000, 0xf66c, 0xf64c, 0xf66c, 0x0000, 0xf66c, 0x2922, 0x0000, 0x0000, 0xf920, 0xe900, 0xea02, 0xf66c, 0xf920, 0xf920, 0xf120,
        0xf120, 0xf100, 0xe900, 0xd900, 0xd0e0, 0xc0c0, 0xb0c0, 0x2020, 0x0000, 0x0000, 0x7080, 0xd0e0, 0xd9e1, 0xee09, 0xe324, 0xf920,
        0xf120, 0xf100, 0xe900, 0xe100, 0xd0e0, 0xc8e0, 0xc0c0, 0xb0c0, 0x2020, 0x0000, 0x0000, 0x0000, 0xb8c0, 0xc9a1, 0xf5e7, 0xcd07,
        0xf920, 0xf120, 0xe900, 0xe100, 0xd8e0, 0xd0e0, 0xc8c0, 0xb8c0, 0xb8c0, 0x2020, 0x0000, 0x0000, 0xd8e0, 0x0800, 0x0861, 0x4a05,
        0xcd07, 0xcd07, 0xdb64, 0xf120, 0xe100, 0xd0e0, 0xc8e0, 0xc8e0, 0xc0c0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x6b90, 0x6b70, 0x634f, 0x5b0e, 0xd0e0, 0xc8c0, 0xc8c0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x7c53, 0x9346, 0x8b26, 0x7ae5, 0x6370, 0x62ed, 0x6a87, 0x7243, 0x5aed, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x8c95, 0x9345, 0x8b25, 0x82e4, 0x6b90, 0x5b2e, 0x6ac8, 0x7284, 0x634f, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x7412, 0x8304, 0x8305, 0x82e4, 0x73d1, 0x634f, 0x72e8, 0x7aa4, 0x6bb1, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6223, 0x8325, 0x8305, 0x82e4, 0x73d1, 0x73d1, 0x7b29, 0x7ae5, 0x5a02, 0xc638, 0xce79,
        0xd69a, 0xb5b6, 0x0000, 0x0000, 0x0000, 0x0000, 0x6223, 0x4aef, 0x4acf, 0x4a8e, 0x428e, 0x426d, 0x424c, 0x3a4c, 0x51c2, 0xad55,
        0x18e3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x20c1, 0x18e5, 0x29a8, 0x4aae, 0x18c4, 0x4a8e, 0x2967, 0x10c4, 0x1880,
        0x39c7, 0x0841, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1905, 0x4aaf, 0x0000, 0x4a8e, 0x18e5, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x20c1, 0x6223, 0x0000, 0x6223, 0x20c1,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

int thief_left[] = {
        // 'fliped good', 17x22px
        0x0000, 0x0000, 0x0000, 0x0000, 0x7ac4, 0x8b25, 0x9345, 0x9365, 0xabe6, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x1060, 0x7aa4, 0x72a4, 0x82e4, 0x8b05, 0x8b25, 0x9345, 0x9366, 0x9b86, 0x9b86, 0xabc6, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x1060, 0x72a4, 0x72a4, 0x82e4, 0x8b05, 0x8b25, 0x9345, 0x9345, 0x9365, 0x9365, 0xa386, 0x3142,
        0x0820, 0x0000, 0x0000, 0x0000, 0x0000, 0x1060, 0x72a3, 0x72a4, 0x8304, 0x8b05, 0x8b45, 0x9345, 0x8b45, 0x9365, 0x9365, 0x9b65,
        0xa3c6, 0x18a1, 0x0000, 0x0000, 0x0000, 0x0000, 0x1000, 0x7882, 0x7882, 0x88a2, 0x88a3, 0x90c3, 0x90c3, 0x98c3, 0x98c3, 0x98c3,
        0x98c3, 0xb0c3, 0x2020, 0x0000, 0x0000, 0x72a4, 0x7aa4, 0x7aa4, 0x72a4, 0x7ac4, 0x7ae4, 0x82e4, 0x82e4, 0x8305, 0x8b05, 0x8b25,
        0x8b25, 0x9345, 0x9345, 0x9365, 0x9b65, 0xabe6, 0x1060, 0x1060, 0x3141, 0xd505, 0xd505, 0xd525, 0xd526, 0xd526, 0xd526, 0xd526,
        0xdd89, 0xe5eb, 0x5225, 0x5a45, 0x20e1, 0x1880, 0x18a1, 0x0000, 0x0000, 0x2901, 0xeda6, 0xd525, 0xeda6, 0xeda6, 0xeda6, 0xd525,
        0xeda6, 0xee0a, 0xf66c, 0x49c5, 0x5205, 0x0861, 0x0000, 0x0000, 0x0000, 0x0000, 0x2922, 0xf66c, 0x0000, 0xf66c, 0xf64c, 0xf66c,
        0x0000, 0xf66c, 0xf64c, 0xf66c, 0x41a4, 0x49e5, 0x0841, 0x0000, 0x0000, 0x0000, 0x0000, 0x2020, 0xb0c0, 0xc0c0, 0xd0e0, 0xd8e0,
        0xe100, 0xf100, 0xf100, 0xf100, 0xf920, 0xf900, 0xf66c, 0xe9e2, 0xe900, 0xf920, 0x0000, 0x0000, 0x2020, 0xb0a0, 0xc0c0, 0xc8c0,
        0xd0e0, 0xe0e0, 0xe8e0, 0xf100, 0xf100, 0xf900, 0xe304, 0xee09, 0xd1c1, 0xd0e0, 0x7080, 0x0000, 0x0000, 0x2020, 0xb8a0, 0xb8c0,
        0xc8c0, 0xd0e0, 0xd8e0, 0xe0e0, 0xe900, 0xf100, 0xf900, 0xcd07, 0xede7, 0xc9a1, 0xb8c0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0xc0c0, 0xc8e0, 0xc8e0, 0xd0e0, 0xd8e0, 0xf100, 0xdb64, 0xcd07, 0xcd07, 0x4a05, 0x0861, 0x0800, 0xd8e0, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0xc8c0, 0xc8c0, 0xd0e0, 0x5aee, 0x634f, 0x6370, 0x6b90, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x5aed, 0x7243, 0x6a87, 0x62ed, 0x6370, 0x7ae5, 0x8b26, 0x9345, 0x7c53, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x634f, 0x6a83, 0x6ac8, 0x5b0e, 0x6b70, 0x82e4, 0x8b05, 0x9324, 0x8c95, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x6bb1, 0x7aa4, 0x6ae8, 0x5b2e, 0x6bb1, 0x82e4, 0x8304, 0x8304, 0x7412, 0x0000, 0x0000, 0x0000,
        0x0000, 0xb5b6, 0xd69a, 0xce79, 0xc638, 0x5a02, 0x7ae4, 0x7b29, 0x73b1, 0x73d2, 0x82e4, 0x8305, 0x8305, 0x6222, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x18e3, 0xa535, 0x51c2, 0x3a4c, 0x3a4c, 0x426d, 0x428d, 0x4a8e, 0x4ace, 0x4aef, 0x6222, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0841, 0x39c7, 0x1880, 0x10a4, 0x2967, 0x428e, 0x10c4, 0x4aae, 0x2988, 0x18e5, 0x20a0,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1905, 0x4a8e, 0x0000, 0x4aaf, 0x1905, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x20c1, 0x6222, 0x0000, 0x6222, 0x20c1,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};


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
    char* display = "dr.tallman is a fantastic cleaner. he can clean the entire myhal centre within two minutes.";
    volatile int * ps2_ptr = (int *)PS2_BASE;
    *ps2_ptr = 0xFF;       //drive the keyboard
    int ps2_data, RVALID;

    int x = 0;
    int y = 0;

    int thief_x = 0;
    int thief_y = 2;
    int thief_dx = 10;
    int police_x = 0;
    int police_dx = 15;
    int police_y = 0;
    bool blink = true;

    while(1){
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
                int length = string_length(display);
                plot_char(START_X + string_length(display) -1, START_Y, " ");
                display++;
            }
        }

        for (int x = 0; x <320; x++){
            for (int y = 0; y < 240; y++){

                if (x >= thief_x && x < thief_x + THIEF_WIDTH && y>= thief_y && y < thief_y + THIEF_HEIGHT && thief_dx > 0){
                    plot_pixel(x, y, thief_right[(y-thief_y)*17 + (x-thief_x)]);
                }
                else if (x >= thief_x && x < thief_x + THIEF_WIDTH && y>= thief_y && y < thief_y + THIEF_HEIGHT && thief_dx < 0){
                    plot_pixel(x, y, thief_left[(y-thief_y)*17 + (x-thief_x)]);
                }
                else if (x >= police_x && x < police_x + POLICE_WIDTH && y>= police_y && y < police_y + POLICE_HEIGHT && police_dx > 0){
                    plot_pixel(x, y, police_right[(y-police_y)*24+(x-police_x)]);
                }else if (x >= police_x && x < police_x + POLICE_WIDTH && y>= police_y && y < police_y + POLICE_HEIGHT && police_dx < 0){
                    plot_pixel(x, y, police_left[(y-police_y)*24+(x-police_x)]);
                }else if (x == 32 && y >= 40 && y <= 45){
                    //if (blink){
                    plot_pixel(x, y, 0XFFFF);
                    //}else{
                    //plot_pixel(x, y, 0X0000);
                    //	blink = true;
                    //}

                }
                else{
                    plot_pixel(x, y, 0X0000);
                }
            }
        }

        thief_x+= thief_dx;

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

        //bool blink = false;

   }

    return 0;
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