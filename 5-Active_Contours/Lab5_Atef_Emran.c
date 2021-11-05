
////////////////////////////////////////////////////////
/////              Lab 5: Active Contours          /////
///////                 Atef Emran              ////////
////////////////////////////////////////////////////////

// Usage:  excutable [input_image.ppm] [points.txt] 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))

// Tune the below paramters 
# define    window 7               // energy calculation window 
# define    weight_1 1             // internal engergy (square distance between two consctive points) weight
# define    weight_2 1             // internal engergy (square variation in distance with the average distance) weight
# define    weight_3 0.7           // external energy weight 

int main(int argc, char *argv[])

{
    FILE	        *fpt;
    int	            px[100],py[100], energy[19*19], i, total_points, x, y, move_x, move_y;
    int             cx, cy, move_point_x, move_point_y, r, c, r2, c2;
    char            header[80];
    int             COLS, ROWS, BYTES;
    unsigned char   *image, *intial_image, *sobel_output, *final_image; 
    double          distance_sqr;
    double          *Gx_filtered, *Gy_filtered, *sobel_filtered, v, average, *energy_window, min_energy;
    double          ie_1[window*window], ie_2[window*window], ee[window*window];
    double          ie_1_norm[window*window], ie_2_norm[window*window], ee_norm[window*window];
    int             min_c, min_r, max, min, m, w;
    int             ie_1_max, ie_1_min, ie_2_max, ie_2_min, ee_max, ee_min;

// [0] Reading the input files from the argument line   //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // [0.1] read contour points file 
    if ((fpt=fopen(argv[2],"r")) == NULL)
    {
        printf("Unable to open %s for reading\n",argv[2]);
        exit(0);
    }
    total_points=0;
    while (1)
    {
        i=fscanf(fpt,"%d %d",&px[total_points],&py[total_points]);
        if (i != 2)
            break;
        total_points++;
        if (total_points > 100)
            break;
    }
    fclose(fpt);

    // [0.2] read the input image
    fpt=fopen(argv[1],"rb");
    fscanf(fpt, "%s %d %d %d ", header, &COLS, &ROWS, &BYTES);  
    image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    final_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));

    fread(image, 1, ROWS*COLS, fpt);
    fclose(fpt); 

// [1] drawing the contour points //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    intial_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    for (i = 0; i < ROWS*COLS; i++) 
    {
       intial_image[i] = image[i];
    }

    for (i = 0; i < total_points; i++)
    {
        for (int r = -3; r <=3; r++)
        {
            intial_image[(py[i]+r)*COLS + (px[i])] = 0;
        }
        for (int c = -3; c <= 3; c++)
        {
            intial_image[(py[i])*COLS + (px[i]+c)] = 0;
        }
    }

    fpt = fopen("intial_cont_image.ppm","wb");
    fprintf(fpt,"%s %d %d 255 ",header,COLS, ROWS);
    fwrite(intial_image,COLS*ROWS,sizeof(unsigned char),fpt);
    fclose(fpt);

// [2] Sobel Filter  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int Gx[9] = {1, 0, -1, 2, 0, -2, 1, 0, -1};
    int Gy[9] = {1, 2, 1, 0, 0, 0, -1, -2, -1};

    Gx_filtered = (double *)calloc(ROWS*COLS, sizeof(double));
    for (r = 0; r < ROWS; r++)
    {
    for (c = 0; c < COLS; c++)
        {
            v = 0.0;
            if (r < 1 || r>=(ROWS-1) || c < 1 || c>=(COLS-1))
            {
                Gx_filtered[r*COLS+c] = 0.0;
            }
            else
            {
                for (r2 = -1; r2 <= 1; r2++)
                {
                    for (c2 = -1; c2 <= 1; c2++)
                    {
                        v += (image[(r+r2)*COLS+(c+c2)]*Gx[(r2+1)*3+(c2+1)]);
                    }
                }
            Gx_filtered[r*COLS+c] = v;
            }
        }
    }
    Gy_filtered = (double *)calloc(ROWS*COLS, sizeof(double));
    for (r = 0; r < ROWS; r++)
    {
    for (c = 0; c < COLS; c++)
    {
        v = 0.0;
        if (r < 1 || r>=(ROWS-1) || c < 1 || c>=(COLS-1))
        {
            Gy_filtered[r*COLS+c] = 0.0;
        }
        else
        {
            for (r2 = -1; r2 <= 1; r2++)
            {
                for (c2 = -1; c2 <= 1; c2++)
                {
                    v += (image[(r+r2)*COLS+(c+c2)]*Gy[(r2+1)*3+(c2+1)]);
                }
            }
            Gy_filtered[r*COLS+c] = v;
        }
    }
    }

    sobel_filtered = (double *)calloc(ROWS*COLS, sizeof(double));
    for (r = 0; r < ROWS; r++)
    {
    for (c = 0; c < COLS; c++)
    {
        sobel_filtered[r*COLS+c] = sqrt(pow(Gx_filtered[r*COLS+c], 2) + pow(Gy_filtered[r*COLS+c], 2));
    }
    }

    // [2.1] Sobel filter : Normalizing this for output
    max = 0; min = 999999999;
    for (int j = 0; j < ROWS*COLS; j++)
    {
        if (sobel_filtered[j] > max) {max = sobel_filtered[j];}
        if (sobel_filtered[j] < min) {min = sobel_filtered[j];}
    }
    float factor = max - min; float feed = 0;
    sobel_output = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    for (r = 0; r < ROWS; r++)
    {
    for (c = 0; c < COLS; c++)
    {
        if (r<1 || c<1)
        {
            sobel_output[r*COLS+c] = 0;
        }
        else{
            feed = (255*(sobel_filtered[r*COLS+c]-min))/(factor);
            int f = round(feed); 
            sobel_output[r*COLS+c] = f;
        }
    }
    }

    fpt=fopen("sobel_filtered.ppm","wb");
    fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
    fwrite(sobel_output, sizeof(unsigned char), ROWS*COLS, fpt);
    fclose(fpt);

