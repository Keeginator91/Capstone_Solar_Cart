/**
 * @file Array_control.h
 * @author Keegan Smith (keeginator42@gmail.com)
 * @brief This file contains function prototypes, the battery structure for 
 *    Battery_array_control as well as declared macros for math and pin assignments
 * @version 0.9
 * @date 2023-12-20
 * 
 * @copyright Copyright (c) 2023
 **/

#ifndef Array_control_H

#define NUM_BATTS 5  //Number of batteries in the array
#define NUM_OUT_FETS_PER_BATT  5  //Number of output FETS used per battery
#define NUM_CHG_FETS_PER_BATT  2  //Number of charging FETS used per battery
#define NUM_CHG_FETS  10 //total number of charging fets   
#define NUM_OUT_FETS  11 //total number of output FETS

//adc conversion constants
#define ADC_RESOLUTION 1023.0f //max value adc will return
#define REF_VOLT          5.0f //reference voltage value
#define ADC_CONVERS_FACT   (REF_VOLT / ADC_RESOLUTION)

//Voltage divider network conversion constants for ADC circuit
#define R_NET_R1_VAL 100000.0f //100K ohm for R1
#define R_NET_R2_VAL  33000.0f //33K ohm for R2
#define R_NET_SCALE_FACTOR ( R_NET_R2_VAL / (R_NET_R1_VAL + R_NET_R2_VAL))  
      //Scaling factor to caclulate voltage divider input voltage

//Battery measurement constants
#define BATT_MAX_VOLTS   5.00f
#define BATT_FLOOR_VOLTS 0.00f
#define UNLOADED_VOLTAGE_MES_WAIT_TIME //ms

//MOSFET switching times (seconds)
#define MOSFET_TURN_ON   0.000000021f 
#define MOSFET_RISE_TIME 0.000000120f 
#define MOSFET_TURN_OFF  0.000000180f 
#define MOSFET_FALL_TIME 0.000000115f
#define MOSFET_ON_DELAY  (MOSFET_TURN_ON  + MOSFET_RISE_TIME)
#define MOSFET_OFF_DELAY (MOSFET_TURN_OFF + MOSFET_FALL_TIME)

/***    PCB schematic pin label to arduino pin id *****/
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

// ADC PIN DECLARATIONS (using pin numbers from ATMega2560)
#define ADC0 54  //Battery 1 (54)
#define ADC1 55  //Battery 2 (55)
#define ADC2 56  //Battery 3 (56)
#define ADC3 57  //Battery 4 (57)
#define ADC4 58  //Battery 5 (58)

/** battery structure declaration **/
typedef struct battery
{
   float voltage_mes;
   bool  is_charging;
   int   adc_pin_assignment;
   int   OUT_FETS[NUM_OUT_FETS_PER_BATT];
   int   CHARGE_FETS[NUM_CHG_FETS_PER_BATT];
} battery;

/** Tertiary setup functions **/
void assign_output_fets();
void assign_charge_fets();

/** Functions to set the MOSFET output configuration **/
void FULL_FET_DISCONNECT();
inline void BATT_CASE_SWTICH(int batt_case);
void BATT_CASE_SWTICH(int batt_case); 

/** Measurement functions **/
void array_loaded_voltages();
void array_unloaded_voltages();

#endif //end Array_control_H