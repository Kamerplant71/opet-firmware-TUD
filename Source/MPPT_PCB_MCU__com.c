/* 
	-----------------------------------------
	MPPT PCB MCU source
	-----------------------------------------
	Made by:	Martin Bliss
	Version:	1.15
	Date:		02.03.2024
	-----------------------------------------
	ATMega 324 Micro controller support
	-----------------------------------------
	  Communication function C-Code FILE
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
// VARIABLES and STRUCTURES
volatile char UART_In_Buff[UART_COM_LENGH+1];		// data input buffer (incomplete transfer data)
volatile uint8_t UART_In_Buff_L;
volatile char UART_Comand[UART_COM_LENGH+1] ;		// complete input command buffer (single)
volatile uint8_t UART_Comand_L;
volatile char OutSTR[UART_COM_LENGH+1];			// command reply buffer (not in transfer)
volatile uint8_t OutSTR_L;		// actual length of the OutSTR, used to efficiently add characters without searching
volatile char UART_Reply[UART_COM_LENGH+1];		// data output buffer 1(data in transfer over UART)
volatile uint8_t UART_Reply_P;
volatile uint8_t UART_Reply_L;
volatile uint8_t Com_ID;
volatile uint8_t Trans_IV_Point;


//Convert 16to8 bit integer
typedef union {
	uint16_t i16;
	struct {
		uint8_t i8l;
		uint8_t i8h;
	};
} convert16to8;


//===========================================================================================
// EEPROM VARIABLES

//===========================================================================================
// FUNCTIONS
//===========================================================================================
//-------------------------------------------------------------------------------------------
// UART Process Input
void UART_Process_Input () {
	/* never mind its old & messy but somehow still works, it separates address & value from a command */
	
	char Address[UART_ADDRESS_LENGH+1];	
	char Value[UART_Value_LENGH+1];
	uint8_t Adr_Str_Pos;
	uint8_t Val_Str_Pos;
	uint8_t Com_Str_Pos;
	uint8_t UART_Com_ID;
	Adr_Str_Pos = 0;
	Val_Str_Pos = 0;
	Com_Str_Pos = 0;
	UART_Com_ID = 0;

	//UART_WriteString (&UART_Comand[0]);	// for debugging

	// Extract URART Communication ID
	if (UART_Comand[1] == UART_ADR_SEP_CHAR) {	// second character [1] address separation
		UART_Com_ID = UART_Comand[0]; 			// set URART communication address [0]
		Com_Str_Pos = 1;
	}
	else {	
		goto UART_Process_Input_END;		// no address found abort
	}

	// check if correct device is requested
	if (UART_Com_ID == Com_ID){			
		// Extract Address
		Com_Str_Pos++;		// skip ADR sep character
		while (UART_Comand[Com_Str_Pos] != UART_SEP_CHAR) {
			if (UART_Comand[Com_Str_Pos] == '\0') {	//Check if end of string (only address)
				Address[Adr_Str_Pos] = '\0';		// set end of string Address
				Value[Val_Str_Pos] = '\0';			// set end of string Value
				goto UART_Execute_Input;					// start process input without value
			}
			if (Com_Str_Pos >= UART_Comand_L) {		// Check UART command length exceeded
				goto UART_Process_Input_END;		// no valid command
			}
			Address[Adr_Str_Pos] = UART_Comand[Com_Str_Pos];
			Com_Str_Pos++;
			Adr_Str_Pos++;
			if(Adr_Str_Pos >= UART_ADDRESS_LENGH) {
				UART_WriteString ("ERR_ADR!");
				goto UART_Process_Input_END;
			}
		}
		Address[Adr_Str_Pos] = '\0';
		//UART_WriteString (&Address[0]);		// for debugging

		// Extract Value
		Com_Str_Pos++;		// skip value separation character
		if (Adr_Str_Pos <= UART_Comand_L) {
			while (UART_Comand[Com_Str_Pos] != '\0') {	
				Value[Val_Str_Pos] = UART_Comand[Com_Str_Pos];
				Com_Str_Pos++;
				Val_Str_Pos++;
				if(Val_Str_Pos >= UART_Value_LENGH) {
					UART_WriteString ("ERR_VAL!");
					goto UART_Process_Input_END;
				}
			}
			Value[Val_Str_Pos] = '\0';
			//UART_WriteString (&Value[0]);		// for debugging
		}
		else {
			Value[0] = '\0';
			//Value[1] = '\0';
			//UART_WriteString ("No Value");	// for debugging
		}
	}
	else {
		goto UART_Process_Input_END;
	}

	//Execute Command
	UART_Execute_Input:;
	//UART_WriteString (&UART_Comand[0]);  	// for debugging
	//UART_WriteString (&Address[0]);		// for debugging
	//UART_WriteString (&Value[0]);			// for debugging
	UART_Execute_Command(&Address[0],&Value[0]);

	//Comm Abort point
	UART_Process_Input_END:;
	
}

