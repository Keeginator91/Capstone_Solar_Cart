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

typedef struct battery
{
   float voltage_mes;
   bool  is_charging;
   int   adc_pin_assignment;
} readings_struct;


/** Functions are to set the output configuration of the array */
void FULL_FET_DISCONNECT();
void BATT_CASE_0(); 
void BATT_CASE_1();
void BATT_CASE_2();
void BATT_CASE_3();
void BATT_CASE_4();

//Measurement functions
void array_loaded_voltages();
void array_unloaded_voltages();


#endif //end Array_control_H