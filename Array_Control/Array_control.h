/**
 * @file Array_control.h
 * @author Keegan Smith (keeginator42@gmail.com)
 * @brief This file contains function prototypes and 
 *    the battery structure for Battery_array_control
 * @version 0.5
 * @date 2023-11-22
 * 
 * @copyright Copyright (c) 2023
 **/

#ifndef Array_control_H

typedef struct battery
{
   float voltage_mes;
   bool  is_charging;
   int   adc_pin_assignment;
   int   FETS[5];
} battery;

/** Functions are to set the output configuration of the array */
void FULL_FET_DISCONNECT();
void BATT_CASE_SWTICH(int batt_case); 

//Measurement functions
void array_loaded_voltages();
void array_unloaded_voltages();

#endif //end Array_control_H