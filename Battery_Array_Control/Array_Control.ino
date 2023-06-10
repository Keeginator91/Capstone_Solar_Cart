/**
 * @file Array_Control.c
 * @author Keegan Smith (keeginator42@gmail.com)
 * @brief This file contains the main function to control and maintain the battery array
 * @version 0.1
 * @date 2023-06-06
 * 
 * @copyright Copyright (c) 2023
*/

/* INCLUDED LIBRARIES*/


/* LOCAL FILES*/
#include "Array_control.h"


/*************
 * CONSTANTS *
 *************/
bool DEBUG = false;

#define BATT_MAX_VOLTS
#define BATT_FLOOR_VOLTS

/******************
* PIN ASSIGNMENTS *
******************/

/* CHARGING FET PIN ASSIGNMENTS*/
/* These pin assignments are not likely to be driven
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
/*
#define ADC_1
#define ADC_2
#define ADC_3
#define ADC_4
#define ADC_5
*/

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
-------------------------------- 
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
FETs 11-21 are output FET pin assignments 1-11
*/

/******************
 * SETUP FUNCITON *
 *****************/

void setup(){
    if (DEBUG)
    {
        Serial.begin(19200);            //serial output for debugging
    }
    
    /* FET PIN CONFIGURATIONS */
    pinMode(OUT_FET1,  OUTPUT);
    pinMode(OUT_FET2,  OUTPUT);
    pinMode(OUT_FET3,  OUTPUT);
    pinMode(OUT_FET4,  OUTPUT);
    pinMode(OUT_FET5,  OUTPUT);
    pinMode(OUT_FET6,  OUTPUT);
    pinMode(OUT_FET7,  OUTPUT);
    pinMode(OUT_FET8,  OUTPUT);
    pinMode(OUT_FET9,  OUTPUT);
    pinMode(OUT_FET10, OUTPUT);

    /* ADC PIN CONFIGURATIONS*/

   BATT_CASE_0(); //initialize to full array disconnect
} //end setup


/****************
* MAIN FUNCTION *
****************/
//Main loop will continually poll the battery voltages
    // if a battery is too low, switch to charging that battery

/**
 * @brief Run through the battery cases to make sure everything works properly.
 * the BATT_CASE_0 and delay in between the case advance, is to reset the relay positions 
 * because the cases only set the relays high.
 */

void loop(void){
         
    BATT_CASE_0();
    delay(2500);
    //call batt case zero before switching, that way we only turn on the ones we need and there is 
        // enough time for switching

    BATT_CASE_1();
    delay(2500);

    BATT_CASE_0(); //reset relays
    delay(2500);   //wait a few seconds before advancing

    BATT_CASE_2();
    delay(2500);

    BATT_CASE_0(); //reset relays
    delay(2500);   //wait a few seconds before advancing

    BATT_CASE_3();
    delay(2500);

    BATT_CASE_0(); //reset relays
    delay(2500);   //wait a few seconds before advancing

    BATT_CASE_4();
    delay(2500);

    BATT_CASE_0(); //reset relays
    delay(2500);   //wait a few seconds before advancing

    BATT_CASE_5();
    delay(2500);

} //end loop


/****************************
* BATTERY CASE DECLARATIONS *
****************************/
/**
 * @brief These functions will be called to change what battery is being charged
*/

//Default array case, everything low
void BATT_CASE_0(){

    if (DEBUG)
    {
         Serial.println("BATT_CASE_0");
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

} //end BATT_CASE_0


void BATT_CASE_1(){
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

} //end BATT_CASE_1


void BATT_CASE_2(){
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

} //end BATT_CASE_2


void BATT_CASE_3(){
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

} //end BATT_CASE_3


void BATT_CASE_4(){
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

} //end BATT_CASE_4


void BATT_CASE_5(){
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

} //end BATT_CASE_5

/*******************************
 * OTHER FUNCTION DECLARATIONS *
 *******************************/

//end Battery_Array_Control.c