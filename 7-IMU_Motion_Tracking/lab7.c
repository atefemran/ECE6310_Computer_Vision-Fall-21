

////////////////////////////////////////////////////////
/////              Lab 7: Motion Tracking          /////
///////                 Atef Emran              ////////
////////////////////////////////////////////////////////

#define window_smoothing 15
#define window_variance 17
#define threshold_var_X 0.00002             
#define threshold_var_Y 0.00009             
#define threshold_var_Z_min 0.0002
#define threshold_var_Z_max 0.013
#define threshold_var_pitch 0.002
#define threshold_var_roll 0.0006
#define threshold_var_yaw 0.02
#define g 9.81

#define time_step 0.05

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[])
{
FILE	    *fpt;
int	        i, w;
double	    time[1300], x_acc[1300], y_acc[1300], z_acc[1300], pitch[1300], roll[1300], yaw[1300];

// mean filtering
double	    x_acc_mean[1300], y_acc_mean[1300], z_acc_mean[1300], pitch_mean[1300], roll_mean[1300], yaw_mean[1300];      // smoothing
int         total_data, limit_smooth, limit_variance;
char        dummy;

// variance
double      avg_x, avg_y, avg_z, avg_pitch, avg_roll, avg_yaw;
double      sum_sq_x, sum_sq_y, sum_sq_z, sum_sq_pitch, sum_sq_roll, sum_sq_yaw;
double      variance_X[1300], variance_Y[1300], variance_Z[1300], variance_pitch[1300], variance_roll[1300], variance_yaw[1300];
int         motion_X[1300], motion_Y[1300], motion_Z[1300], motion_pitch[1300], motion_roll[1300], motion_yaw[1300];

// rest decission
int         rest[1300], rest_change[1300], rest_start[300], end_rest[300];
int         rest_start_index, rest_end_index;

// motion decissions
int         motion[1300], motion_change[1300], motion_start[300], end_motion[300];
int         motion_start_index, motion_end_index;

// motion calculations 
int start, end;
double init_vel_X, init_vel_Y, init_vel_Z, init_angle_roll, init_angle_pitch, init_angle_yaw;
double distance_X, distance_Y, distance_Z, angle_roll, angle_pitch, angle_yaw;
double avg_vel_X, avg_vel_Y, avg_vel_Z; 
double vel_X, vel_Y, vel_Z;        

// [1] Importing the data               ////////////////////////////////////////////////////////////////////////////////////////////////////
fpt=fopen("acc_gyro.txt","rb");
fscanf(fpt, "%s %s %s %s %s %s %s", &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy);
total_data=0;
while (1)
{
    i=fscanf(fpt,"%lf %lf %lf %lf %lf %lf %lf",&time[total_data], &x_acc[total_data], &y_acc[total_data], &z_acc[total_data], &pitch[total_data], &roll[total_data], &yaw[total_data]);
    if (i != 7)
        break;
    total_data++;
}
fclose(fpt);

printf("%lf", time[total_data-1]);

// [2] smoothing the data               ////////////////////////////////////////////////////////////////////////////////////////////////////
limit_smooth = window_smoothing / 2;

for (i = 0; i <total_data; i++)
{
    if (((i+limit_smooth)<0) || ((i+limit_smooth)>(total_data-1))){
        x_acc_mean[i] = x_acc[i];
        y_acc_mean[i] = y_acc[i];
        z_acc_mean[i] = z_acc[i];
        pitch_mean[i] = pitch[i];
        roll_mean[i]  = roll[i];
        yaw_mean[i]   = yaw[i];
    }
    else{
        x_acc_mean[i] = 0;
        y_acc_mean[i] = 0;
        z_acc_mean[i] = 0;
        pitch_mean[i] = 0;
        roll_mean[i]  = 0;
        yaw_mean[i]   = 0;
        for (w = -limit_smooth; w <= limit_smooth; w++)
        {
            x_acc_mean[i] += x_acc[i+w];
            y_acc_mean[i] += y_acc[i+w];
            z_acc_mean[i] += z_acc[i+w];
            pitch_mean[i] += pitch[i+w];
            roll_mean[i]  += roll[i+w];
            yaw_mean[i]   += yaw[i+w];
        }
        x_acc_mean[i] = x_acc_mean[i] / window_smoothing;
        y_acc_mean[i] = y_acc_mean[i] / window_smoothing;
        z_acc_mean[i] = z_acc_mean[i] / window_smoothing;
        pitch_mean[i] = pitch_mean[i] / window_smoothing;
        roll_mean[i]  = roll_mean[i] / window_smoothing;
        yaw_mean[i]   = yaw_mean[i] / window_smoothing;
    }
}


// [3] Calculating the variance and decide motion or not
  limit_variance = window_variance/2;
  for ( i = 0; i < total_data; i++)
  {
    
    // calculate the average of the values in the window
    if (i < limit_variance || i > (total_data - limit_variance))
    {
      variance_X[i] = 0.0;
      variance_Y[i] = 0.0;
      variance_Z[i] = 0.0;
      variance_pitch[i] = 0.0;
      variance_roll[i] = 0.0;
      variance_yaw[i] = 0.0;
    }
    else
    {
      avg_x  = 0.0; avg_y = 0.0; avg_z = 0.0; avg_pitch = 0.0; avg_roll = 0.0; avg_yaw = 0.0;
      for (int m = -limit_variance; m <= limit_variance; m++)
      {
        avg_x += x_acc_mean[i+m];
        avg_y += y_acc_mean[i+m];
        avg_z += z_acc_mean[i+m];
        avg_pitch += pitch_mean[i+m];
        avg_roll += roll_mean[i+m];
        avg_yaw += yaw_mean[i+m];
      }
      avg_x = avg_x / window_variance; 
      avg_y = avg_y / window_variance; 
      avg_z = avg_z / window_variance;
      avg_pitch = avg_pitch / window_variance; 
      avg_roll = avg_roll / window_variance; 
      avg_yaw = avg_yaw / window_variance;

      // sum of square difference for each point in the window to the mean
      sum_sq_x = 0; sum_sq_y = 0; sum_sq_z = 0; sum_sq_pitch = 0; sum_sq_roll = 0; sum_sq_yaw = 0;
      for (int m = -limit_variance; m <= limit_variance; m++)
      {
        sum_sq_x += pow((x_acc_mean[i+m] - avg_x), 2);
        sum_sq_y += pow((y_acc_mean[i+m] - avg_y), 2);
        sum_sq_z += pow((z_acc_mean[i+m] - avg_z), 2);
        sum_sq_pitch += pow((pitch_mean[i+m] - avg_pitch), 2);
        sum_sq_roll += pow((roll_mean[i+m] - avg_roll), 2);
        sum_sq_yaw += pow((yaw_mean[i+m] - avg_yaw), 2);
      }

      // calculating the variance
      variance_X[i] = sum_sq_x / (window_variance - 1); 
      variance_Y[i] = sum_sq_y / (window_variance - 1); 
      variance_Z[i] = sum_sq_z / (window_variance - 1);
      variance_pitch[i] = sum_sq_pitch / (window_variance - 1); 
      variance_roll[i] = sum_sq_roll / (window_variance - 1); 
      variance_yaw[i] = sum_sq_yaw / (window_variance - 1);

      if (time[i] < 28){
      printf("%lf, X: %lf, Y: %lf \n", time[i], variance_X[i], variance_Y[i]);}
    }
  } 

  // deciding if there is a motion in a direction of not based on the variance threshold
  for (i = 0; i < total_data; i++)
  {
    if (variance_X[i] <= threshold_var_X) {motion_X[i] = 0;}
    else if (variance_X[i] > threshold_var_X) {motion_X[i] = 1;}

    if (variance_Y[i] <= threshold_var_Y) {motion_Y[i] = 0;}
    else if (variance_Y[i] > threshold_var_Y) {motion_Y[i] = 1;}

    if (variance_Z[i] <= threshold_var_Z_min && variance_Z[i] >= threshold_var_Z_max) {motion_Z[i] = 0;}
    else if (variance_Z[i] > threshold_var_Z_min && variance_Z[i] < threshold_var_Z_max) {motion_Z[i] = 1;}

    if (variance_pitch[i] <= threshold_var_pitch) {motion_pitch[i] = 0;}
    else if (variance_pitch[i] > threshold_var_pitch) {motion_pitch[i] = 1;}

    if (variance_roll[i] <= threshold_var_roll) {motion_roll[i] = 0;}
    else if (variance_roll[i] > threshold_var_roll) {motion_roll[i] = 1;}

    if (variance_yaw[i] <= threshold_var_yaw) {motion_yaw[i] = 0;}
    else if (variance_yaw[i] > threshold_var_yaw) {motion_yaw[i] = 1;}
  }


// [4] Determine the rest periods and print them out

  // Determining which indeces we have rest
  for (i = 0; i < total_data; i++)
  {
    if (motion_X[i] == 0 && motion_Y[i] == 0 && motion_Z[i] == 0 && motion_roll[i] == 0 && motion_pitch[i] == 0 && motion_yaw[i] == 0) {rest[i] = 1;}
    else if (motion_X[i] == 1 || motion_Y[i] == 1 || motion_Z[i] == 1 || motion_roll[i] == 1 || motion_pitch[i] == 1 || motion_yaw[i] == 1) {rest[i] = 0;}
  }

  // Determining the start and the end points of the rest 
  for (i = 0; i < total_data; i++)
  {
    if (i+1 > total_data) {rest_change[i] = 0;}
    else {rest_change[i] = rest[i+1] - rest[i];}        // positive change_rest => motion to rest; negative change_rest => rest to motion
  }
  rest_change[0] = 1;                                   // start at rest
  
  // getting the index of start and end
  rest_end_index = 0;
  rest_start_index = 0;
  for (i = 0; i < total_data; i++)
  {
    if (rest_change[i] == 1) {rest_start[rest_start_index] = i + 1; rest_start_index++;}
    else if (rest_change[i] == -1) {end_rest[rest_end_index] = i; rest_end_index++;}
    else if (rest_change[i] == 0) {continue;}
  }

  // output the rest data 
  fpt = fopen("rest_period.txt","wb");
  fprintf(fpt, "start_time \t end_time \t start_index \t end_index \n");
  for (i = 0; i < rest_start_index; i++)
  {
    fprintf(fpt, "%f \t %f \t %d \t %d\n", time[rest_start[i]], time[end_rest[i]], rest_start[i], end_rest[i]);
  }
  fclose(fpt);

// [5] Determine the motion periods 
  
  // Determining which indeces we have motion
  for (i = 0; i < total_data; i++)
  {
    if (motion_X[i] == 0 && motion_Y[i] == 0 && motion_Z[i] == 0 && motion_roll[i] == 0 && motion_pitch[i] == 0 && motion_yaw[i] == 0) {motion[i] = 0;}
    else if (motion_X[i] == 1 || motion_Y[i] == 1 || motion_Z[i] == 1 || motion_roll[i] == 1 || motion_pitch[i] == 1 || motion_yaw[i] == 1) {motion[i] = 1;}
  }

  // Determining the start and the end points of the motion 
  for (i = 0; i < total_data; i++)
  {
    if (i+1 > total_data) {motion_change[i] = 0;}
    else {motion_change[i] = motion[i+1] - motion[i];}        // positive change_motion => rest to motion; negative change_motion => motion to rest
  }

  // getting the index of start and end
  motion_end_index = 0;
  motion_start_index = 0;
  for (i = 0; i < total_data; i++)
  {
    if (motion_change[i] == 1) {motion_start[motion_start_index] = i + 1; motion_start_index++;}
    else if (motion_change[i] == -1) {end_motion[motion_end_index] = i; motion_end_index++;}
    else if (motion_change[i] == 0) {continue;}
  }

// [6] Integrating for the distance and angle and prinitng out the results of the motion
  fpt = fopen("motion_periods_calculations.txt","wb");
  fprintf(fpt, "start_time \t end_time \t start_index \t end_index \t distance_X \t distance_Y \t distance_Z \t angle_roll \t angle_pitch \t angle_yaw \n");

  for (i = 0; i < motion_start_index; i++)
  {
    start = motion_start[i]; end = end_motion[i];

    init_vel_X = 0.0, init_vel_Y = 0.0, init_vel_Z = 0.0, init_angle_roll = 0.0, init_angle_pitch = 0.0, init_angle_yaw = 0.0;
    distance_X = 0.0, distance_Y = 0.0, distance_Z = 0.0, angle_roll = 0.0, angle_pitch = 0.0, angle_yaw = 0.0;

    for (int k = start; k <= end; k++)          // looping from the start of the motion to the end of the motion in the mean filtered data to calculate
    {
      avg_vel_X = 0.0, avg_vel_Y = 0.0, avg_vel_Z = 0.0; 
      vel_X = 0.0, vel_Y = 0.0, vel_Z = 0.0;                    // at the start of the motion velocities equal zero
      
      // using accleration data to calculate X distance
      vel_X = init_vel_X + (x_acc_mean[k]*time_step*g);
      avg_vel_X = (vel_X + init_vel_X)/ 2;
      distance_X = distance_X + avg_vel_X*time_step;
      init_vel_X = vel_X;

      // using accleration data to calculate y distance
      vel_Y = init_vel_Y + (y_acc_mean[k]*time_step*g);
      avg_vel_Y = (vel_Y + init_vel_Y)/ 2;
      distance_Y = distance_Y + avg_vel_Y*time_step;
      init_vel_Y = vel_Y;

      // using accleration data to calculate z distance
      vel_Z = init_vel_Z + (z_acc_mean[k]*time_step*g);         // ignoring the influence of gravity on z acceleration
      avg_vel_Z = (vel_Z + init_vel_Z)/ 2;
      distance_Z = distance_Z + avg_vel_Z*time_step;
      init_vel_Z = vel_Z;

      // angle of roll
      angle_roll = (roll_mean[k]*time_step) + init_angle_roll;
      init_angle_roll = angle_roll;

      // angle of pitch
      angle_pitch = (pitch_mean[k]*time_step) + init_angle_pitch;
      init_angle_pitch = angle_pitch;

      // angle of yaw
      angle_yaw = (yaw_mean[k]*time_step) + init_angle_yaw;
      init_angle_yaw = angle_yaw;
    }
    fprintf(fpt, "%lf \t %lf \t %d \t %d \t %lf \t %lf \t %lf \t %lf \t %lf \t %lf\n",  time[motion_start[i]], time[end_motion[i]], motion_start[i], end_motion[i], distance_X, distance_Y, distance_Z, angle_roll, angle_pitch, angle_yaw);
  }
  fclose(fpt);

}