//-------------------------------------------------------------------------------------------
// EXECUTE Command
void UART_Execute_Command(char *Address, char *Value){
	
	char READ_IND_CHAR = UART_COM_READ_INDICATOR; // write to variable to use less memory
	uint8_t Last_OutSTR_L;
	// uint8_t Tmp_UI8;
	float Tmp_FLOAT;
	int32_t Tmp_I32;
		
	//##############################
	// Quick Reply / control Commands
	//------------------------------
	// READ Status & PV output data
	COM_Copy_To_OutSTR_From_Start("READ?");
	if (COM_Compare_to_OutStr(Address)) {
		convert16to8 SysStatus;
		SysStatus.i8l = SysStatus_A;
		SysStatus.i8h = SysStatus_B;
		itoa (SysStatus.i16, Value, 10);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_Volt);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_Curr_Corr);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_Offset);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_Bias);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_NTC_Temp_1);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_NTC_Temp_2);
		COM_Add_To_OutSTR_with_Sep(Value);
		if(is_SysConfig_TEMP_On) {
			FloatToString(Value, AI_RTD_Temp);
			COM_Add_To_OutSTR_with_Sep(Value);
		}
		UART_WriteString (&OutSTR[0]);
		CLR__Status_MainTimerOverRun; // clear timer overrun flag
		goto UART_Execute_Command_END;
	}
	//------------------------------
	// R/W - Control Output State
	COM_Copy_To_OutSTR_From_Start("OUTP");
	if (COM_Compare_to_OutStr(Address)) {
		if (Address[OutSTR_L] == '\0') {
			// Write data (no ? for read only)
			if (Value[0] == '1') {
				SET__SysControl_Ouput_On;
			}
			else if (Value[0] == '0') CLR__SysControl_Ouput_On;
			else goto UART_Execute_Command_ERROR;
		}
		else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
		else goto UART_Execute_Command_ERROR;
		// Return data read-back
		if(is_SysControl_Ouput_On) COM_Add_To_OutSTR_with_Sep("1");
		else COM_Add_To_OutSTR_with_Sep("0");
		UART_WriteString (&OutSTR[0]);
		goto UART_Execute_Command_END;
	}
	
	//##############################
	// System Root Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("*");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// R- *SBR? - READ System Status byte 
		COM_Add_To_OutSTR_At_Position("SBR?", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			convert16to8 SysStatus;
			SysStatus.i8l = SysStatus_A;
			SysStatus.i8h = SysStatus_B;
			itoa (SysStatus.i16, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			CLR__Status_MainTimerOverRun; // clear timer overrun flag
			goto UART_Execute_Command_END;
		}		
		//------------------------------
		// R - *OPC? - Operation Complete query
		COM_Add_To_OutSTR_At_Position("OPC?", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			COM_Copy_To_OutSTR_From_Start("1");
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R - *IDN? - read device ID with revision and sample name
		COM_Add_To_OutSTR_At_Position("IDN?", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			COM_Add_To_OutSTR_with_Sep(DeviceName);
			COM_Add_To_OutSTR_with_Sep(FirmwareVersion);
			COM_Add_To_OutSTR_with_Sep(Sample_Name);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// W - *RST - Reset Device
		COM_Add_To_OutSTR_At_Position("RST", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			COM_Add_To_OutSTR_with_Sep("1");
			UART_WriteString (&OutSTR[0]);
			while (UART_Reply_L != 0)	{ // Wait until buffer is empty
			}
			cli(); // Turn off interrupts
			goto *0; // Get to RESET point
		}
	}
	// END: System Root Commands
	//##############################	
	
	//##############################
	// Load Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("LOAD:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// R/W - LOAD:MODE - PV load mode control
		COM_Add_To_OutSTR_At_Position("MODE", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data, if number is invalid will return 0, meaning output turns off instantly
				PV_Mode_CTR = (uint8_t) atol(Value);
				itoa (PV_Mode_CTR, Value, 10);
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				itoa (PV_Mode_ACT, Value, 10); // return active LOAD mode
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - LOAD:SETVOLT - Voltage Set point, applied in manual mode or as start in transient
		COM_Add_To_OutSTR_At_Position("SETVOLT", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data, if number is invalid will return 0, meaning short circuit Set point
				PV_Volt_Setpoint = StringToFloat(Value);	//Read string to float
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			FloatToString(Value, PV_Volt_Setpoint);	//Write float back to string
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - LOAD:SETCURR - Current Set point for current load mode
		COM_Add_To_OutSTR_At_Position("SETCURR", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data, if number is invalid will return 0, meaning short circuit Set point
				PV_Curr_Setpoint = StringToFloat(Value);	//Read string to float
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			FloatToString(Value, PV_Curr_Setpoint);	//Write float back to string
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - LOAD:MPPT:DELAY - MPPT update delay in cycles 
		COM_Add_To_OutSTR_At_Position("MPPT:DELAY", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				// limit boundaries
				if (Tmp_I32 > 60000) LOAD_MPPT_Timer_Update_Delay = 60000;
				else if (Tmp_I32 <= 0) LOAD_MPPT_Timer_Update_Delay = 1;
				else LOAD_MPPT_Timer_Update_Delay = (uint16_t) Tmp_I32;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			ltoa (LOAD_MPPT_Timer_Update_Delay, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
	}
	// END: Load Control Commands
	//##############################
	
	//##############################
	// IV Trace Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("IV:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// W - IV:MEAS - Start IV Measurement 
		COM_Add_To_OutSTR_At_Position("MEAS", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data - no Value to read here
				if (Check_Temp_Disconnect_State() || is_Status_Bias_Error || !is_Output_On) {
					CLR__SysControl_IV_Meas;
					COM_Add_To_OutSTR_with_Sep("0");
				}
				else {
					SET__SysControl_IV_Meas;
					//TODO fix IV measurement time prediction calculation
					if (IV_Point_Set_Delay_ms >= IV_Setup_Max_Delay_For_Mul) IV_Setup_Signal_Set_Time = 60000;
					else IV_Setup_Signal_Set_Time = IV_Point_Set_Delay_ms * IV_Setup_Signal_Set_Multiplier + IV_Range_Set_Settle_Time_ms;
					Tmp_FLOAT =	(float) IV_Meas_Points * (float) IV_Point_Set_Delay_ms + (float) IV_Meas_Points * (float) IV_Point_Meas_Nu_AVR_Sets * 
								(0.22 + (float) IV_Point_Meas_Nu_AVR_VoltCurr * 0.022) + 200.0 + (4.0 * (float) IV_Setup_Signal_Set_Time) ; // rough calculation of IV measurement time in [ms]
					Tmp_I32 = Tmp_FLOAT;
					ltoa (Tmp_I32, Value, 10);
					COM_Add_To_OutSTR_with_Sep(Value);
				}
			}
			else goto UART_Execute_Command_ERROR;
			// Return handshake
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R - IV:DATA - Read/Transfer IV curve data (Initiate transfer with number of IV points, ...)
		COM_Add_To_OutSTR_At_Position("DATA", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == READ_IND_CHAR) {
				// read data - no Value to read here
				SET__COM_Status_IV_Data_Transfer;	// set to transferring IV data
				Trans_IV_Point = 0;
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				itoa (IV_Report, Value, 10);
				COM_Add_To_OutSTR_with_Sep(Value);
				if((SysConfig & BIT(0))){ //Add module temperature if available
					FloatToString(Value, AI_RTD_Temp);  
					COM_Add_To_OutSTR_with_Sep(Value);
				}

				UART_WriteString (&OutSTR[0]);
				CLR__COM_Status_IV_Buffer_Ready;
				CLR__Status_NewIvDataAvailable;
			}
			else goto UART_Execute_Command_ERROR;
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:POINTS - Number of IV points
		COM_Add_To_OutSTR_At_Position("POINTS", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				// limit boundaries
				if (Tmp_I32 >= MAX_IV_POINTS) IV_Meas_Points = MAX_IV_POINTS - 1;
				else if (Tmp_I32 <= MIN_IV_POINTS) IV_Meas_Points = MIN_IV_POINTS;
				else IV_Meas_Points = (uint8_t) Tmp_I32;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (IV_Meas_Points, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:DELAY - Settling delay in ms
		COM_Add_To_OutSTR_At_Position("DELAY", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				// limit boundaries
				if (Tmp_I32 > 60000) IV_Point_Set_Delay_ms = 60000;
				else if (Tmp_I32 <= 0) IV_Point_Set_Delay_ms = 1;
				else IV_Point_Set_Delay_ms = (uint16_t) Tmp_I32;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			ltoa (IV_Point_Set_Delay_ms, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:AVR:VC - number of average voltage current measurements per set
		COM_Add_To_OutSTR_At_Position("AVR:VC", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				// limit boundaries
				if (Tmp_I32 > 255) IV_Point_Meas_Nu_AVR_VoltCurr = 255;
				else if (Tmp_I32 <= 0) IV_Point_Meas_Nu_AVR_VoltCurr = 1;
				else IV_Point_Meas_Nu_AVR_VoltCurr = (uint8_t) Tmp_I32;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (IV_Point_Meas_Nu_AVR_VoltCurr, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:AVR:SETS - number of average voltage current measurements sets
		COM_Add_To_OutSTR_At_Position("AVR:SETS", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				// limit boundaries
				if (Tmp_I32 > 255) IV_Point_Meas_Nu_AVR_Sets = 255;
				else if (Tmp_I32 <= 0) IV_Point_Meas_Nu_AVR_Sets = 1;
				else IV_Point_Meas_Nu_AVR_Sets = (uint8_t) Tmp_I32;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (IV_Point_Meas_Nu_AVR_Sets, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:MODE - IV curve measurement mode byte
		COM_Add_To_OutSTR_At_Position("MODE", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				// limit boundaries
				if (Tmp_I32 >= 255 || Tmp_I32 < 0) COM_Add_To_OutSTR_with_Sep("?");
				else IV_Mode = (uint8_t) Tmp_I32;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (IV_Mode, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:COS:PHASE - cos IV mode phase angle at VOC
		COM_Add_To_OutSTR_At_Position("COS:PHASE", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				IV_Cos_Max_Phase = StringToFloat(Value);	//Read string to float
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			FloatToString(Value, IV_Cos_Max_Phase);	//Write float back to string
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - IV:VOC:MULT - overshoot control multiplier on VOC measurement
		COM_Add_To_OutSTR_At_Position("VOC:MULT", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				IV_Voc_Overshoot_Fact = StringToFloat(Value);	//Read string to float
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			FloatToString(Value, IV_Voc_Overshoot_Fact);	//Write float back to string
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
	}
	// END: IV Trace Control Commands
	//##############################
		
	//##############################
	// Transient Meas Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("TRANS:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// W - TRANS:MEAS - Start Transient Measurement
		COM_Add_To_OutSTR_At_Position("MEAS", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data - no Value to read here
				if (Check_Temp_Disconnect_State() || is_Status_Bias_Error || !is_Output_On) {
					CLR__SysControl_Trans_Meas;
					COM_Add_To_OutSTR_with_Sep("0");
				}
				else {
					SET__SysControl_Trans_Meas;
					//TODO fix Trans measurement time prediction calculation
					if (IV_Point_Set_Delay_ms >= IV_Setup_Max_Delay_For_Mul) IV_Setup_Signal_Set_Time = 60000;
					else IV_Setup_Signal_Set_Time = IV_Point_Set_Delay_ms * IV_Setup_Signal_Set_Multiplier + IV_Range_Set_Settle_Time_ms;
					Tmp_FLOAT =	(float) IV_Meas_Points * 10.0 + 200.0 + (4.0 * (float) IV_Setup_Signal_Set_Time) ; // rough calculation of Trans measurement time in [ms]
					Tmp_I32 = Tmp_FLOAT;
					ltoa (Tmp_I32, Value, 10);
					COM_Add_To_OutSTR_with_Sep(Value);
				}
			}
			else goto UART_Execute_Command_ERROR;
			// Return handshake
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - TRANS:ENDVOLT - Voltage END point - end point of voltage transient
		COM_Add_To_OutSTR_At_Position("ENDVOLT", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data, if number is invalid will return 0, meaning short circuit endpoint
				PV_Volt_Trs = StringToFloat(Value);	//Read string to float
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			FloatToString(Value, PV_Volt_Trs);	//Write float back to string
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - TRANS:DELAY - transient point delay in us
		COM_Add_To_OutSTR_At_Position("DELAY", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data, if number is invalid will return 0, meaning no delay (minimum possible)
				Trs_Delay_us = StringToFloat(Value);	//Read string to float
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) COM_Add_QuestMark_To_OutSTR_No_Sep();
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			FloatToString(Value, Trs_Delay_us);	//Write float back to string
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		
	}
	// END: Transient Meas Control Commands
	//##############################
			
	//##############################
	// Range Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("RANGE:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {	
		//------------------------------
		// R - RANGE:ACTVAL - Read active measurement range values
		COM_Add_To_OutSTR_At_Position("ACTVAL", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == READ_IND_CHAR) {
				FloatToString(Value, AI_Volt_Range_Val);
				COM_Add_To_OutSTR_with_Sep(Value);
				FloatToString(Value, AI_Curr_Range_Val);
				COM_Add_To_OutSTR_with_Sep(Value);
				UART_WriteString (&OutSTR[0]);
			}
			else goto UART_Execute_Command_ERROR;
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - RANGE:IDVOLT - Voltage Range ID
		COM_Add_To_OutSTR_At_Position("IDVOLT", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > Volt_range_max_ID || Tmp_I32 < 0){ // auto range setting
						CLR__R_Stat_Volt_R_Manual;
						Range_Volt_ID_Man = RANGE_ID_AUTORANGE;
						COM_Add_To_OutSTR_with_Sep("auto");
					}
					else{
						SET__R_Stat_Volt_R_Manual;
						Range_Volt_ID_Man = (uint8_t) Tmp_I32;
						itoa (Range_Volt_ID_Man, Value, 10);
						COM_Add_To_OutSTR_with_Sep(Value);
					}
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				if (Range_Volt_ID_Man == RANGE_ID_AUTORANGE) COM_Add_To_OutSTR_with_Sep("auto");
				else {
					itoa (Range_Volt_ID_Man, Value, 10); // return active LOAD mode
					COM_Add_To_OutSTR_with_Sep(Value);
				}
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - RANGE:IDCURR - Current Range ID
		COM_Add_To_OutSTR_At_Position("IDCURR", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > Curr_range_max_ID || Tmp_I32 < 0){ // auto range setting
						CLR__R_Stat_Curr_R_Manual;
						Range_Curr_ID_Man = RANGE_ID_AUTORANGE;
						COM_Add_To_OutSTR_with_Sep("auto");
					}
					else{
						SET__R_Stat_Curr_R_Manual;
						Range_Curr_ID_Man = (uint8_t) Tmp_I32;
						itoa (Range_Curr_ID_Man, Value, 10);
						COM_Add_To_OutSTR_with_Sep(Value);
					}
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				if (Range_Curr_ID_Man == RANGE_ID_AUTORANGE) COM_Add_To_OutSTR_with_Sep("auto");
				else {
					itoa (Range_Curr_ID_Man, Value, 10); // return active LOAD mode
					COM_Add_To_OutSTR_with_Sep(Value);
				}
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// W - RANGE:OCRST - Reset Over current clamp
		COM_Add_To_OutSTR_At_Position("OCRST", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data - no Value to read here
				Reset_Over_Current_Clamp();		// reset over-current clamp
			}
			else goto UART_Execute_Command_ERROR;
			// Return handshake
			COM_Add_To_OutSTR_with_Sep("1");
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// W - RANGE:ONVOLT - voltage range enabled byte for auto range
		COM_Add_To_OutSTR_At_Position("ONVOLT", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > 255 || Tmp_I32 < 0) Range_Volt_Enable = 255; // value out of range
					else Range_Volt_Enable = (uint8_t) Tmp_I32;
					itoa (Range_Volt_Enable, Value, 10); // return enabled byte
					COM_Add_To_OutSTR_with_Sep(Value);
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				itoa (Range_Volt_Enable, Value, 10); // return enabled byte
				COM_Add_To_OutSTR_with_Sep(Value);
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// W - RANGE:ONCURR - current range enabled byte for auto range
		COM_Add_To_OutSTR_At_Position("ONCURR", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > 255 || Tmp_I32 < 0) Range_Curr_Enable = 255; // value out of range
					else Range_Curr_Enable = (uint8_t) Tmp_I32;
					itoa (Range_Curr_Enable, Value, 10); // return enabled byte
					COM_Add_To_OutSTR_with_Sep(Value);
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				itoa (Range_Curr_Enable, Value, 10); // return enabled byte
				COM_Add_To_OutSTR_with_Sep(Value);
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// Range Calibration commands
		//------------------------------
		// RANGE:CAL
		COM_Add_To_OutSTR_At_Position("CAL:", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			Last_OutSTR_L = OutSTR_L;
			//------------------------------
			// R/W - RANGE:CAL:MODE - Calibration mode control
			COM_Add_To_OutSTR_At_Position("MODE", Last_OutSTR_L);
			if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
				if (Address[OutSTR_L] == '\0'){
					// Write Data
					SET__SysControl_Cal_Mode;
					if (Compare_String(Value, "VOLT", 4) == 1) {
						// Voltage Calibration
						SET__R_Stat_Cal_Mode_Volt_Act;
						CLR__R_Stat_Cal_Mode_Curr_Act;
						Range_Volt_ID_Act = 255; // flag reload of range data
					}
					else if (Compare_String(Value, "CURR", 4) == 1) {
						// Voltage Calibration
						SET__R_Stat_Cal_Mode_Curr_Act;
						CLR__R_Stat_Cal_Mode_Volt_Act;
						Range_Curr_ID_Act = 255; // flag reload of range data
					}
					else {
						// Exit calibration mode
						CLR__SysControl_Cal_Mode;
						if (is_R_Stat_Cal_Mode_Volt_Act) Range_Volt_ID_Act = 255; // flag reload of range data
						else if (is_R_Stat_Cal_Mode_Curr_Act) Range_Curr_ID_Act = 255; // flag reload of range data
						CLR__R_Stat_Cal_Mode_Volt_Act;
						CLR__R_Stat_Cal_Mode_Curr_Act;
					}
				}
				else if (Address[OutSTR_L] == READ_IND_CHAR) {
					// Read data
					COM_Add_QuestMark_To_OutSTR_No_Sep();
				}
				else goto UART_Execute_Command_ERROR;
				// Feedback reply
				if (is_SysControl_Cal_Mode) {
					if (is_R_Stat_Cal_Mode_Volt_Act) COM_Add_To_OutSTR_with_Sep("VOLT");
					else if (is_R_Stat_Cal_Mode_Curr_Act) COM_Add_To_OutSTR_with_Sep("CURR");
					else COM_Add_To_OutSTR_with_Sep("E?");
				}
				else COM_Add_To_OutSTR_with_Sep("0");
				UART_WriteString (&OutSTR[0]);
				goto UART_Execute_Command_END;
			}
			//------------------------------
			// R/W - RANGE:CAL:SCALE - Calibration Range Scale
			COM_Add_To_OutSTR_At_Position("SCALE", Last_OutSTR_L);
			if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
				if (Address[OutSTR_L] == '\0'){
					// Write Data
					Tmp_FLOAT = StringToFloat(Value);	//Read string to float
					if (is_SysControl_Cal_Mode) {
						if (is_R_Stat_Cal_Mode_Volt_Act || is_R_Stat_Cal_Mode_Curr_Act) {
							EEPROM_Range_Cal_Set_Scale(Tmp_FLOAT);
						}
					}
				}
				else if (Address[OutSTR_L] == READ_IND_CHAR) {
					// Read data
					COM_Add_QuestMark_To_OutSTR_No_Sep();
				}
				else goto UART_Execute_Command_ERROR;
				// Feedback reply
				if (is_SysControl_Cal_Mode) {
					if (is_R_Stat_Cal_Mode_Volt_Act || is_R_Stat_Cal_Mode_Curr_Act) {
						Tmp_FLOAT = EEPROM_Range_Cal_Get_Scale();
						FloatToString(Value, Tmp_FLOAT);
						COM_Add_To_OutSTR_with_Sep(Value);
					}
					else COM_Add_To_OutSTR_with_Sep("E?");
				}
				else COM_Add_To_OutSTR_with_Sep("No Cal!");
				UART_WriteString (&OutSTR[0]);
				goto UART_Execute_Command_END;
			}
			//------------------------------
			// R/W -RANGE:CAL:OFFSET - Calibration Range Offset
			COM_Add_To_OutSTR_At_Position("OFFSET", Last_OutSTR_L);
			if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
				if (Address[OutSTR_L] == '\0'){
					// Write Data
					Tmp_FLOAT = StringToFloat(Value);	//Read string to float
					if (is_SysControl_Cal_Mode) {
						if (is_R_Stat_Cal_Mode_Volt_Act || is_R_Stat_Cal_Mode_Curr_Act) {
							EEPROM_Range_Cal_Set_Offset(Tmp_FLOAT);
						}
					}
				}
				else if (Address[OutSTR_L] == READ_IND_CHAR) {
					// Read data
					COM_Add_QuestMark_To_OutSTR_No_Sep();
				}
				else goto UART_Execute_Command_ERROR;
				// Feedback reply
				if (is_SysControl_Cal_Mode) {
					if (is_R_Stat_Cal_Mode_Volt_Act || is_R_Stat_Cal_Mode_Curr_Act) {
						Tmp_FLOAT = EEPROM_Range_Cal_Get_Offset();
						FloatToString(Value, Tmp_FLOAT);
						COM_Add_To_OutSTR_with_Sep(Value);
					}
					else COM_Add_To_OutSTR_with_Sep("E?");
				}
				else COM_Add_To_OutSTR_with_Sep("No Cal!");
				UART_WriteString (&OutSTR[0]);
				goto UART_Execute_Command_END;
			}
		}
		// END: Range Calibration Commands
		//------------------------------		
	}
	// END: Range Control Commands
	//##############################

	//##############################
	// Fan Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("FAN:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// R/W - FAN:MODE - Set Fan control mode
		COM_Add_To_OutSTR_At_Position("MODE", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				Tmp_I32 = atol(Value);	//Read string to long integer
				if (Tmp_I32 == 0) { // Man mode fan off
					SET__FAN_MAN_Mode;
					CLR__FAN_ENABLED_Request;
					COM_Add_To_OutSTR_with_Sep("MAN OFF");
				}
				else if (Tmp_I32 == 1) { // Man mode fan off
					SET__FAN_MAN_Mode;
					SET__FAN_ENABLED_Request;
					COM_Add_To_OutSTR_with_Sep("MAN ON");
				}
				else {
					CLR__FAN_MAN_Mode;
					COM_Add_To_OutSTR_with_Sep("AUTO");
				}
				UART_WriteString (&OutSTR[0]);
				goto UART_Execute_Command_END;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) { // Return data read-back
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				if ((is_FAN_IS_ENABLED) && (is_FAN_MAN_Mode)) {
					COM_Add_To_OutSTR_with_Sep("MAN ON");
				}
				else if ((!is_FAN_IS_ENABLED) && (is_FAN_MAN_Mode)) {
					COM_Add_To_OutSTR_with_Sep("MAN OFF");
				}
				else if ((is_FAN_IS_ENABLED) && (!is_FAN_MAN_Mode)) {
					COM_Add_To_OutSTR_with_Sep("AUTO ON");
				}
				else COM_Add_To_OutSTR_with_Sep("AUTO OFF");
				UART_WriteString (&OutSTR[0]);
				goto UART_Execute_Command_END;
			}
			else goto UART_Execute_Command_ERROR;
		}		
	}
	// END: Fan Control Commands
	//##############################

	//##############################
	// Driver Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("DRIV:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// R - DRIV:VSET - read Driver DAC voltage Setpoint
		COM_Add_To_OutSTR_At_Position("VSET?", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			FloatToString(Value, PV_Volt_DAC_ACT);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;			
		}		
		//------------------------------
		// R/W - DRIV:IDGAIN - Driver Regulator Gain Resistor ID
		COM_Add_To_OutSTR_At_Position("IDGAIN", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > PI_CTR_PROP_ID_MAX || Tmp_I32 < 0){ // boundaries
						PI_CTR_Prop_ID_Man = PI_CTR_PROP_ID_MAX;
					}
					else PI_CTR_Prop_ID_Man = (uint8_t) Tmp_I32;
					itoa (PI_CTR_Prop_ID_Man, Value, 10);
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				itoa (PI_CTR_Prop_ID_Act, Value, 10);
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - DRIV:IDINT - Driver Regulator Integrator Capacitor ID
		COM_Add_To_OutSTR_At_Position("IDINT", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > PI_CTR_INT_ID_MAX || Tmp_I32 < 0){ // boundaries
						PI_CTR_Int_ID_Man = PI_CTR_INT_ID_MAX;
					}
					else PI_CTR_Int_ID_Man = (uint8_t) Tmp_I32;
					itoa (PI_CTR_Int_ID_Man, Value, 10);
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
				itoa (PI_CTR_Int_ID_Act, Value, 10);
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
	
	}
	// END: Driver Control Commands
	//##############################
					
	//##############################
	// ADC Control Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("ADC:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// R/W - ADC:AVR:VC - number average measurement voltage and current
		COM_Add_To_OutSTR_At_Position("AVR:VC", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > 255) ADC_Num_Avr_VoltCurr = 255;
					else if(Tmp_I32 <= 0) ADC_Num_Avr_VoltCurr = 1;
					else ADC_Num_Avr_VoltCurr = (uint8_t) Tmp_I32;
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
				// recalculate multiplier/divider
				ADC_Avr_Mult_VoltCurr = 1.0 / ((float) ADC_Num_Avr_VoltCurr * (float) ADC_Cycl_Avr_VoltCurr);
				// set clear buffer flags
				SET__R_Stat_Volt_Clear_Buffer;
				SET__R_Stat_Curr_Clear_Buffer;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (ADC_Num_Avr_VoltCurr, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - ADC:AVR:OTHER - number average measurement for other channels
		COM_Add_To_OutSTR_At_Position("AVR:OTHER", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 > 255) ADC_Num_Avr_Other = 255;
					else if(Tmp_I32 <= 0) ADC_Num_Avr_Other = 1;
					else ADC_Num_Avr_Other = (uint8_t) Tmp_I32;
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
				// recalculate multiplier/divider
				ADC_Avr_Mult_Other = 1.0 / ((float) ADC_Num_Avr_Other);
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (ADC_Num_Avr_Other, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R/W - ADC:CYCLES:VC - number averaged cycles for voltage and current
		COM_Add_To_OutSTR_At_Position("CYCLES:VC", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				if (isdigit(Value[0])) {
					Tmp_I32 = atol(Value);	//Read string to integer
					if(Tmp_I32 >= ADC_Cycl_Avr_Buff_MAX) ADC_Cycl_Avr_VoltCurr = ADC_Cycl_Avr_Buff_MAX - 1;
					else if(Tmp_I32 <= 0) ADC_Cycl_Avr_VoltCurr = 1;
					else ADC_Cycl_Avr_VoltCurr = (uint8_t) Tmp_I32;
				}
				else goto UART_Execute_Command_ERROR; // if no numerical value given error...
				// recalculate multiplier/divider
				ADC_Avr_Mult_VoltCurr = 1.0 / ((float) ADC_Num_Avr_VoltCurr * (float) ADC_Cycl_Avr_VoltCurr);
				// set clear buffer flags
				SET__R_Stat_Volt_Clear_Buffer;
				SET__R_Stat_Curr_Clear_Buffer;
			}
			else if (Address[OutSTR_L] == READ_IND_CHAR) {
				// Read Data
				COM_Add_QuestMark_To_OutSTR_No_Sep();
			}
			else goto UART_Execute_Command_ERROR;
			// Return data read-back
			itoa (ADC_Cycl_Avr_VoltCurr, Value, 10);
			COM_Add_To_OutSTR_with_Sep(Value);
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}		
	}
	// END: ADC Control Commands
	//##############################
						
	//##############################
	// EEPROM Commands
	//------------------------------
	COM_Copy_To_OutSTR_From_Start("EEROM:");
	Last_OutSTR_L = OutSTR_L;
	if (COM_Compare_to_OutStr(Address)) {
		//------------------------------
		// W - EEROM:WRITE - Write to EEPROM
		COM_Add_To_OutSTR_At_Position("WRITE", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L] == '\0') {
				// Write data
				EEPROM_ACCESS_UART(&Value[0], 1);
				COM_Add_To_OutSTR_with_Sep(Value);
			}
			else goto UART_Execute_Command_ERROR;
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}
		//------------------------------
		// R - EEROM:READ - Read from to EEPROM
		COM_Add_To_OutSTR_At_Position("READ?", Last_OutSTR_L);
		if (COM_Compare_to_OutStr_Offset(Address, Last_OutSTR_L)) {
			if (Address[OutSTR_L]  == '\0') {
				// Read data this time...
				EEPROM_ACCESS_UART(&Value[0], 0);
				COM_Add_To_OutSTR_with_Sep(Value);
			}
			else goto UART_Execute_Command_ERROR;
			UART_WriteString (&OutSTR[0]);
			goto UART_Execute_Command_END;
		}		
	}

	COM_Copy_To_OutSTR_From_Start("TEMP?");
	if (COM_Compare_to_OutStr(Address)) {
		FloatToString(Value, AI_NTC_Temp_1);
		COM_Add_To_OutSTR_with_Sep(Value);
		FloatToString(Value, AI_NTC_Temp_2);
		COM_Add_To_OutSTR_with_Sep(Value);
		if(is_SysConfig_TEMP_On) {		
			FloatToString(Value, AI_RTD_Temp);
			COM_Add_To_OutSTR_with_Sep(Value);
		}
	
		#ifdef PCBconfig_TEMP_Heat_Monitoring_enabled
			for (uint8_t i =0; i<8; i++){
				FloatToString(Value, AI_HEAT_NTC_Temp[i]);
				COM_Add_To_OutSTR_with_Sep(Value);			
			}

		#endif

		UART_WriteString (&OutSTR[0]);
		CLR__Status_MainTimerOverRun; // clear timer overrun flag
		goto UART_Execute_Command_END;
	}





	// END: EEPROM Commands
	//##############################
	
	// -----------------------------	
	// Com error point
	UART_Execute_Command_ERROR:;
	UART_WriteString ("?"); // Return command not found or error
	// Com end point
	UART_Execute_Command_END:;
			
}


//-------------------------------------------------------------------------------------------
// Transfer IV data
void UART_Transfer_IV() {
	/* This Function coordinates the IV curve data transfer.
	- IV data is send in "small chunks at a time" to keep the data transfer buffer size small
	- The OutSTR buffer is filled while the UART transfer buffer is emptied through the RS485
	- when the UART buffer is empty, the OutSTR buffer is copied to the UART buffer and emptied
	- once all is transfered the function flags are reset and a termination character is send
	- function is called in the eternal main loop independent off measurement or control timers
	*/

	char Value[UART_Value_LENGH+1];
	
	// check if IV buffer is full
	if (is_COM_Status_IV_Buffer_Ready_On) {
		// new PV IV data buffer is ready for transfer
		if (UART_Reply_L == 0) {
			// UART transfer buffer is empty
			// check if there is IV data in the OutString buffer
			if (OutSTR[0] == '\0') {
				// if no data - indicated end of transfer, terminate IV transfer
				CLR__COM_Status_IV_Data_Transfer;	// stop IV data transfer
				Trans_IV_Point = 0;
			}
			UART_WriteString (&OutSTR[0]);	//Output
			CLR__COM_Status_IV_Buffer_Ready;
		}
	}
	
	// check if IV buffer was emptied just before
	if (!is_COM_Status_IV_Buffer_Ready_On) {
		// buffer empty make new IV data buffer
		if (Trans_IV_Point < IV_Meas_Points) {
			// add some more IV data to buffer
			FloatToString(Value, PV_IV_Voltage[Trans_IV_Point]);
			OutSTR_L = 0;
			COM_Add_To_OutSTR_with_Sep(Value);
			FloatToString(Value, PV_IV_Current[Trans_IV_Point]);
			COM_Add_To_OutSTR_with_Sep(Value);
			Trans_IV_Point++;
		}
		else {
			// Finish transfer with termination character
			Trans_IV_Point = 0;
			OutSTR[0] = '\0';
		}
		SET__COM_Status_IV_Buffer_Ready;	// indicate buffer written
	}
	
}

//===========================================================================================
// Data Transfer Functions
//===========================================================================================
//-------------------------------------------------------------------------------------------
// UART Write Character
void UART_WriteChar(char c) {
	/* Function to send a character to USART serial port
	- mostly used for debugging - use with caution
	*/
	while (!(UCSR0A & (1<<UDRE0))); // waits until ready
	_delay_us(2);
	SETBIT(RS485_port, RS485_pin);		// RS485 com enable TxD & disable RxD
	_delay_us(2);
	UDR0 = c; // send
	_delay_ms(2);
	CLRBIT(RS485_port, RS485_pin);		// RS485 com disable TxD & enable RxD
}

//-------------------------------------------------------------------------------------------
// UART Write String
void UART_WriteString (volatile char *STR) {
	/* this function transfers the OutSTR buffer tot the UART reply buffer and initiates the data transfer
	- function waits until the transfer line is ready, sets the RS485 control outputs and enabled the transmitting
	- the first character is moved onto the UART register and the rest are controlled by the UART send data interrupt
	*/
	
	uint8_t i = 0;

	while (UART_Reply_L != 0)	{//Wait until buffer is empty
	}
	while (STR[i] != '\0')	{ //transfer String to buffer
		UART_Reply[i] = STR[i];
		i++;
	}
	// set com and string termination
	// if not IV transfer, then add termination
	if (!is_COM_Status_IV_Data_Transfer_On) {
		UART_Reply[i] = UART_TERM_CHAR;
		i++;
	}
	UART_Reply[i] = '\0';
	UART_Reply_L = i;
	UART_Reply_P = 1;
	// Start transmitting
	while (!(UCSR0A & (1<<UDRE0))); // waits until ready (in case of debug UART_WriteChar calls)
	//_delay_us(10);
	SETBIT(RS485_port, RS485_pin);		// RS485 com enable TxD & disable RxD
	_delay_us(1);				// wait RS485 direction change complete
	UCSR0B |= (1<<TXCIE0);		//Enable transmitting interrupt
	UDR0 = UART_Reply[0]; // send first character, the rest are send with the UART interrupt

}


//===========================================================================================
// Command and output string handling functions
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Compare String to OutSTR
bool COM_Compare_to_OutStr(char *ComandSTR) {
	/* this com handling function compares a given string the OutSTR from start to the position the OutSTR ends
	- it is used to compare the first section of the received command to the Code Section Command stored in OutSTR
	- returns true if both match */
	uint8_t i;
	bool match = true;
	i = OutSTR_L; // set counter to the writing to the OutSTR write pointer
	while (i) {
		i--;
		if (ComandSTR[i] != OutSTR[i]){
			match = false;
			break;
		}
	}
	return match;
}

//-------------------------------------------------------------------------------------------
// Compare String from offset onwards
bool COM_Compare_to_OutStr_Offset(char *ComandSTR, uint8_t Offset) {
	/* this com handling function compares a given string the OutSTR from offset to the position the OutSTR ends
	- it is used to compare the first section of the received command to the Code Section Command stored in OutSTR
	- returns true if both match */
	uint8_t i;
	bool match = true;
	i = OutSTR_L; // set counter to the writing to the OutSTR write pointer
	while (i > Offset) {
		i--;
		if (ComandSTR[i] != OutSTR[i]){
			match = false;
			break;
		}
	}
	return match;
}

//-------------------------------------------------------------------------------------------
// Copy string to OutSTR, overwrite last entry
void COM_Copy_To_OutSTR_From_Start(char *CopySTR){
	/* this optimized com handling function overwrites the OutSTR variable with a new string
	- string position counter is reset, data copied and ends at length value of CopySTR */
	// Reset OutSTR position indicator
	OutSTR_L = 0;
	// copy string
	while (CopySTR[OutSTR_L] != '\0')	{ //transfer String to buffer
		OutSTR[OutSTR_L] = CopySTR[OutSTR_L];
		OutSTR_L ++;
	}
	// add string end indicator
	OutSTR[OutSTR_L] = '\0';
}

//-------------------------------------------------------------------------------------------
// Add String at end and of OutSTR without Separation
void COM_Add_To_OutSTR_No_Sep(char *CopySTR){
	/* this com handling function add CopySTR to the end of OutSTR without separation */
	// copy string to the end
	uint8_t i = 0;
	while (CopySTR[i] != '\0')	{ //transfer String to buffer
		OutSTR[OutSTR_L] = CopySTR[i];
		OutSTR_L ++;
		i++;
	}
	// add string end indicator
	OutSTR[OutSTR_L] = '\0';
}

//-------------------------------------------------------------------------------------------
// Add to end of OutSTR with separation char in the middle - speed optimized function for UART com
void COM_Add_To_OutSTR_with_Sep(volatile char *AddSTR) {
	uint8_t i = 0;
	// add separation character
	OutSTR[OutSTR_L] = UART_SEP_CHAR;
	OutSTR_L++;
	// add new string thereafter
	while (AddSTR[i] != '\0') {
		OutSTR[OutSTR_L] = AddSTR[i];
		i++;
		OutSTR_L ++;
	}
	// add string end indicator
	OutSTR[OutSTR_L] = '\0';
}

//-------------------------------------------------------------------------------------------
// Add String at end and of OutSTR without Separation
void COM_Add_To_OutSTR_At_Position(char *CopySTR, uint8_t Position){
	/* this com handling function adds CopySTR to the end of OutSTR at a specific position 
	- no separation & string termination at the end */
	// copy string to specific position
	OutSTR_L = Position;
	uint8_t i = 0;
	while (CopySTR[i] != '\0')	{ //transfer String to buffer
		OutSTR[OutSTR_L] = CopySTR[i];
		OutSTR_L ++;
		i++;
	}
	// add string end indicator
	OutSTR[OutSTR_L] = '\0';
}

//-------------------------------------------------------------------------------------------
// Add ? to end and of OutSTR without Separation
void COM_Add_QuestMark_To_OutSTR_No_Sep(){
	/* this com handling function adds ? to the end of OutSTR without separation */
	//add ? to the string
	OutSTR[OutSTR_L] = UART_COM_READ_INDICATOR;
	OutSTR_L ++;
	
	// add string end indicator
	OutSTR[OutSTR_L] = '\0';
}

//===========================================================================================
// Standard string handling functions
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Compare two Strings
uint8_t Compare_String(char *STR1, volatile char *STR2, uint8_t Length) {
	uint8_t i;
	i = 0;
	while (i != Length) {
		if (*STR1 != *STR2){
			break;
		}
		STR1++;
		STR2++;
		i++;
	}
	if (i == Length) return 1;
	else return 0;
}

//-------------------------------------------------------------------------------------------
// Copy String
void Copy_String(volatile char *CopSTR, char *OrigSTR) {
	uint8_t i;
	i = 0;
	
	while (OrigSTR[i] != '\0')	{ //transfer String to buffer
		CopSTR[i] = OrigSTR[i];
		i++;
	}
	CopSTR[i] = '\0';

}

//-------------------------------------------------------------------------------------------
// Concentrate String
void Concentrate_String(char *STR1, char *STR2, volatile char *ConSTR) {
	uint8_t i = 0;
	uint8_t l = 0;
		
	while (STR1[l] != '\0') {
		ConSTR[l] = STR1[l];
		l++;
	}
	while (STR2[i] != '\0') {
		ConSTR[l] = STR2[i];
		i++;
		l++;
	}
	ConSTR[l] = '\0';

}

//-------------------------------------------------------------------------------------------
// Add String
void AddEnd_String(volatile char *ConSTR, volatile char *AddSTR) {
	uint8_t i = 0;
	uint8_t l = 0;
		
	while (ConSTR[l] != '\0') {
		l++;
	}
	while (AddSTR[i] != '\0') {
		ConSTR[l] = AddSTR[i];
		i++;
		l++;
	}
	ConSTR[l] = '\0';

}

//===========================================================================================
// Support conversion functions
//===========================================================================================
//-------------------------------------------------------------------------------------------
// Convert float to string 
void FloatToString(char *STR, float Value) {

	dtostre(Value, STR, 5, 3);
	
}

//-------------------------------------------------------------------------------------------
// Convert String to Float
float StringToFloat(char *STR) {
	
// 	float Value;
// 	Value = atof(STR);
// 	return Value;
	return atof(STR);

}