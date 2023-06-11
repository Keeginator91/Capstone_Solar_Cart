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

typedef struct Array_control
{
    double batt_1;
    double batt_2;
    double batt_3;
    double batt_4;
    double batt_5;
} array_struct;


/** Functions are to set the output configuration of the array */
void BATT_CASE_0();
void BATT_CASE_1(); 
void BATT_CASE_2();
void BATT_CASE_3();
void BATT_CASE_4();
void BATT_CASE_5();
//Functions are to return 1 if the operation was successful and 0 if not

void array_loaded_voltages(array_struct &loaded_voltages);
void array_unloaded_voltages(array_struct &unloaded_voltages);

#endif //end Array_control_H