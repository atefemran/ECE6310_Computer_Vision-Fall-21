#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){
    FILE            *fpt1;
    FILE            *fpt2;
    char            header[80];
    char            header2[80];
    unsigned char   *image1, *image2, diff;
    int             ROWS, COLS, BYTES;
    int             ROWS2, COLS2, BYTES2;
    int             r,c,count;
    
    // Opening and reading the first image
    fpt1=fopen(argv[1], "rb");
    fscanf(fpt1, "%s %d %d %d ", header, &ROWS, &COLS, &BYTES);
    image1 = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    fread(image1, 1, ROWS*COLS, fpt1);
    fclose(fpt1);

    // Opening and reading the second image
    fpt2=fopen(argv[2], "rb");
    fscanf(fpt2, "%s %d %d %d ", header2, &ROWS2, &COLS2, &BYTES2);
    image2 = (unsigned char *)calloc(ROWS2*COLS2, sizeof(unsigned char));
    fread(image2, 1, ROWS2*COLS2, fpt2);
    fclose(fpt2);

    count=0;

    for (r=0; r<ROWS; r++)
        for (c=0; c<COLS; c++){
            if (image1[r*COLS+c] != image2[r*COLS+c]){
                printf("Pixel (%d,%d) is not identical %d, %d\n", r, c,image1[r*COLS+c], image2[r*COLS+c]);
            }
            else {count++;}   
        }
    if (count == ROWS*COLS){
    printf("Identical! Total number of identical pixels = %d, and total number of pixels in first pic are %d \n", count, (ROWS*COLS));}
}
