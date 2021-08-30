
// The program brings .ppm image, reads it, and smooth/filter it using one of three differing smoothing techniques, 
// and returns the filtered/smoothed image

// In the terminal: [0-excutable_file] [1-image_to_be_filtered] [2-filter_size] [3-filtering_method]
// Filtering methods: (1) basic 2D convolution using mean, (2) Seperable Filter, (3) Separable and Sliding window Filters

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>


int main(int argc, char *argv[]){
    FILE            *fpt;
    unsigned char   *image;
    unsigned char   *filtered;
    char            header[100];
    int             ROWS, COLS, BYTES;
    int             filter_side, filtering_method,r,c,ri,ci,i;
    struct timespec	tp1,tp2;
    double          *temp, sum;

    // Opening and reading the input image
    fpt=fopen(argv[1], "rb");
    fscanf(fpt, "%s %d %d %d ", header, &ROWS, &COLS, &BYTES);
    image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    fread(image, 1, ROWS*COLS, fpt);
    fclose(fpt);

    // Checking the terminal input
    if (argc != 4){ printf("Error: Not enough arguments, [0-excutable_file] [1-image_to_be_filtered] [2-filter_size] [3-filtering_method] \n");
        exit(0);}

    if ((atoi(argv[2])<0) || (atoi(argv[2])>ROWS) || (atoi(argv[2])>COLS)){
        printf("Error: [2] The filter size should be a number between (0) and the image min width or hight (%d, %d) \n", ROWS,COLS);
        exit(0);}
    
    if ((atoi(argv[3])<0) || (atoi(argv[3])>3)){
        printf("Error: [3] The filtering method should be between 1-3 Filtering methods: (1) basic 2D convolution using mean, (2) Seperable Filter, (3) Separable and Sliding window Filters\n");
        exit(0);}

    if (fpt==NULL){
         printf("Error: [1] Image (%s) was not imported, please check the name and make sure it is in the same directory of the excutable", argv[1]);
         exit(0);}

    // Capturing the terminal inputs
    filter_side = atoi(argv[2])/2;      //the size of the filter square
    filtering_method = atoi(argv[3]);   //the filter method

    //Creating the new filtered image and temp image dynamic memory
    filtered = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    temp=(double *)calloc(ROWS*COLS,sizeof(double));                    // to properly maintain the precision in progress

    // Accuiring the start time
    clock_gettime(CLOCK_REALTIME, &tp1);
    
    // [1] basic 2D convolution using mean  //////////////////////////////////////////
    if (filtering_method == 1){
        for (r=0; r<ROWS; r++)
            for (c=0; c<COLS; c++){
                if (r>=filter_side && r<(ROWS-filter_side) && c>=filter_side && c<(COLS-filter_side)){
                    // Convolution box loop
                    sum = 0;
                    for (ri=-filter_side; ri<=filter_side; ri++){
                        for (ci=-filter_side; ci<=filter_side; ci++){
                            sum += image[(r+ri)*COLS + (c+ci)];
                        }                                         
                    }
                    filtered[r*COLS+c] = round(sum/(atoi(argv[2])*atoi(argv[2])));      // rounding to have consitint result between the versions 
                }
                else {                                                                  // Convolution box with 1 outer-pixel => 0 
                    filtered[r*COLS+c] =0;
                }
            }
        fpt = fopen("01_filtered_2D_convolution.ppm","w");                              // Open new image

        // Accuiring the end time
        clock_gettime(CLOCK_REALTIME,&tp2);
        // report how long it took to smooth
        printf("[01] The smoothing processing time is %ld\n",tp2.tv_nsec-tp1.tv_nsec);
    }

    // [2] Seperable Filter                 //////////////////////////////////////////
    else if (filtering_method ==2){
        // Accuiring the start time
        clock_gettime(CLOCK_REALTIME, &tp1);

       // Rows direction
       for (r=0; r<ROWS; r++)
            for (c=0; c<COLS; c++){  
                if (c<filter_side || c>=(COLS-filter_side)){                            // Convolution box with 1 outer-pixel => 0 
                   filtered[r*COLS+c] =0;
                }
                else {
                    sum = 0;
                    for (ci=-filter_side; ci<=filter_side; ci++){
                        sum += image[(r)*COLS + (c+ci)];
                    }
                    temp[r*COLS+c] = (sum/atoi(argv[2]));  
                }
            }

        // Colums Diretion
        for (c=0; c<COLS; c++)
            for (r=0; r<ROWS; r++){                                                     // Convolution box with 1 outer-pixel => 0
                if (r<filter_side || r>=(ROWS-filter_side)){
                    filtered[r*COLS+c] =0;
                }
                else {
                    sum = 0;
                    for (ri=-filter_side; ri<=filter_side; ri++){
                        sum += temp[(r+ri)*COLS + (c)];
                    }
                    filtered[r*COLS+c] = round(sum/atoi(argv[2]));                      // rounding to have consitint result between the versions 
                }
            }
        fpt = fopen("02_separable_filters.ppm","w");                                    // Open new image

        // Accuiring the end time
        clock_gettime(CLOCK_REALTIME,&tp2);
        // report how long it took to smooth
        printf("[02] Processing time (ns) = %ld\n",tp2.tv_nsec-tp1.tv_nsec);

    }
    
    // [3] Separable and Sliding window Filters     //////////////////////////////////////////
    else if (filtering_method ==3){
        // Accuiring the start time
        clock_gettime(CLOCK_REALTIME, &tp1);
        // Rows direction
        for (r=0; r<ROWS; r++)
            for (c=0; c<COLS; c++){  
                sum = 0;                // 0 at the start of each line
                if (c<filter_side || c>=(COLS-filter_side)){                            // Convolution box with 1 outer-pixel => 0
                   filtered[r*COLS+c] =0;
                }
                else {
                    if (sum ==0){       // at the start of the line? Accuiring the first window
                        for (ci=-filter_side; ci<=filter_side; ci++){
                            sum += image[(r)*COLS + (c+ci)];
                        }
                    }
                    else{               // the line started already
                         temp[r*COLS+c]=sum - image[r*COLS+(c-filter_side-1)] + image[r*COLS+(c+filter_side)];
                    }
                    temp[r*COLS+c] = (sum/atoi(argv[2]));  
                }
            }

        // Colums direction
        for (c=0; c<ROWS; c++)
            for (r=0; r<COLS; r++){  
                sum = 0;                // 0 at the start of each line
                if (r<filter_side || r>=(ROWS-filter_side)){
                   filtered[r*COLS+c] =0;
                }
                else {
                    if (sum ==0){       // at the start of the line? Accuiring the first window
                        for (ri=-filter_side; ri<=filter_side; ri++){
                            sum += temp[(r+ri)*COLS + (c)];
                        }
                    }
                    else{               // the line started already
                         temp[r*COLS+c]=sum - temp[(r-filter_side-1)*COLS+(c)] + temp[(r+filter_side)*COLS+(c)];
                    }
                    filtered[r*COLS+c] = round(sum/atoi(argv[2]));  
                }
            }

        fpt = fopen("03_separable_and_sliding_window_filters.ppm","w");                 // Open new image
        
        // Accuiring the end time
        clock_gettime(CLOCK_REALTIME,&tp2);
        // report how long it took to smooth
        printf("[03] Processing time (ns) = %ld\n",tp2.tv_nsec-tp1.tv_nsec);
    }
    // Writting the output Image
    fprintf(fpt,"%s %d %d 255 ",header, ROWS,COLS);
    fwrite(filtered,COLS*ROWS,sizeof(unsigned char),fpt);
    fclose(fpt);
    printf("Filtered image has been successfully saved in the same directory");
}
