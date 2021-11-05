#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// [0-excutable_file] [1-text_image.ppm] [2-letter_template.ppm] [3-ground_truth.txt] [4-the_letter] [5-number_of_end_points_in_the_letter] [6-number_of_branch_points_in_the_letter]

void main (int argc, char *argv[]){
    FILE            *gt_file, *input_file, *template_file, *msf_file, *output_text, *binary_file, *test_file;
    char            header[80], letter[80], gt_letter[50], *charcter;
    unsigned char   *input_image, *template_image, *msf_image_normalized, *binary_image, *center_of_letter_image, *binary_letter_image, *to_be_erased;
    double          *msf_image, temp_mean =0;
    int             ROWS_input, COLS_input, BYTES_input, ROWS_temp, COLS_temp, BYTES_temp, sum, gt_r, gt_c;
    int             i, r, c, rc, cc, max, min, detected, not_detected, T, TP, FP, FN, TN, gt_row, gt_col, transitions_count, edge_neighbour_count;
    int             not_edge_check, erased_count, branch_points_count, end_points_count, letter_branch_points, letter_end_points;
    int             *template_zero_mean_centered;

    // Checking the terminal input ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    if (argc != 7){ printf("Error: Not enough arguments, [0-excutable_file] [1-text_image.ppm] [2-letter_template.ppm] [3-ground_truth.txt] [4-the_letter] [5-number_of_end_points_in_the_letter] [6-number_of_branch_points_in_the_letter] \n");
        exit(0);}

    if ((atoi(argv[5])<0) || (atoi(argv[6])<0)) {printf("Error 5, 6: Number of branches or end points shouldn't be negative");}

    charcter  = argv[4];                                    // Capturing the charcter to be explored from the arg input
    letter_end_points = atoi(argv[5]);                      // number of end points in the the letter chosen
    letter_branch_points = atoi(argv[6]);                   // number of branch points in the letter chosen

    // Creating the Zero Mean Centred Template from the template image ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
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
    
    // Creating Normalized Matched-Spatial Filter (MSF) from the input image and the template /////////////////////////////////////////////////////////////////////////////////////////////////
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

    // Exploring range of thresholds and creating ROC curve using template matching and sekeletonization///////////////////////////////////////////////////////////////////////////////////////
    output_text = fopen("T_TP_FP_FN_TN_TPR_FPR.txt", "w");  // the output file 
    fprintf(output_text,"T  TP  FP  FN  TN  TPR  FPR \n");  // adding the header to the line

    for (T=100; T<=255; T++){
        
        // creating new binary image --> Not needed in the final output
        binary_image = (unsigned char *)calloc(ROWS_input*COLS_input, sizeof(unsigned char));
        for (i=0; i<(ROWS_input*COLS_input); i++){
            if (msf_image_normalized[i] >= T) {binary_image[i] = 255;}
            else {binary_image[i] = 0;}
        }

        // Opening the Ground Truth file
        gt_file = fopen(argv[3], "rb");

        // for each letter in ground truth file - using the ground truth inputs and the Normalized MSF output - 
        TP = FP = FN = TN = 0;
        while (1){
            i = fscanf(gt_file, "%s %d %d", gt_letter, &gt_col, &gt_row);  

            if (i!=3) {break;}
            else{
                detected = not_detected = 0;
                
                //the surrounding box to the ground truth pixel --> Output (Detected or not detected)
                for (r=-(ROWS_temp/2); r<=(ROWS_temp/2); r++){
                    for (c=-(COLS_temp/2); c<=(COLS_temp/2); c++){
                        if (msf_image_normalized[(gt_row+r)*COLS_input+(gt_col+c)] >= T) {detected = 1;}
                        else {not_detected = 1;}
                    }
                }

                // (9*15) binary image surrounding the ground truth center of the detected letter - threshold = 128
                center_of_letter_image = (unsigned char *)calloc(ROWS_temp*COLS_temp, sizeof(unsigned char));
                if (detected == 1){
                    for (r=-(ROWS_temp/2); r<=(ROWS_temp/2); r++){
                        for (c=-(COLS_temp/2); c<=(COLS_temp/2); c++){
                            if (input_image[(gt_row+r)*COLS_input+(gt_col+c)] >= 128){
                                center_of_letter_image[(r+(ROWS_temp/2))*COLS_temp + (c+(COLS_temp/2))] = 0;
                            } else {
                                center_of_letter_image[(r+(ROWS_temp/2))*COLS_temp + (c+(COLS_temp/2))] = 255;
                            }
                        }
                    }
                }

                // adding a surrounding 1 pixel to the binary image surrounding the letter
                binary_letter_image = (unsigned char *)calloc((ROWS_temp+2)*(COLS_temp+2), sizeof(unsigned char));
                for (r = 1; r <= ROWS_temp; r++){               // start from one as 0 and end will be zeros already
                    for (c = 1; c <= COLS_temp; c++){           // start from one as 0 and end will be zeros already
                        binary_letter_image[r*(COLS_temp+2)+c] = center_of_letter_image[(r-1)*COLS_temp+(c-1)];
                    }
                }              

                // skeletonization of the binary image
                while (1){

                    // check each letter 
                    to_be_erased = (unsigned char *)calloc((ROWS_temp+2)*(COLS_temp+2), sizeof(unsigned char));
                    for(r = 1; r<=ROWS_temp; r++){
                        for(c = 1; c<=COLS_temp; c++){
                            
                            // edge pixel?
                            if (binary_letter_image[r*(COLS_temp+2) + c] == 255){
                                
                                // Count the number of edge->non-edge transitions in CW
                                transitions_count = ((binary_letter_image[(r-1)*(COLS_temp+2)+(c)] - binary_letter_image[(r-1)*(COLS_temp+2)+(c-1)]) == -255) +
                                                    ((binary_letter_image[(r-1)*(COLS_temp+2)+(c+1)] - binary_letter_image[(r-1)*(COLS_temp+2)+(c)]) == -255) +
                                                    ((binary_letter_image[(r)*(COLS_temp+2)+(c+1)] - binary_letter_image[(r-1)*(COLS_temp+2)+(c+1)]) == -255) +
                                                    ((binary_letter_image[(r+1)*(COLS_temp+2)+(c+1)] - binary_letter_image[(r)*(COLS_temp+2)+(c+1)]) == -255) +
                                                    ((binary_letter_image[(r+1)*(COLS_temp+2)+(c)] - binary_letter_image[(r+1)*(COLS_temp+2)+(c+1)]) == -255) +
                                                    ((binary_letter_image[(r+1)*(COLS_temp+2)+(c-1)] - binary_letter_image[(r+1)*(COLS_temp+2)+(c)]) == -255) +
                                                    ((binary_letter_image[(r)*(COLS_temp+2)+(c-1)] - binary_letter_image[(r+1)*(COLS_temp+2)+(c-1)]) == -255) +
                                                    ((binary_letter_image[(r-1)*(COLS_temp+2)+(c-1)] - binary_letter_image[(r)*(COLS_temp+2)+(c-1)]) == -255);

                                // Count the number of edge neighbor pixels
                                edge_neighbour_count = (binary_letter_image[(r-1)*(COLS_temp+2)+(c-1)] == 255) + 
                                                    (binary_letter_image[(r-1)*(COLS_temp+2)+(c)] == 255) + 
                                                    (binary_letter_image[(r-1)*(COLS_temp+2)+(c+1)] == 255) + 
                                                    (binary_letter_image[(r)*(COLS_temp+2)+(c+1)] == 255) + 
                                                    (binary_letter_image[(r+1)*(COLS_temp+2)+(c+1)] == 255) + 
                                                    (binary_letter_image[(r+1)*(COLS_temp+2)+(c)] == 255) + 
                                                    (binary_letter_image[(r+1)*(COLS_temp+2)+(c-1)] == 255) + 
                                                    (binary_letter_image[(r)*(COLS_temp+2)+(c-1)] == 255);
                                
                                // North, East, or (West and South) are not edge pixels
                                not_edge_check = 0;
                                if ((binary_letter_image[(r-1)*(COLS_temp+2)+(c)] == 0) || (binary_letter_image[(r)*(COLS_temp+2)+(c+1)] == 0) || ((binary_letter_image[(r)*(COLS_temp+2)+(c-1)] == 0) && (binary_letter_image[(r+1)*(COLS_temp+2)+(c)] == 0))){
                                    not_edge_check = 1;
                                }

                                // Marked for erasure check
                                if ((transitions_count == 1) && (edge_neighbour_count>=2) && (edge_neighbour_count<=6) && (not_edge_check ==1)) {to_be_erased[r*(COLS_temp+2)+c] = 1;}                       
                                else {to_be_erased[r*(COLS_temp+2)+c] = 0;}
                            }
                            else {continue;}
                        }
                    }
                    
                    // erasing the marked pixels
                    erased_count = 0;
                    for (i = 0; i < ((ROWS_temp+2)*(COLS_temp+2)); i++){
                        if (to_be_erased[i] == 1){
                            binary_letter_image[i] = 0;
                            erased_count += 1;
                        }
                    }

                    // break if you are done with the skeletonization
                    if (erased_count == 0) {break;}                
                }
                
                // Counting the end and branch points 
                end_points_count = 0;
                branch_points_count = 0;
                for(r = 1; r<=ROWS_temp; r++){
                    for(c = 1; c<=COLS_temp; c++){
                        if (binary_letter_image[r*(COLS_temp+2) + c] == 255){
                            transitions_count = ((binary_letter_image[(r-1)*(COLS_temp+2)+(c)] - binary_letter_image[(r-1)*(COLS_temp+2)+(c-1)]) == -255) +
                                                ((binary_letter_image[(r-1)*(COLS_temp+2)+(c+1)] - binary_letter_image[(r-1)*(COLS_temp+2)+(c)]) == -255) +
                                                ((binary_letter_image[(r)*(COLS_temp+2)+(c+1)] - binary_letter_image[(r-1)*(COLS_temp+2)+(c+1)]) == -255) +
                                                ((binary_letter_image[(r+1)*(COLS_temp+2)+(c+1)] - binary_letter_image[(r)*(COLS_temp+2)+(c+1)]) == -255) +
                                                ((binary_letter_image[(r+1)*(COLS_temp+2)+(c)] - binary_letter_image[(r+1)*(COLS_temp+2)+(c+1)]) == -255) +
                                                ((binary_letter_image[(r+1)*(COLS_temp+2)+(c-1)] - binary_letter_image[(r+1)*(COLS_temp+2)+(c)]) == -255) +
                                                ((binary_letter_image[(r)*(COLS_temp+2)+(c-1)] - binary_letter_image[(r+1)*(COLS_temp+2)+(c-1)]) == -255) +
                                                ((binary_letter_image[(r-1)*(COLS_temp+2)+(c-1)] - binary_letter_image[(r)*(COLS_temp+2)+(c-1)]) == -255);

                            if (transitions_count == 1) {end_points_count += 1;} 
                            else if (transitions_count > 2) {branch_points_count += 1;}
                        }
                    }
                }

                // Testing against the letter properties
                if ((end_points_count == letter_end_points) && (branch_points_count == letter_branch_points)){
                    detected = 1;
                    not_detected = 0;
                } else{
                    detected = 0;
                    not_detected = 1;
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

    // Writing a binary image with a choosen T ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    T = 207;        // The knee point in the ROC graph
    binary_image = (unsigned char *)calloc(ROWS_input*COLS_input, sizeof(unsigned char));
        for (i=0; i<(ROWS_input*COLS_input); i++){
            if (msf_image_normalized[i] >= T){binary_image[i] = 255;}
            else {binary_image[i] = 0;}
        }
    
    binary_file = fopen("binary_with_T.ppm","w");
    fprintf(binary_file,"%s %d %d 255 ",header,COLS_input, ROWS_input);
    fwrite(binary_image,COLS_input*ROWS_input,sizeof(unsigned char),binary_file);
    fclose(binary_file);
}