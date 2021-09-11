#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// [0-excutable_file] [1-text_image.ppm] [2-letter_template.ppm] [3-the_letter_template] [4-ground_truth.txt]

void main (int argc, char *argv[]){
    FILE            *gt_file, *input_file, *template_file, *msf_file, *output_text, *binary_file;
    char            header[80], letter[80], gt_letter[50], *charcter;
    unsigned char   *input_image, *template_image, *msf_image_normalized, *binary_image;
    double          *msf_image, temp_mean =0;
    int             ROWS_input, COLS_input, BYTES_input, ROWS_temp, COLS_temp, BYTES_temp, sum, gt_r, gt_c;
    int             i, r, c, rc, cc, max, min, detected, not_detected, T, TP, FP, FN, TN, gt_row, gt_col;
    int             *template_zero_mean_centered;

    // Checking the terminal input
    if (argc != 5){ printf("Error: Not enough arguments, [0-excutable_file] [1-text_image.ppm] [2-letter_template.ppm] [3-the_letter_template] [4-ground_truth.txt] \n");
        exit(0);}

    // Creating the Zero Mean Centred Template from the template image ////////////////////////////////////////////////////////////////////////////////
    // Reading the template image
    template_file = fopen(argv[2], "rb");
    if (template_file==NULL){
        printf("Error: [3] Image (%s) was not imported, please check the name and make sure it is in the same directory of the excutable", argv[1]);
        exit(0);}
    fscanf(template_file, "%s %d %d %d ", header, &COLS_temp, &ROWS_temp, &BYTES_temp);  // intemplate provided rows = cols and vice versa 
    template_image = (unsigned char *)calloc(ROWS_temp*COLS_temp, sizeof(unsigned char));
    fread(template_image, 1, ROWS_temp*COLS_temp, template_file);
    fclose(template_file); 

    // Calculating the mean of the template image    
    for (i=0; i<=(ROWS_temp*COLS_temp); i++){
        temp_mean += (double)template_image[i];
    }
    temp_mean = temp_mean/((double)ROWS_temp*(double)COLS_temp);
    printf("Template image mean = %d\n", (int)temp_mean);

    // Creating the zero mean centred template 
    template_zero_mean_centered = (int *)calloc(ROWS_temp*COLS_temp, sizeof(int));
    for (i=0; i<(ROWS_temp*COLS_temp); i++){
        template_zero_mean_centered[i] = (int)((double)template_image[i] - temp_mean);
    }
    
    // Creating Normalized Matched-Spatial Filter (MSF) from the input image and the template //////////////////////////////////////////////////////////
    // Reading the input image
    input_file = fopen((argv[1]), "rb");
    if (template_file==NULL){
        printf("Error: [1] Image (%s) was not imported, please check the name and make sure it is in the same directory of the excutable", argv[1]);
        exit(0);}
    fscanf(input_file, "%s %d %d %d ", header, &COLS_input, &ROWS_input, &BYTES_input);
    input_image = (unsigned char*)calloc(ROWS_input*COLS_input, sizeof(unsigned char));
    fread(input_image, sizeof(unsigned char), ROWS_input*COLS_input, input_file);
    fclose(input_file);

    // Creating the MSF image
    msf_image = (double *)calloc(ROWS_input*COLS_input, sizeof(double));

    for (r=0; r<ROWS_input; r++){
        for (c=0; c<COLS_input; c++){
            sum =0;
            if (r<(ROWS_temp/2) || r>=(ROWS_input-(ROWS_temp/2)) || c<(COLS_temp/2) || c>=(COLS_input-(COLS_temp/2))){
                msf_image[r*COLS_input+c] = 0;
            }
            else{
                // Convolution box
                for (rc=-(ROWS_temp/2); rc<=(ROWS_temp/2); rc++){
                    for (cc=-(COLS_temp/2); cc<=(COLS_temp/2); cc++){
                        sum += (template_zero_mean_centered[(rc+(ROWS_temp/2))*COLS_temp+(cc+(COLS_temp/2))]*input_image[(r+rc)*COLS_input+(c+cc)]);                                            
                    }
                }
                msf_image[r*COLS_input+c] = sum;
            }
        }    

    }

    // Normalizing the MSF image
    max  =0; min = 10000000;
    for (i=0; i<(ROWS_input*COLS_input); i++){
        if (msf_image[i] > max) {max = msf_image[i];}
        else if (msf_image[i]<min) {min = msf_image[i];}       
    }
    printf("max = %d, and min = %d \n", max, min);

    msf_image_normalized = (unsigned char *)calloc(ROWS_input*COLS_input, sizeof(unsigned char));
    int pixel;
    for (i=0; i<(ROWS_input*COLS_input); i++){
        msf_image_normalized[i] = ((255*(msf_image[i] - min)/(max-min)) + 0);
    }

    msf_file = fopen("msf_normalized.ppm","w");
    fprintf(msf_file,"%s %d %d 255 ",header,COLS_input, ROWS_input);
    fwrite(msf_image_normalized,COLS_input*ROWS_input,sizeof(unsigned char),msf_file);
    fclose(msf_file);

    // Exploring range of thresholds and creating ROC curve ////////////////////////////////////////////////////////////////////////////////////////////
    charcter  = argv[3];                                    // Capturing the charcter to be explored from the arg input
    output_text = fopen("T_TP_FP_FN_TN_TPR_FPR.txt", "w");  // the output file 
    fprintf(output_text,"T  TP  FP  FN  TN  TPR  FPR \n");  // adding the header to the line

    for (T=100; T<=255; T++){
        // creating new binary image --> Not needed in the final output
        binary_image = (unsigned char *)calloc(ROWS_input*COLS_input, sizeof(unsigned char));
        for (i=0; i<(ROWS_input*COLS_input); i++){
            if (msf_image_normalized[i] >= T){binary_image[i] = 255;}
            else {binary_image[i] = 0;}
        }

        // Opening the Ground Truth file
        gt_file = fopen(argv[4], "rb");

        // Exploring the input image looping with different threshholds and using the ground truth inputs and the Normalized MSF output
        TP = FP = FN = TN = 0;
        while (1){
            i = fscanf(gt_file, "%s %d %d", gt_letter, &gt_col, &gt_row);  
                 
            if (i!=3){
                break;
            }
            else{
                detected = not_detected = 0;
                
                //the surrounding box to the ground truth pixel --> Output (Detected or not detected)
                for (r=-(ROWS_temp/2); r<=(ROWS_temp/2); r++){
                    for (c=-(COLS_temp/2); c<=(COLS_temp/2); c++){
                        if (msf_image_normalized[(gt_row+r)*COLS_input+(gt_col+c)] >= T){
                            detected = 1;                  
                        }
                        else{
                            not_detected = 1; 
                        }
                    }
                }

                // Calculating the TP, FP, FN, and TN
                if (detected == 1){
                    if (*gt_letter == *charcter) {TP += 1;}
                    else {FP += 1;}
                }
                else if ((not_detected == 1) && (detected == 0))
                {
                    if (*gt_letter == *charcter) {FN += 1;}
                    else {TN += 1;}
                }
            }
        }

        // Output the results into new file 
        // printf("T = %d, TP = %d, FP = %d, FN =%d, TN = %d, TPR = %f, FPR = %f \n", T, TP, FP, FN, TN, ((double)TP/(double)(TP+FN)), ((double)FP/(double)(FP+TN)));
        fprintf(output_text," %d  %d  %d %d  %d  %f  %f \n", T, TP, FP, FN, TN, ((double)TP/(double)(TP+FN)), ((double)FP/(double)(FP+TN)));
        fclose(gt_file);
    }
    
    fclose(output_text);

    // Writing a binary image with a choosen T /////////////////////////////////////////////////////////////////////////////////////////////////
    T = 207;        // The knee point in the ROC graph
    binary_image = (unsigned char *)calloc(ROWS_input*COLS_input, sizeof(unsigned char));
        for (i=0; i<(ROWS_input*COLS_input); i++){
            if (msf_image_normalized[i] >= T){binary_image[i] = 255;}
            else {binary_image[i] = 0;}
        }
    
    binary_file = fopen("binary_with_T.ppm","w");
    fprintf(msf_file,"%s %d %d 255 ",header,COLS_input, ROWS_input);
    fwrite(binary_image,COLS_input*ROWS_input,sizeof(unsigned char),msf_file);
    fclose(msf_file);
}