// [3] Excuting the active contour operations ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    energy_window = (double *)calloc(window*window, sizeof(double));


    for (int counter=0; counter<30; counter ++)
    {

    // loop through the contour points   
    for (i=0; i<total_points; i++)
    {
        // [3.0] average distance between all contour points
        average = 0;
        for (m=0; m<total_points; m++)
        {
        if ((m+1)>=total_points)
        {
            average += sqrt(pow(px[m]-px[0], 2) + pow(py[m]-py[0], 2));
        }
        else
        {
            average += sqrt(pow(px[m]-px[m+1], 2) + pow(py[m]-py[m+1], 2));
        }
        }
        average = average / total_points;

        // [3.1] calculating the energies pixel by pixel in the window surrounding the contour point
        for (r = -window/2; r<=window/2; r++)
        {
        for (c = -window/2; c<=window/2; c++)
        {
            if ((i+1)>=total_points)
            {
                ie_1[(r+(window/2))*window + (c+(window/2))] = (pow(px[0]-(px[i]+c), 2) + pow(py[0]-(py[i]+r), 2)); 
            }
            else
            {
                ie_1[(r+(window/2))*window + (c+(window/2))] = (pow(px[i+1]-(px[i]+c), 2) + pow(py[i+1]-(py[i]+r), 2));      
            }
        
        ie_2[(r+(window/2))*window + (c+(window/2))] = pow(sqrt(ie_1[(r+(window/2))*window + (c+(window/2))])-average, 2);
        ee[(r+(window/2))*window + (c+(window/2))] = pow(sobel_output[(py[i]+r)*COLS + (px[i]+c)], 2);

        }
        }

        // [3.2] Normalizaing the energies values
        ie_1_max = ie_2_max = ee_max = -9999999;
        ie_1_min = ie_2_min = ee_min = 9999999;

        for (int j = 0; j < window*window; j++)
        {
            if (ie_1[j] > ie_1_max) {ie_1_max = ie_1[j];}
            if (ie_1[j] < ie_1_min) {ie_1_min = ie_1[j];}

            if (ie_2[j] > ie_2_max) {ie_2_max = ie_2[j];}
            if (ie_2[j] < ie_2_min) {ie_2_min = ie_2[j];}

            if (ee[j] > ee_max) {ee_max = ee[j];}
            if (ee[j] < ee_min) {ee_min = ee[j];}
        } 
        
        for (r = 0; r < window; r++)
        {
        for (c = 0; c < window; c++)
        {
            ie_1_norm[r*window+c] = (ie_1[r*window+c] - ie_1_min) / (ie_1_max - ie_1_min);
            ie_2_norm[r*window+c] = (ie_2[r*window+c] - ie_2_min) / (ie_2_max - ie_2_min);
            ee_norm[r*window+c] = (ee[r*window+c] - ee_min) / (ee_max - ee_min);
        }
        }
        
        // [3.3] adding the normalized enrgies
        for (r = 0; r < window; r++)
        {
        for (c = 0; c < window; c++)
        {
                energy_window[r*window + c] = (weight_1 * ie_1_norm[r*window + c]) + (weight_2 * ie_2_norm[r*window + c]) - (weight_3 * ee_norm[r*window + c]);
        }
        }
        
        // [3.4] capturing the minimum energy coordinates in the window, and updating the conttour coordinates
        min_r = min_c = 0; 
        min_energy = INFINITY;
        
        for (r = 0; r<window; r++)
        {
        for (c = 0; c<window; c++)
        {
            if (energy_window[r*window + c]<min_energy) 
            {
                min_energy = energy_window[r*window + c];

                min_r = r-(window/2);
                min_c = c-(window/2);
            }
        }
        }

        px[i] = px[i] + min_c;
        py[i] = py[i] + min_r;
              
    }
    }

// [4] Exporting the output image and contour txt ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // [4.1] Exporting the final countour image
    for (i = 0; i < ROWS*COLS; i++) 
    {
        final_image[i] = image[i];
    }

    for (i = 0; i < total_points; i++)
    {
        for (int r = -3; r <=3; r++)
        {
            final_image[(py[i]+r)*COLS + (px[i])] = 0;
        }
        for (int c = -3; c <= 3; c++)
        {
            final_image[(py[i])*COLS + (px[i]+c)] = 0;
        }
    }

    fpt = fopen("final_cont_image.ppm","wb");
    fprintf(fpt,"%s %d %d 255 ",header,COLS, ROWS);
    fwrite(final_image,COLS*ROWS,sizeof(unsigned char),fpt);
    fclose(fpt);

    // [4.2] Printing the final co
    fpt=fopen("hawk_final.txt","wb");
    for (i=0; i<total_points; i++)
    {
        fprintf(fpt, "%d %d \n", px[i], py[i]);
    }
    fclose(fpt);

}
