/**
 * @file Array_Control.c
 * @author Keegan Smith (keeginator42@gmail.com), Matthew DeSantis, Thomas Cecelya
 * @brief This file contains the main function to control and maintain the battery array
 * @version 0.12
 * @date 2023-12-20
 * 
 * @copyright Copyright (c) 2023
*/

/* INCLUDED LIBRARIES*/
#include <stdbool.h>

/* LOCAL FILES*/
#include "Array_control.h"

/****************
 * LOCAL MACROS *
 ****************/

/* CHARGING FET PIN ASSIGNMENTS*/
//assign CHG_FET macro with a U09PD.. PCB pin ID
#define CHG_FET1     U09PD2             // Battery 1 Positive (solar panel positive)
#define CHG_FET2     U09PD5             // Battery 1 Negative (solar panel negative)

#define CHG_FET3     U09PD8  
#define CHG_FET4     U09PD9  

#define CHG_FET5     U09PD12  
#define CHG_FET6     U09PD22 

#define CHG_FET7     U09PD24 
#define CHG_FET8     U09PD25  

#define CHG_FET9     U09PD27 
#define CHG_FET10    U09PD29

/* OUTPUT FET PIN ASSIGNMENTS*/
//assign OUT_FET.. with a U09PD.. PCB pin ID
#define OUT_FET11    U09PD4
#define OUT_FET12    U09PD3
#define OUT_FET13    U09PD7
#define OUT_FET14    U09PD6   
#define OUT_FET15    U09PD11   
#define OUT_FET16    U09PD10  
#define OUT_FET17    U09PD23   
#define OUT_FET18    U09PD13   
#define OUT_FET19    U09PD28 
#define OUT_FET20    U09PD26 
#define OUT_FET21    U09PD30  

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

/*************
 * CONSTANTS *
 *************/
bool DEBUG = true;

battery batts_array[NUM_BATTS]; // {batt_0, batt_1, ...}

/**GLOBAL PIN ASSIGNMENT ARRAYS*/
        //output fets
const int output_fet_array[NUM_OUT_FETS] = {
    OUT_FET11, OUT_FET12, OUT_FET13, 
    OUT_FET14, OUT_FET15, OUT_FET16, 
    OUT_FET17, OUT_FET18, OUT_FET19, 
    OUT_FET20, OUT_FET21 
};
        //charing fets
const int charging_fet_array[NUM_CHG_FETS] = {
    CHG_FET1, CHG_FET2, CHG_FET3, CHG_FET4,
    CHG_FET5, CHG_FET6, CHG_FET7, CHG_FET8, 
    CHG_FET9, CHG_FET10
};
        //adc pins
const int adc_pins[5] = {ADC0, ADC1, ADC2, ADC3, ADC4};

/** GLOBAL OUTPUT FET ASSIGNMENT ARRAY*/
    //Used to assign what MOSFETS to their respective battery cases.
    // The index of the array is [Battery case][FETs to assign to case]
const int FET_assignments[NUM_BATTS][NUM_OUT_FETS_PER_BATT] = { 
  /*[0][0] case: 0*/  { OUT_FET12, OUT_FET15, OUT_FET17, OUT_FET19, OUT_FET21 }, 
  /*[1][0] case: 1*/  { OUT_FET11, OUT_FET14, OUT_FET17, OUT_FET19, OUT_FET21 },
  /*[2][0] case: 2*/  { OUT_FET11, OUT_FET13, OUT_FET16, OUT_FET19, OUT_FET21 },
  /*[3][0] case: 3*/  { OUT_FET11, OUT_FET13, OUT_FET15, OUT_FET18, OUT_FET21 },
  /*[4][0] case: 4*/  { OUT_FET11, OUT_FET13, OUT_FET15, OUT_FET17, OUT_FET20 } 
};


/******************
 * SETUP FUNCITON *
 *****************/

