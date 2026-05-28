/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.02
	Date:		17.01.2022
	-----------------------------------------
	ATMega 128 Micro controller support
	-----------------------------------------
	   PI controller C-code file
	=========================================
*/

//===========================================================================================
// INCLUDE Header

#include "MPPT_PCB_MCU__Main.h"
#include "MPPT_PCB_MCU__Com.h"
#include "MPPT_PCB_MCU__IO.h"
#include "MPPT_PCB_MCU__EROM.h"
#include "MPPT_PCB_MCU__LOAD_CTR.h"
#include "MPPT_PCB_MCU__Range.h"
#include "MPPT_PCB_MCU__PI_CTR.h"
#include "MPPT_PCB_MCU__IV_Trans.h"
#include "MPPT_PCB_MCU__TEMP.h"

//===========================================================================================
// MEMSPACE DATA

//PROGMEM  const uint16_t SINE_DATA[] = {0,1,3}

//===========================================================================================
// VARIABLES and STRUCTURES

// PI controller config variables
volatile uint8_t PI_CTR_Prop_ID_Act;
volatile uint8_t PI_CTR_Prop_ID_Man;
volatile uint8_t PI_CTR_Int_ID_Act;
volatile uint8_t PI_CTR_Int_ID_Man;


//===========================================================================================
// EEPROM VARIABLES
#ifdef PCBconfig_Current_is_150mA
	EEMEM uint8_t EROM_PI_CTR_Prop_ID = 2;
	EEMEM uint8_t EROM_PI_CTR_Int_ID = 2;
#endif /* PCBconfig_Current_is_150mA */

#ifdef PCBconfig_Current_is_340mA
EEMEM uint8_t EROM_PI_CTR_Prop_ID = 2;
EEMEM uint8_t EROM_PI_CTR_Int_ID = 2;
#endif /* PCBconfig_Current_is_340mA */

#ifdef PCBconfig_Current_is_15A
	EEMEM uint8_t EROM_PI_CTR_Prop_ID = 3;
	EEMEM uint8_t EROM_PI_CTR_Int_ID = 2;
#endif /* PCBconfig_Current_is_15A */


//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Set Gain and Integrator multiplexers
void PI_CRT__Process_Controller_Settings(){
	/* this function processes the controller settings
	- does not do much right now, but here for extension of dynamic PI regulation */
	
	// Set proportional resistor
	if (PI_CTR_Prop_ID_Act != PI_CTR_Prop_ID_Man) {
		PI_CTR__Set_New_Prop_ID(PI_CTR_Prop_ID_Man);
	}
	// Set integrator capacitor
	if (PI_CTR_Int_ID_Act != PI_CTR_Int_ID_Man) {
		PI_CTR__Set_New_Int_ID(PI_CTR_Int_ID_Man);
	}
	
}

//-------------------------------------------------------------------------------------------
// Set new proportional resistor from ID
void PI_CTR__Set_New_Prop_ID(uint8_t Prop_ID){
	/* controls the multiplexer to change the proportional resistor */
	
	// set GPIO variables dependent on ID given	
	 if (Prop_ID == 0) {
		CLR__PI_Var_Prop_A0;
		CLR__PI_Var_Prop_A1;
	 }
	 else if (Prop_ID == 1) {
		 SET__PI_Var_Prop_A0;
		 CLR__PI_Var_Prop_A1;
	 }
	 else if (Prop_ID == 2) {
		 CLR__PI_Var_Prop_A0;
		 SET__PI_Var_Prop_A1;
	 }
	 else if (Prop_ID == 3) {
		 SET__PI_Var_Prop_A0;
		 SET__PI_Var_Prop_A1;
	 }
	 else {
		 SET__PI_Var_Prop_A0;
		 SET__PI_Var_Prop_A1;
	 }
	
	// Transfer new DO line state to GPIOE
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	// Wait a bit
	_delay_us(100);
	// store new setting
	PI_CTR_Prop_ID_Act = Prop_ID;
	
}

//-------------------------------------------------------------------------------------------
// Set new integrator capacitor from ID
void PI_CTR__Set_New_Int_ID(uint8_t Int_ID){
	/* controls the multiplexer to change the integrator capacitor */
	
	// set GPIO variables dependent on ID given
	if (Int_ID == 0) {
		CLR__PI_Var_Int_A0;
		CLR__PI_Var_Int_A1;
	}
	else if (Int_ID == 1) {
		SET__PI_Var_Int_A0;
		CLR__PI_Var_Int_A1;
	}
	else if (Int_ID == 2) {
		CLR__PI_Var_Int_A0;
		SET__PI_Var_Int_A1;
	}
	else if (Int_ID == 3) {
		SET__PI_Var_Int_A0;
		SET__PI_Var_Int_A1;
	}
	else {
		SET__PI_Var_Int_A0;
		SET__PI_Var_Int_A1;
	}
	
	// Transfer new DO line state to GPIOE
	DIOExp_Write_Outputs(DIO_REG_GPOI, DIOE_PORT_A_STATE, DIOE_PORT_B_STATE);
	// Wait a bit
	_delay_us(100);
	// store new setting
	PI_CTR_Int_ID_Act = Int_ID;
	
}
