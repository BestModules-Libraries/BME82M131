/*****************************************************************
File:             BME82M131.h
Author:           BEST MODULES CORP.
Description:      Define classes and required variables
Version:          V1.0.2 --2024-07-15
******************************************************************/
#ifndef _H
#define _H
 
#include "Arduino.h"
#include <Wire.h>

#define MODULE_MID      (0x48)
/** GAIN selection */
typedef enum
{
  GAIN_1,         // ALS gain 1x
  GAIN_2,				  // ALS gain 2x
  GAIN_1_8,       // ALS gain 1/8x
  GAIN_1_4,       // ALS gain 1/4x
  GAIN_INVALID
} GAIN_t;
/** ALS integration time setting */
typedef enum
{
	IT_25ms,        // ALS intetgration time 25ms
	IT_50ms,        // ALS intetgration time 50ms
	IT_100ms,				// ALS intetgration time 100ms
	IT_200ms,				// ALS intetgration time 200ms
	IT_400ms,				// ALS intetgration time 400ms
	IT_800ms,				// ALS intetgration time 800ms
	IT_INVALID
} IT_TIME_t;
		
typedef enum
{
	CONF_IT_25ms = 0b1100,      // ALS intetgration time 25ms
	CONF_IT_50ms = 0b1000,      // ALS intetgration time 50ms
	CONF_IT_100ms = 0b0000,		  // ALS intetgration time 100ms
  CONF_IT_200ms = 0b0001,			// ALS intetgration time 200ms
	CONF_IT_400ms = 0b0010,			// ALS intetgration time 400ms
  CONF_IT_800ms = 0b0011,			// ALS intetgration time 800ms
  CONF_IT_INVALID
} CONF_IT_TIME_t;	
/** ALS persistence protect number setting */
typedef enum
{
  PERS_1,							// ALS irq persisance 1 sample
  PERS_2,             // ALS irq persisance 2 sample
  PERS_4,							// ALS irq persisance 4 sample
  PERS_8,							// ALS irq persisance 8 sample
  PERS_INVALID
} PERS_t;

/** ALS interrupt enable setting */
typedef enum
{
  INT_DISABLE,
  INT_ENABLE,
  INT_INVALID
} INTEN_t;
/** ALS shut down setting */
typedef enum
{
  POWER_ON,
  SHUT_DOWN,
  SD_INVALID
} SD_t;
typedef enum
{
	POWER_SAVING_MODE1,              //Power saving mode 1
	POWER_SAVING_MODE2, 						 //Power saving mode 2
	POWER_SAVING_MODE3,						   //Power saving mode 3
	POWER_SAVING_MODE4, 						 //Power saving mode 4
	POWER_SAVING_INVALID
} PSM_MODE_t;
typedef enum
{
	POWER_SAVING_MODE_DISABLE,                             
	POWER_SAVING_MODE_ENABLE,
	PSMEN_INVALID
} PSMEN_t;
/** ALS interrupt status, logical OR of the crossing low and high thrteshold INT triggers */
typedef enum
{
  INT_STATUS_NONE,
  INT_STATUS_HIGH,
  INT_STATUS_LOW,
  INT_STATUS_BOTH,
  INT_STATUS_INVALID
} INT_STATUS_t;

class BME82M131
{
	public:
	  BME82M131(uint8_t intPin = 2, TwoWire *theWire = &Wire);
    void begin(uint8_t i2c_addr=0x48);	
    uint8_t getINT();
    uint8_t getNumber();
    uint16_t readALS(uint8_t sensornumber);
    uint16_t readWhite(uint8_t sensornumber);
    float readLux(uint8_t sensorNmuber);
    uint8_t getInterruptFalg(uint8_t sensorNmuber);

    uint8_t getPowerSavingMode(uint8_t sensorNmuber); 
    uint8_t getPowerSavingModeStatus(uint8_t sensorNmuber); 
    uint8_t getMoudleStatus(uint8_t sensorNmuber);      
    uint8_t isInterruptEnabled(uint8_t sensorNmuber); 
    uint8_t getPersistence(uint8_t sensorNmuber);
    uint8_t getIntegrationTime(uint8_t sensorNmuber);
    uint8_t getGain(uint8_t sensorNmuber); 
    uint16_t getHighThreshold(uint8_t sensorNmuber);
    uint16_t getLowThreshold(uint8_t sensorNmuber);
    
    bool setPowerSavingMode(uint8_t sensorNmuber, PSM_MODE_t mode);  
    bool setPowerSavingModeStatus(uint8_t sensorNmuber, PSMEN_t psmen);   
    bool  ALSOn( uint8_t sensorNmuber);
    bool  ALSDown( uint8_t sensorNmuber); 
    bool setInterruptStatus(uint8_t sensorNmuber, INTEN_t ie);   
    bool setPersistence(uint8_t sensorNmuber, PERS_t pers);  
    bool setIntegrationTime(uint8_t sensorNmuber, IT_TIME_t it);
    bool setGain(uint8_t sensorNmuber, GAIN_t gain); 
    bool setHighThreshold(uint8_t sensorNmuber, uint16_t threshold);
    bool setLowThreshold(uint8_t sensorNmuber, uint16_t threshold);                                  
    