void setup(){
    
    // FIX
    Serial.begin(19200);            //serial output for debugging 

    //respective FET assignment functions
    assign_output_fets(); //also assigns ADC pin 
    assign_charge_fets();

    // Establish serial port for debugging
    if (DEBUG)
    {
        Serial.begin(19200);            //serial output for debugging

        //Formatted serial output of assignment functions results. Using one line for each
            //"printf" function being translated to Serial.print
        for(int i = 0; i < NUM_BATTS; i++){

            //printf("Battery %d configuration\n", i);                      // <- this is the printf we are translating
            Serial.print("Battery "); Serial.print(i); Serial.println(" configuration"); // <- translated printf funciton

            //printf("    is_charging: %d\n", batts_array[i].is_charging);
            Serial.print("      is_charging: "); Serial.println(batts_array[i].is_charging);

            //printf("    adc_pin_assignment: %d\n", batts_array[i].adc_pin_assignment);
            Serial.print("      adc_pin_assignement: "); Serial.println(batts_array[i].adc_pin_assignment);

            //printf("    OUT_FETS: ");
            Serial.print("      OUT_FETS: ");
            
            for(int ii = 0; ii < NUM_OUT_FETS_PER_BATT; ii++){
                //printf("%d ", batts_array[i].OUT_FETS[ii]);
                Serial.print(batts_array[i].OUT_FETS[ii]); Serial.print(" ");
            }
            //printf("\n");
            Serial.println(" ");

            //printf("  CHARGE_FETS: ");
            Serial.print("      CHARGE_FETS: ");

            for(int ii = 0; ii < NUM_CHG_FETS_PER_BATT; ii++){
                //printf("%d ", batts_array[i].CHARGE_FETS[ii]);
                Serial.print(batts_array[i].CHARGE_FETS[ii]); Serial.print(" ");
            }
            //printf("\n");
            Serial.println(" ");
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
      BATT_CASE_SWITCH(1);

      array_loaded_voltages();

        while (1)
        {
            //endless wait while we manually use interrupts
                //to switch between function
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
            // if (batts_array[i].voltage_mes < min && !batts_array[i].is_charging) {
            if (batts_array[i].voltage_mes < min) {
                min_batt_index = i;
                min = batts_array[i].voltage_mes;
            }
            //We only care if a battery is at it's max voltage if its charging. We'll probably run into issues here with HW -KS
            else if (batts_array[i].voltage_mes > max && batts_array[i].is_charging) {
                max_batt_index = i; 
                max = batts_array[i].voltage_mes;
            }
        }

        // ******** FIX ******** //
        Serial.print("Battery with minimum voltage: ");
        Serial.println(min_batt_index);

        //check for a maxed or dead battery and charge the dead one, or take the max battery off 
            //and charge the battery with the lowest voltage
        // if (batts_array[max_batt_index].voltage_mes >= BATT_MAX_VOLTS 
        //            || batts_array[min_batt_index].voltage_mes <= BATT_FLOOR_VOLTS) {
            
        //    FULL_FET_DISCONNECT(); // disengage all FETS 

            //Use the min index to change the charging scheme to the lowest battery in the array
                    //we'll do this by passing the min battery index through the case switch
        //  BATT_CASE_SWITCH(min_batt_index);
        //}
        //else {
        //    BATT_CASE_SWTICH(0); //if all the batteries are evenly charged, then we'll just go to a default case so that nothing gets borked
        //}  

        FULL_FET_DISCONNECT(); // disengage all FETs prior to changing case

        BATT_CASE_SWITCH(min_batt_index); // Switch to the case with the lowest measured battery terminal voltage

        delay(60000);
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

    if (DEBUG)
    {
         Serial.println("FETs disconnected");
    }

    // Disengage "charge" MOSFETs connecting batteries to solar panel
    for (int i = 0; i < NUM_CHG_FETS; i++)
    {
        digitalWrite(charging_fet_array[i], LOW);
    }

    delay(MOSFET_OFF_DELAY);
    
    // Disengage series "output" MOSFETs
    for (int i = 0; i < NUM_OUT_FETS; i++)
    {
        digitalWrite(output_fet_array[i], LOW);
    }
    
    delay(MOSFET_OFF_DELAY);
} //end FULL_FET_DISCONNECT


inline void BATT_CASE_SWTICH(int batt_case){
    BATT_CASE_SWITCH(batt_case);
}
/***
 * This function handles setting the battery bypass
 * MOSFETS high. It does this by intaking a battery case number, then 
 * iterating over the FETS array inside the battery structure.
*/
void BATT_CASE_SWITCH(int batt_case){

    FULL_FET_DISCONNECT();

    for (int i = 0; i < NUM_OUT_FETS_PER_BATT; i++)
    {
        digitalWrite(batts_array[batt_case].OUT_FETS[i], HIGH);
    }
    
    delay(MOSFET_ON_DELAY); //wait for the FETS to fully turn on

    for (int i = 0; i < NUM_CHG_FETS_PER_BATT; i++)
    {
        digitalWrite(batts_array[batt_case].CHARGE_FETS[i], HIGH);
    }

    delay(MOSFET_ON_DELAY); //wait for the FETS to fully turn on
    batts_array[batt_case].is_charging = true; //raise is_charging flag specific case

    if(DEBUG){
      Serial.print("Case ");
      Serial.print(batt_case);
      Serial.print(": \n");
      Serial.println("Case switched");
    }

} //end BATT_CASE_0


/*************************
 * MEASUREMENT FUNCTIONS *
 *************************/

void array_loaded_voltages(){

    for (int i = 0; i < NUM_BATTS; i++)
    {
        // batts_array[i].voltage_mes = (analogRead(batts_array[i].adc_pin_assignment) * ADC_CONVERS_FACT ) / R_NET_SCALE_FACTOR;
        batts_array[i].voltage_mes = (analogRead(batts_array[i].adc_pin_assignment) * ADC_CONVERS_FACT );
        
        if(DEBUG){
          Serial.print("Raw ADC Value: ");
          Serial.println(analogRead(batts_array[i].adc_pin_assignment));
          Serial.print("Battery Index: ");
          Serial.print(i);
          Serial.print(" , Measurement: ");
          Serial.println(batts_array[i].voltage_mes);
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

/****************************
 * TERTIARY SETUP FUNCTIONS *
 ****************************/

void assign_output_fets(){

    //iterate over the batts_array and x of FET_assignments to perform pin assignments and set charging flags low
    for (int battery_case_index = 0; battery_case_index < NUM_BATTS; battery_case_index++)
    {
        //Since we are already iterating over the array, lets initialize the adc pin and the is_charging flag in the structure
        batts_array[battery_case_index].adc_pin_assignment = adc_pins[battery_case_index];
        batts_array[battery_case_index].is_charging = false;    //nothing is charging, initialize to 0

        //itertate in y of FET_assignments to assign FET config to each battery
        for (int FET_assignment_index = 0; FET_assignment_index < NUM_OUT_FETS_PER_BATT ; FET_assignment_index++)
        {
            //a little clunky, but beats having another for loop 
            pinMode(FET_assignments[battery_case_index][FET_assignment_index], OUTPUT);  
                    
            //set the FETS array in the structure to the 2nd element of the FET_assignments array
            batts_array[battery_case_index].OUT_FETS[FET_assignment_index] = FET_assignments[battery_case_index][FET_assignment_index];
        }
    }
}

void assign_charge_fets(){

    int j = 0;
    int ii = 0;

    for (int i = 0; i < NUM_CHG_FETS; i++, ii++)
    {
        pinMode(charging_fet_array[i], OUTPUT);

        if ( (i % 2 == 0) && (i != 0) )
        {
            j++;
            ii = 0;
        }
        batts_array[j].CHARGE_FETS[ii] = charging_fet_array[i];        
    }
}