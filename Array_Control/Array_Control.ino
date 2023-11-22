/**
 * @file Array_Control.c
 * @author Keegan Smith (keeginator42@gmail.com), Matthew DeSantis, Thomas Cecelya
 * @brief This file contains the main function to control and maintain the battery array
 * @version 0.4
 * @date 2023-09-27
 * 
 * @copyright Copyright (c) 2023
*/

/* INCLUDED LIBRARIES*/
#include <stdbool.h>

/* LOCAL FILES*/
#include "Array_control.h"


/*************
 * CONSTANTS *
 *************/
bool DEBUG = true;

#define NUM_BATTS 5  //Number of batteries in the array

//adc conversion constants
#define ADC_RESOLUTION 1023 //max value adc will return
#define REF_VOLT          5 //reference voltage value
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
#define MOSFET_ON_DELAY (MOSFET_TURN_ON + MOSTFET_ON_DELAY)
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

/* OUTPUT FET PIN ASSIGNMENTS*/
            //current pin assignments are for arduino ATMega
#define OUT_FET1   2
#define OUT_FET2   3
#define OUT_FET3   4
#define OUT_FET4   5
#define OUT_FET5   6
#define OUT_FET6   7
#define OUT_FET7   8
#define OUT_FET8   9
#define OUT_FET9  10
#define OUT_FET10 11
//NOTE: Digital Pin 1 is TX and messes everything up if its populated

/* ADC PIN DECLARATIONS*/
#define BATT_TAP_1 A0  //Battery 1
#define BATT_TAP_2 A1  //Battery 2
#define BATT_TAP_3 A2  //Battery 3
#define BATT_TAP_4 A3  //Battery 4
#define BATT_TAP_5 A4  //Battery 5


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
    int adc_pins = {A0, A1, A2, A3, A4 }; //ADC pins are read as integers so we'll throw them in here for pin assignments 
                                                //vs code won't like this, but A0, A1 etc. are passed into analogRead(pin) as an int
    if (DEBUG)
    {
        Serial.begin(19200);            //serial output for debugging
    }
    
    /* FET PIN CONFIGURATIONS */
    for (int i = 2; i <= 11; i++)
    {
        pinMode(i, OUTPUT);
        if (DEBUG)
        {
            Serial.println("OUT_FET%d, config to output", i - 1);
        }
        
    }
    
    //sei(); //Enable interrupts


    /* ADC PIN CONFIGURATIONS*/
    analogReference(DEFAULT); //arduino specific function, 5V for Vref-pin of ADC

    //initialize the adc pin and the is_charging flag in the structure
    for (int i = 0; i < NUM_BATTS; i++)
    {
        batts_array[i].adc_pin_assignment = adc_pins[i]; //we'll fill the pin assignment in the structure from the adc pin array 
                                                            //the other way we could do this is brute force
        batts_array[i].is_charging = false;
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
 * because the cases only set the relays high.
 */


void loop(void){
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
        switch (min_batt_index)
        {
            case 0:
                BATT_CASE_0();
                break;

            case 1:
                BATT_CASE_1();
                break;

            case 2:
                BATT_CASE_2();
                break;

            case 3:
                BATT_CASE_3();
                break;

            case 4:
                BATT_CASE_4();
                break;
            
            default:
                //Catistrophic failure. Restart the system by calling loop.
                if(DEBUG) Serial.println("ERROR: min_bat_index out of bounds. Restarting system...");
                FULL_FET_DISCONNECT(); //make sure we disconnected everything so we don't blow up
                loop(); //Call loop to restart program
        }
    }

    else {
        BATT_CASE_0(); //if all the batteries are evenly charged, then we'll just go to a defualt case so that nothing gets borked
    }    
} //end loop


/**********************
* INTERRUPT FUNCTIONS *
**********************/

/****************************
* BATTERY CASE DECLARATIONS *
****************************/
/**
 * @brief These functions will be called to change what battery is being charged
*/

//Default array case, everything low
void FULL_FET_DISCONNECT(){

    if (DEBUG)
    {
         Serial.println("FETs disconnected");
    }

    digitalWrite(OUT_FET1,  LOW);
    digitalWrite(OUT_FET2,  LOW);
    digitalWrite(OUT_FET3,  LOW);
    digitalWrite(OUT_FET4,  LOW);
    digitalWrite(OUT_FET5,  LOW);
    digitalWrite(OUT_FET6,  LOW);
    digitalWrite(OUT_FET7,  LOW);
    digitalWrite(OUT_FET8,  LOW);
    digitalWrite(OUT_FET9,  LOW);
    digitalWrite(OUT_FET10, LOW);

    delay(MOSFET_OFF_DELAY);

} //end FULL_FET_DISCONNECT

