/**
 * @file Array_control.h
 * @author Keegan Smith (keeginator42@gmail.com)
 * @brief This file contains function prototypes for Battery_array_control
 * @version 0.2
 * @date 2023-06-07
 * 
 * @copyright Copyright (c) 2023
 **/

#ifndef Array_control_H

typedef struct Array_readings
{
    float batt_1;
    float batt_2;
    float batt_3;
    float batt_4;
    float batt_5;
} readings_struct;


/** Functions are to set the output configuration of the array */
void BATT_CASE_0();
void BATT_CASE_1(); 
void BATT_CASE_2();
void BATT_CASE_3();
void BATT_CASE_4();
void BATT_CASE_5();

//Measurement functions
void array_loaded_voltages(Array_readings &loaded_voltages);
void array_unloaded_voltages(Array_readings &unloaded_voltages);


#endif //end Array_control_H