/**
 * @file Array_control.h
 * @author Keegan Smith (keeginator42@gmail.com)
 * @brief This file contains function prototypes and 
 *    the battery structure for Battery_array_control
 * @version 0.7
 * @date 2023-12-19
 * 
 * @copyright Copyright (c) 2023
 **/

#ifndef Array_control_H

/***    PCB pin label to arduino pin id *****/
#define U09PD2  2
#define U09PD3  3
#define U09PD4  4
#define U09PD5  5
#define U09PD6  6
#define U09PD7  7
#define U09PD8  8
#define U09PD9  9
#define U09PD10 10
#define U09PD11 11
#define U09PD12 12
#define U09PD13 13
#define U09PD14 14
#define U09PD15 15
#define U09PD16 16
#define U09PD17 17
#define U09PD18 18
#define U09PD19 19
#define U09PD20 20
#define U09PD21 21
#define U09PD22 22
#define U09PD23 23
#define U09PD24 24
#define U09PD25 25
#define U09PD26 26
#define U09PD27 27
#define U09PD28 28
#define U09PD29 29
#define U09PD30 30



typedef struct battery
{
   float voltage_mes;
   bool  is_charging;
   int   adc_pin_assignment;
   int   OUT_FETS[5];
   int   CHARGE_FETS[2];
} battery;

/** Tertiary setup functioln */
void assign_output_fets();
void assign_charge_fets();

/** Functions to set the output configuration of the array */
void FULL_FET_DISCONNECT();
inline void BATT_CASE_SWTICH(int batt_case);
void BATT_CASE_SWTICH(int batt_case); 

/** Measurement functions */
void array_loaded_voltages();
void array_unloaded_voltages();

#endif //end Array_control_H