/**
 * Charging battery 0
*/
void BATT_CASE_0(){
    //ENGAGE OUTPUT FETS: 2, 5, 7, 9, 10
    //ENGAGE CHARGING FETS: 1, 2
    if (DEBUG)
    {
        Serial.println("BATT_CASE_1");
    }
    
    digitalWrite(OUT_FET2,  HIGH);
    digitalWrite(OUT_FET5,  HIGH);
    digitalWrite(OUT_FET7,  HIGH);
    digitalWrite(OUT_FET9,  HIGH);
    digitalWrite(OUT_FET10, HIGH);

    delay(MOSFET_ON_DELAY);

    batts_array[0].is_charging = true; //raise is_charging flag

} //end BATT_CASE_0

void BATT_CASE_1(){
    //ENGAGE OUTPUT FETS: 1, 4, 7, 9, 10
    //ENGAGE CHARGING FETS: 3, 4
    if (DEBUG)
    {
        Serial.println("BATT_CASE_2");
    }

    digitalWrite(OUT_FET1,  HIGH);
    digitalWrite(OUT_FET4,  HIGH);
    digitalWrite(OUT_FET7,  HIGH);
    digitalWrite(OUT_FET9,  HIGH);
    digitalWrite(OUT_FET10, HIGH);

    delay(MOSFET_ON_DELAY);

    batts_array[1].is_charging = true; //raise is_charging flag
}

void BATT_CASE_2(){
    //ENGAGE OUTPUT FETS: 1, 3, 6, 9, 10
    //ENGAGE CHARGING FETS: 5, 6
    if (DEBUG)
    {
        Serial.println("BATT_CASE_3");
    }
    
    digitalWrite(OUT_FET1,  HIGH);
    digitalWrite(OUT_FET3,  HIGH);
    digitalWrite(OUT_FET6,  HIGH);
    digitalWrite(OUT_FET9,  HIGH);
    digitalWrite(OUT_FET10, HIGH);

    delay(MOSFET_ON_DELAY);

    batts_array[2].is_charging = true; //raise is_charging flag

} //end BATT_CASE_2

void BATT_CASE_3(){
    //ENGAGE OUTPUT FETS: 1, 3, 5, 8, 10
    //ENGAGE CHARGING FETS: 7, 8
    if (DEBUG)
    {
        Serial.println("BATT_CASE_4");
    }
    
    digitalWrite(OUT_FET1,  HIGH);
    digitalWrite(OUT_FET3,  HIGH);
    digitalWrite(OUT_FET5,  HIGH);
    digitalWrite(OUT_FET8,  HIGH);
    digitalWrite(OUT_FET10, HIGH);

    delay(MOSFET_ON_DELAY);

    batts_array[3].is_charging = true; //raise is_charging flag

} //end BATT_CASE_3

void BATT_CASE_4(){
    //ENGAGE OUTPUT FETS: 1, 3, 5, 7, 9
    //ENGAGE CHARGING FETS: 9, 10
    if (DEBUG)
    {
        Serial.println("BATT_CASE_5");
    }
        
    digitalWrite(OUT_FET1, HIGH);
    digitalWrite(OUT_FET3, HIGH);
    digitalWrite(OUT_FET5, HIGH);
    digitalWrite(OUT_FET7, HIGH);
    digitalWrite(OUT_FET9, HIGH);

    delay(MOSFET_ON_DELAY);

    batts_array[4].is_charging = true; //raise is_charging flag

} //end BATT_CASE_4

/*******************************
 * OTHER FUNCTION DECLARATIONS *
 *******************************/

void array_loaded_voltages(){

    for (int i = 0; i < NUM_BATTS; i++)
    {
        batts_array[i].voltage_mes = ( analogRead(batts_array[i].adc_pin_assignment) * ADC_CONVERS_FACT ) / R_NET_SCALE_FACTOR;
    }
}

void array_unloaded_voltages(){

    FULL_FET_DISCONNECT();  //disconnect batteries for some amount of time
    delay(UNLOADED_VOLTAGE_MES_WAIT_TIME);
    /*
     * after the delay to let the battery voltages rest, we can call the array_loaded_voltages()
     * since this function just has added features compared to that function 
     */
    array_loaded_voltages();
    
}//end Array_Control.c