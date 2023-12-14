/**
 * @file Array_control.h
 * @author Keegan Smith (keeginator42@gmail.com)
 * @brief This file contains function prototypes and 
 *    the battery structure for Battery_array_control
 * @version 0.6
 * @date 2023-12-11
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

/** ISR function prototypes */
void BUTTON0_ISR();
void BUTTON1_ISR();
void BUTTON2_ISR();
void BUTTON3_ISR();
void BUTTON4_ISR();
void BUTTON5_ISR();

/** Functions to set the output configuration of the array */
void FULL_FET_DISCONNECT();
void BATT_CASE_SWTICH(int batt_case); 

/** Measurement functions */
void array_loaded_voltages();
void array_unloaded_voltages();

/** Tertiary functions */
bool button_debounce(const int button_pin);

#endif //end Array_control_H