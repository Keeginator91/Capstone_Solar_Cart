/**
 * @file Array_Control.c
 * @author Keegan Smith (keeginator42@gmail.com), Matthew DeSantis, Thomas Cecelya
 * @brief This file contains the main function to control and maintain the battery array
 * @version 0.9
 * @date 2023-12-11
 * 
 * @copyright Copyright (c) 2023
*/

/* INCLUDED LIBRARIES*/
#include <stdbool.h>
#include <string.h>
#include <avr/interrupt.h>


/* LOCAL FILES*/
#include "Array_control.h"

/*************
 * CONSTANTS *
 *************/
bool DEBUG = true;

#define NUM_BATTS 5  //Number of batteries in the array
#define NUM_FETS  5  //Number of FETS used to bypass a battery
#define FET_ARRAY_LEN 11 //total number of FETS used

//adc conversion constants
#define ADC_RESOLUTION 1023.0 //max value adc will return
#define REF_VOLT          5.0 //reference voltage value
#define ADC_CONVERS_FACT   (REF_VOLT / ADC_RESOLUTION)

//Voltage divider network conversion constants
#define R1_VAL 100000 //100K ohm for R1
#define R2_VAL  33000 //33K ohm for R2
#define R_NET_SCALE_FACTOR ( R2_VAL / (R1_VAL + R2_VAL))  //Scaling factor to caclulate voltage divider input voltage

//Battery measurement constants
#define BATT_MAX_VOLTS   0
#define BATT_FLOOR_VOLTS 0
#define UNLOADED_VOLTAGE_MES_WAIT_TIME //ms

//MOSFET switching times (seconds)
#define MOSFET_TURN_ON   0.000000021 
#define MOSFET_RISE_TIME 0.000000120 
#define MOSFET_TURN_OFF  0.000000180 
#define MOSFET_FALL_TIME 0.000000115
#define MOSFET_ON_DELAY  (MOSFET_TURN_ON  + MOSFET_RISE_TIME)
#define MOSFET_OFF_DELAY (MOSFET_TURN_OFF + MOSFET_FALL_TIME)

/******************
* PIN ASSIGNMENTS *
******************/

/* CHARGING FET PIN ASSIGNMENTS*/
/* These pin assignments are not likely to be driven seperately
    as the FETs will be driven from battery bypass FETs*/
/*
#define CHG_FET1  //paired with OUT_FET2
#define CHG_FET2  //paired with OUT_FET2

#define CHG_FET3  //paired with OUT_FET4
#define CHG_FET4  //paired with OUT_FET4

#define CHG_FET5  //paired with OUT_FET6
#define CHG_FET6  //paired with OUT_FET6

#define CHG_FET7  //paired with OUT_FET8
#define CHG_FET8  //paired with OUT_FET8

#define CHG_FET9  //paired with OUT_FET9
#define CHG_FET10 //paired with OUT_FET9
*/

/* INTERRUPT BUTTON PIN ASSIGNMENTS */
//      NAME       DIGITAL PIN #    
#define BUTTON0     30 //60      //Calls battery case 0
#define BUTTON1     21 //31 //59      //Calls battery case 1
#define BUTTON2     32 //58      //Calls battery case 2
#define BUTTON3     33 //57      //Calls battery case 3
#define BUTTON4     34 //56      //Calls battery case 4
#define BUTTON5     35 //55      //Calls FULL_FET_DISCONNECT()

/* OUTPUT FET PIN ASSIGNMENTS*/
            //current pin assignments are for arduino ATMega
#define OUT_FET11   2
#define OUT_FET12   3
#define OUT_FET13   4
#define OUT_FET14   5
#define OUT_FET15   6
#define OUT_FET16   7
#define OUT_FET17   8
#define OUT_FET18   9
#define OUT_FET19  10
#define OUT_FET20  11
#define OUT_FET21  12
//NOTE: Digital Pin 1 is TX and messes everything up if its populated

/**
 * @brief as per arduino mega header file, these
 * are the integer values for the respective ADC inputs
 */
// ADC PIN DECLARATIONS (using pin numbers from ATMega2560)
#define ADC0 54  //Battery 1
#define ADC1 55  //Battery 2
#define ADC2 56  //Battery 3
#define ADC3 57  //Battery 4
#define ADC4 58  //Battery 5