	private:
		/** Provide bit field access to the configuration register */
    uint8_t _tx_buf[10] = {0};
    uint8_t _rx_buf[10] = {0};
    uint8_t _SumOfModules = 1;
    TwoWire *_i2cPort = NULL;
    uint8_t _i2cAddr ;
    uint8_t _intPin;
    bool initialize();
    bool isConnected();
    bool setStatus(uint8_t sensornumber, SD_t sd);     
    void sendData(uint8_t id, uint8_t len, uint8_t par[]); 
    void writeBytes(uint8_t wbuf[], uint8_t wlen);       
    bool readBytes(uint8_t rbuf[], uint8_t rlen);
    bool getAlsConfig(uint8_t sensornumber,uint16_t &als_conf);
    bool getPsmReg(uint8_t SensorNmuber, uint16_t &psm);
    IT_TIME_t ITTimeFromCONf(CONF_IT_TIME_t it);
    CONF_IT_TIME_t configITTime(IT_TIME_t it);
		typedef struct
		{
			union
			{
				uint16_t all;
				struct
				{	
					uint16_t ALS_SD : 1; 	 // ALS shut down setting
					uint16_t ALS_INT_EN : 1; // ALS interrupt enable setting
					uint16_t ALS_RES1 : 2; 	 // Reserved
					uint16_t ALS_PERS : 2; 	 // ALS persistence protect number setting
					uint16_t ALS_IT : 4; 	 // ALS integration time setting
					uint16_t ALS_RES2 : 1; 	 // Reserved
					uint16_t ALS_GAIN : 2; 	 // ALS GAIN selection
					uint16_t ALS_RES3 : 3; 	 // Reserved
				};
			};
		} CONF_REG_t;
		
		CONF_REG_t _ConfReg;
		
        /** Provide bit field access to the configuration register */
		typedef struct
		{
			union
			{
				uint16_t all;
				struct
				{	
					uint16_t PSM_EN: 1; 	 // ALS shut down setting
					uint16_t PSM_MODE : 2; // ALS interrupt enable setting
					uint16_t PSM_RES : 13;
				};
			};
		} PSM_REG_t;
		
		PSM_REG_t _PsmReg;
		
		/** Provide bit field access to the interrupt status register
			Note: reading the interrupt status register clears the interrupts.
            So, we need to check both interrupt flags in a single read. */
		typedef struct
		{
			union
			{
				uint16_t all;
				struct
				{
					uint16_t INT_STATUS_RES : 14; // Reserved
					// Bit 14 indicates if the high threshold was exceeded
					// Bit 15 indicates if the low threshold was exceeded
					uint16_t INT_STATUS_FLAGS : 2;
				};
			};
		} INT_STATUS_REG_t;	

		/** The sensor resolution vs. gain and integration time. Taken from the BME82M131 Application Note. */
		const float LUX_RESOLUTION[4][6] =
		{
			// 25ms    50ms    100ms   200ms   400ms   800ms
			{0.2304, 0.1152, 0.0576, 0.0288, 0.0144, 0.0072}, // Gain 1
			{0.1152, 0.0576, 0.0288, 0.0144, 0.0072, 0.0036}, // Gain 2
			{1.8432, 0.9216, 0.4608, 0.2304, 0.1152, 0.0576}, // Gain 1/8
			{0.9216, 0.4608, 0.2304, 0.1152, 0.0576, 0.0288}  // Gain 1/4
		};
		/** The BME82M131 gain (sensitivity) settings as text (string) */
		const char *GAIN_SETTINGS[5] =
        {
         // Note: these are in the order defined by ALS_SM and sensitivity_mode_t
         "x1","x2","x1/8","x1/4","INVALID"
        };

         /** The BME82M131 integration time settings as text (strting) */
        const char *INTEGRATION_TIMES[7] =
        {
			"25ms","50ms","100ms","200ms","400ms","800ms","INVALID"
        };
		/** The BME82M131 persistence protect settings as text (string) */
        const char *PERSISTENCE_PROTECT_SETTINGS[5] =
        {
			"1", "2", "4", "8", "INVALID"
        };
		/** The BME82M131 power saving mode seting as text (string) */
        const char *POWER_SAVING_MODE_SETTINGS[5] =
        {
			"MODE1", "MODE2", "MODE3", "MODE4", "INVALID"
        };	
        //CMD
		
		const uint8_t _CMD_CHECK_MODULE = 0x01;
		const uint8_t _CMD_W_ALS_CONFIG = 0x02;
		const uint8_t _CMD_W_HIGH_THRESHOLD = 0x03;
		const uint8_t _CMD_W_LOW_THRESHOLD = 0x04;
		const uint8_t _CMD_W_POWER_SAVING_MODE = 0x05;
		const uint8_t _CMD_R_ALS_CONFIG = 0x06;
		const uint8_t _CMD_R_HIGH_THRESHOLD = 0x07;
		const uint8_t _CMD_R_LOW_THRESHOLD = 0x08;
		const uint8_t _CMD_R_POWER_SAVING_MODE = 0x09;
		const uint8_t _CMD_R_ALS_OUTPUT = 0x0A;
		const uint8_t _CMD_R_WHITE_OUTPUT = 0x0B;
		const uint8_t _CMD_R_INT_STATUS = 0x0C;
		
};
#endif