/* FET CONFIGURATION TABLE
| Fet# | 0 | 1 | 2 | 3 | 4 | 5 |
|   1  |   | x |   |   |   |   | 
|   2  |   | x |   |   |   |   | 
|   3  |   |   | x |   |   |   | 
|   4  |   |   | x |   |   |   | 
|   5  |   |   |   | x |   |   | 
|   6  |   |   |   | x |   |   | 
|   7  |   |   |   |   | x |   | 
|   8  |   |   |   |   | x |   | 
|   9  |   |   |   |   |   | x | 
|  10  |   |   |   |   |   | x |
+------+---+---+---+---+---+---+ 
|  11  |   |   | x | x | x | x |
|  12  |   | x |   |   |   |   | 
|  13  |   |   |   | x | x | x | 
|  14  |   |   | x |   |   |   | 
|  15  |   | x |   |   | x | x | 
|  16  |   |   |   | x |   |   | 
|  17  |   | x | x |   |   | x | 
|  18  |   |   |   |   | x |   | 
|  19  |   | x | x | x |   |   | 
|  20  |   |   |   |   |   | x | 
|  21  |   | x | x | x | x |   | 
x = turn on
FETs 1-10 are charging FETs
FETs 11-21 are output FET pin assignments 2-11
*/

// global array continaing measured battery voltages
// used for determining the highest and lowest voltages of a single battery array with iterations
battery batts_array[NUM_BATTS]; // {batt_0, batt_1, ...}

/******************
 * SETUP FUNCITON *
 *****************/

void setup(){
    
    // Establish serial port for debugging
    if (DEBUG)
    {
        Serial.begin(19200);            //serial output for debugging
    }

    //integer values reflect arduino mega pins
    int adc_pins[5] = {A0, A1, A2, A3, A4}; //ADC pins are read as integers so we'll throw them in here for pin assignments 

    //Fet assignment array. The index of the array is the battery case. ie, [0][0] is case 0, [1][0] is case 1
    int FET_assignments[NUM_BATTS][NUM_FETS] = { 
        { OUT_FET12, OUT_FET15, OUT_FET17, OUT_FET19, OUT_FET21 }, 
        { OUT_FET11, OUT_FET14, OUT_FET17, OUT_FET19, OUT_FET21 },
        { OUT_FET11, OUT_FET13, OUT_FET16, OUT_FET19, OUT_FET21 },
        { OUT_FET11, OUT_FET13, OUT_FET15, OUT_FET18, OUT_FET21 },
        { OUT_FET11, OUT_FET13, OUT_FET15, OUT_FET17, OUT_FET20 } };

    //iterate over the array to perform pin assignments and set charging flags low
    for (int battery_case_index = 0; battery_case_index < NUM_BATTS; battery_case_index++)
    {
        //Since we are already iterating over the array, lets initialize the adc pin and the is_charging flag in the structure
        batts_array[battery_case_index].adc_pin_assignment = adc_pins[battery_case_index];
        Serial.print("Assigned ADC Pin: ");
        Serial.println(batts_array[battery_case_index].adc_pin_assignment);

        batts_array[battery_case_index].is_charging        = false;

        //itertate in 2d to assign FET config to each battery
        for (int FET_assignment_index = 0; FET_assignment_index < NUM_FETS ; FET_assignment_index++)
        {
            pinMode(FET_assignment_index, OUTPUT);  //might as well set the pins to outputs while we are iterating
        
            if (DEBUG)
            {
            Serial.print("Batt_Case ");
            Serial.print(battery_case_index);
            Serial.print(", OUT_FET ");
            Serial.print(FET_assignment_index);
            Serial.println(", configured to output");
            }
            //set the FETS array in the structure to the 2d element of the FET_assignments array
            batts_array[battery_case_index].FETS[FET_assignment_index] = FET_assignments[battery_case_index][FET_assignment_index];
        }
    }

    FULL_FET_DISCONNECT(); //initialize to full array disconnect
} //end setup


/****************
* MAIN FUNCTION *
****************/
//Main loop will continually poll the battery voltages
    // if a battery is too low, switch to charging that battery

/**
 * @brief Run through the battery cases to make sure everything works properly.
 * the FULL_FET_DISCONNECT and delay in between the case advance, is to reset the relay positions 
 * because the cases only set the relays high. Also included is a DEBUG function that allows for 
 * user serial input (keyboard). The terminal UI supplies a list of commands and their respective descriptions
 * the user can use to have the controller enter a specified state. 
 */


void loop(void){
    // Manual Debug Control
    if (DEBUG){

      // Manually change the input to BATT_CASE_SWITCH() to run through each case:
      Serial.println("Loop Debug");
      
      // Loop through each battery switch case:
      for(int index = 0; index < NUM_BATTS; index++){
        BATT_CASE_SWITCH(index);
        array_loaded_voltages();
        delay(2000);
      }
    }// end if DEBUG
  

    // Automatic MOSFET Control
    else {
        array_loaded_voltages(); //perform an array measurement

        // these limits are for loop comparison
        float min = 100;    
        float max = 0;     

        // storing the values of our min and max indices
        int max_batt_index = 0; 
        int min_batt_index = 0;

        //iterate over the battery array and find our min and max values and save respective indecies
        for (int i = 0; i < NUM_BATTS; i++) {
            
            //we want to ingore the lower battery if it's already being charged.
            if (batts_array[i].voltage_mes < min && !batts_array[i].is_charging) {
                min_batt_index = i;
                min = batts_array[i].voltage_mes;
            }
            //We only care if a battery is at it's max voltage if its charging. We'll probably run into issues here with HW -KS
            else if (batts_array[i].voltage_mes > max && batts_array[i].is_charging) {
                max_batt_index = i; 
                max = batts_array[i].voltage_mes;
            }
        }

        //check for a maxed or dead battery and charge the dead one, or take the max battery off and charge the battery with the lowest voltage
        if (batts_array[max_batt_index].voltage_mes >= BATT_MAX_VOLTS || batts_array[min_batt_index].voltage_mes <= BATT_FLOOR_VOLTS) {
            
            FULL_FET_DISCONNECT(); // disengage all FETS 

            //Use the min index to change the charging scheme to the lowest battery in the array
                    //we'll do this by passing the min battery index through the case switch
            BATT_CASE_SWITCH(min_batt_index);
        }
        else {
            BATT_CASE_SWTICH(0); //if all the batteries are evenly charged, then we'll just go to a default case so that nothing gets borked
        }    
    }//end else


} //end loop

/****************************
* BATTERY CASE DECLARATIONS *
****************************/
/**
 * @brief These functions will be called to change what battery is being charged
*/

//Default array case, everything low
void FULL_FET_DISCONNECT(){
    int output_fet_array[FET_ARRAY_LEN] = {
        OUT_FET11, OUT_FET12, OUT_FET13, 
        OUT_FET14, OUT_FET15, OUT_FET16, 
        OUT_FET17, OUT_FET18, OUT_FET19, 
        OUT_FET20, OUT_FET21 };

    if (DEBUG)
    {
         Serial.println("FETs disconnected");
    }

    for (int i = 0; i < FET_ARRAY_LEN; i++)
    {
        digitalWrite(output_fet_array[i], LOW);
    }
    
    delay(MOSFET_OFF_DELAY);

} //end FULL_FET_DISCONNECT


inline void BATT_CASE_SWTICH(int batt_case) {
    BATT_CASE_SWTICH(batt_case);
}
/***
 * This function handles setting the battery bypass
 * MOSFETS high. It does this by intaking a battery case number, then 
 * iterating over the FETS array inside the battery structure.
*/
void BATT_CASE_SWITCH(int batt_case){

    FULL_FET_DISCONNECT();

    for (int i = 0; i < NUM_FETS; i++)
    {
        digitalWrite(batts_array[batt_case].FETS[i], HIGH);
    }
    
    delay(MOSFET_ON_DELAY); //wait for the FETS to fully turn on
    batts_array[batt_case].is_charging = true; //raise is_charging flag specific case

    if(DEBUG){
      Serial.println("Case switched");
    }

} //end BATT_CASE_0


/*******************************
 * OTHER FUNCTION DECLARATIONS *
 *******************************/

void array_loaded_voltages(){

    for (int i = 0; i < NUM_BATTS; i++)
    {

        if(DEBUG){
          // Compute the voltage from the ADC output:
          batts_array[i].voltage_mes = (analogRead(batts_array[i].adc_pin_assignment) * ADC_CONVERS_FACT);

          // Print the raw ADC value and the converted analog voltage:
          Serial.print("Raw ADC Value: ");
          Serial.println(analogRead(batts_array[i].adc_pin_assignment));
          Serial.print("Battery Index: ");
          Serial.print(i);
          Serial.print(" , Measurement: ");
          Serial.println(batts_array[i].voltage_mes);
        }

        else{
          // Apply scale factor from differential voltage amplifiers:
          batts_array[i].voltage_mes = (analogRead(batts_array[i].adc_pin_assignment) * ADC_CONVERS_FACT ) / R_NET_SCALE_FACTOR;
        }
    }
}

void array_unloaded_voltages(){

    FULL_FET_DISCONNECT();  //disconnect batteries for some amount of time
    //delay(UNLOADED_VOLTAGE_MES_WAIT_TIME); // should not be needed since there is a delay to let all FETS disconnect
    /*
     * after the delay to let the battery voltages rest, we can call the array_loaded_voltages()
     * since this function just has added features compared to that function 
     */
    array_loaded_voltages();
}

