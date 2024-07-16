/*****************************************************************
File:        BME82M131.cpp
Author:      BEST MODULES CORP.
Description: I2C communication with the BME82M131 and obtain the corresponding value  
Version:     V1.0.2 --2024-07-15
******************************************************************/
#include "BME82M131.h"
/*********************************************************************************
Description: Constructor
Parameters:     
            intPin：INT Output pin connection with Arduino, the INT will be pulled down when an object approaches
            *wirePort: Wire object if your board has more than one I2C interface     
Return:          
Others:     
***********************************************************************************/
BME82M131::BME82M131(uint8_t intPin, TwoWire *theWire)
{
	_intPin = intPin;
	_i2cPort = theWire;
}
/*********************************************************
Description: Constructor
Parameters:   i2c_addr：IIC  addr          
Return:     
Others:       
**********************************************************/
void BME82M131::begin(uint8_t i2c_addr)
{	
	delay(500);
 	_i2cAddr =i2c_addr ;
	_i2cPort->begin();
  pinMode(_intPin, INPUT_PULLUP);
	getNumber();
	initialize();
}

/**********************************************************
Description: get INT PIN Status
Parameters:  
Return:      INT PIN Status(1 bit)   
Others:      1:INT status is HIGH(default)
             0:INT status is LOW(get a interrupt)
**********************************************************/
uint8_t BME82M131::getINT()
{
      uint8_t statusValue = 0;
      statusValue = digitalRead(_intPin);
      return statusValue;     
}
/********************************************************************
Description: Gets the number of modules
Parameters:            
Return:   _rx_buf[4]: Number of connecting modules 
Others:         
*********************************************************************/
uint8_t BME82M131::getNumber()
{
	uint8_t status;
	_tx_buf[3] = _CMD_CHECK_MODULE;	
	delay(15);
  sendData(1, 5, _tx_buf);
	delay(15);
  if(readBytes(_rx_buf,6))
	{
  status=_rx_buf[3];
    if(status==0)
    {
      _SumOfModules = _rx_buf[4];
      return _rx_buf[4];
    }
    else
    {
      return 0;
    }
	}
 else
 {
	return 0;
 }
}
/***************************************************************************************
Description: Get the VEML7700's ambient light sensor data (ALS)
Parameters:       
            sensornumber : Module Number
Return:
             als:ambient light data
Others:         
****************************************************************************************/
uint16_t BME82M131::readALS(uint8_t sensornumber)
{
  uint8_t status; 
  _tx_buf[3] = _CMD_R_ALS_OUTPUT;
  delay(15);
  sendData(sensornumber, 5, _tx_buf);
  delay(15);
   if(readBytes(_rx_buf,7))
   {
    status = _rx_buf[3];
    if(status == 0)
    {
      uint16_t als=0;
      als = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
      return als;
    }
    else
    {
      return 0;
    }    
   }
   else
   {
    return 0;
   }  
}
/***************************************************************************************
Description: Get the VEML7700's white level data (WHITE)
Parameters:       
            sensornumber : Module Number
Return:     
             als:white level data
Others:         
****************************************************************************************/
uint16_t BME82M131::readWhite(uint8_t sensornumber)
{  
  uint8_t status;
  uint16_t white=0;
  _tx_buf[3] = _CMD_R_WHITE_OUTPUT;

  delay(50);
  sendData(sensornumber, 5, _tx_buf);
  delay(50);
  if(readBytes(_rx_buf,7))
  {
    status = _rx_buf[3];
    if(status == 0)
    {
      white = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
     return white;
    }
    else
    {
      return 0;
    } 
  }
  return 0;
}
/***************************************************************************************
Description: Read the sensor data and calculate the lux
Parameters:      
            sensorNmuber : Module Number
Return:     
            lux:Floating point Lux data
Others:         
****************************************************************************************/
float BME82M131::readLux(uint8_t sensorNmuber)
{
  GAIN_t gain=GAIN_2;
  IT_TIME_t it=IT_800ms;
  uint16_t als=0;
  float lux=0;
  float resolution; 
  uint8_t gain_flag=getGain(sensorNmuber);
  if(gain_flag==0)
  {
    gain=GAIN_1;
  }
  else if(gain_flag==1)
  {
     gain=GAIN_2;
  }
 else if(gain_flag==2)
  {
     gain=GAIN_1_8;
  }
 else if(gain_flag==3)
  {
     gain=GAIN_1_4;
  }
  else if(gain_flag==4)
  {
    return 0;
  }
  uint8_t integration_flag=getIntegrationTime(sensorNmuber);  
  if(integration_flag==0)
  {
   it= IT_25ms;
  }
  else if(integration_flag==1)
  {
    it= IT_50ms;
  }
  else if(integration_flag==2)
  {
    it= IT_100ms;
  }
  else if(integration_flag==3)
  {
    it= IT_200ms;
  }
  else if(integration_flag==4)
  {
    it= IT_400ms;
  }
  else if(integration_flag==5)
  {
    it= IT_800ms;
  }
  else if(integration_flag==6)
  {
    return 0;
  }
  als = readALS(sensorNmuber);
  resolution = LUX_RESOLUTION[gain][it];
  lux = (float)als * resolution;
  return lux;
}
/***************************************************************************************
Description: get the VEML7700's interrupt status register
Parameters:  sensorNmuber : Module Number
Return:      0 : INT_STATUS_NONE,
             1 : INT_STATUS_HIGH,
             2 : INT_STATUS_LOW,
             3 : INT_STATUS_BOTH
Others:         
****************************************************************************************/
uint8_t BME82M131::getInterruptFalg(uint8_t sensorNmuber)
{
  uint8_t status;
  INT_STATUS_t  it_status;
  INT_STATUS_REG_t REG_Temp;
  
  _tx_buf[3] = _CMD_R_INT_STATUS;

  delay(50);
  sendData(sensorNmuber, 5, _tx_buf);
  delay(50);
  if(readBytes( _rx_buf,7))
  {
    status = _rx_buf[3];
    if(status == 0)
    {
      REG_Temp.all = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
      it_status = (INT_STATUS_t)REG_Temp.INT_STATUS_FLAGS;
      if(it_status==INT_STATUS_NONE)  
      {
        return 0; 
      }
      else if(it_status==INT_STATUS_HIGH)  
      {
        return 1; 
      }
      else if(it_status==INT_STATUS_LOW)  
      {
        return 2; 
      }
      else if(it_status==INT_STATUS_BOTH)  
      {
        return 3; 
      }
    }
    else
    {
     return 0;
    }
  }
  else
  {
     return 0;
  } 
  return 0;
}
/***************************************************************************************
Description: Get the VEML7700's Power saving mode.
Parameters:       
            sensorNmuber : Module Number              
Return:     0-POWER_SAVING_MODE1              
            1-POWER_SAVING_MODE2              
            2-POWER_SAVING_MODE3              
            3-POWER_SAVING_MODE4
Others:         
***********************************************************************************/
uint8_t BME82M131::getPowerSavingMode(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t psm;
  PSM_MODE_t  mode;
  status = getPsmReg(sensorNmuber, psm);
  if(status)
  {
    _PsmReg.all = psm;
    mode = (PSM_MODE_t)_PsmReg.PSM_MODE;
    if(mode == POWER_SAVING_MODE1)
    {
      return 0;
    }
    else if(mode == POWER_SAVING_MODE2)
    {
      return 1;
    }
    else if(mode == POWER_SAVING_MODE3)
    {
      return 2;
    }
    else if(mode == POWER_SAVING_MODE4)
    {
      return 3;
    }
  }
  else
  {
    return 0;
  }
  return 0;
}
/***************************************************************************************
Description: Get the VEML7700's power saving mode enable state.
Parameters:       
            sensorNmuber : Module Number    
Return:     0-POWER_SAVING_MODE_DISABLE                                   
            1-POWER_SAVING_MODE_ENABLE
Others:         
***********************************************************************************/
uint8_t BME82M131::getPowerSavingModeStatus(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t psm;
  PSMEN_t psmen;
  status = getPsmReg(sensorNmuber, psm);
  if(status)
  {
    _PsmReg.all = psm;
    psmen = (PSMEN_t)_PsmReg.PSM_EN;
    if(psmen==POWER_SAVING_MODE_DISABLE)
    {
      return 0;
    }
    else if(psmen==POWER_SAVING_MODE_ENABLE)
    {
      return 1;
    }
  }
  else
  {
    return 0;
  }
  return 0;
}

/***************************************************************************************
Description: Get the VEML7700's moudle status (ALS_SD)
Parameters:       
            sensorNmuber : Module Number       
Return:     0-POWER_ON
            1-SHUT_DOWN
Others:         
****************************************************************************************/
uint8_t BME82M131::getMoudleStatus(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t als_conf; 
  SD_t  sd;
  status = getAlsConfig(sensorNmuber, als_conf); 
  
  if(status)
  {
    _ConfReg.all = als_conf;
    sd = (SD_t)_ConfReg.ALS_SD;
   if(sd==POWER_ON)
   {
    return 0;
   }
   else if(sd==SHUT_DOWN)
   {
    return 1;
   }
  }
  else
  {
    return 0;
  } 
  return 0;
}

/***************************************************************************************
Description: Get the VEML7700's interrupt enable setting (ALS_INT_EN)
Parameters:      
            sensorNmuber : Module Number
Return:  
          0-INT_DISABLE
          1-INT_ENABLE 
Others:         
****************************************************************************************/
uint8_t BME82M131::isInterruptEnabled(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t als_conf;
  INTEN_t ie;
  status = getAlsConfig(sensorNmuber, als_conf); 
  if(status)
  {
    _ConfReg.all = als_conf;
    ie = (INTEN_t)_ConfReg.ALS_INT_EN;
    if(ie == INT_DISABLE)
    {
      return 0;
    }
    else if(ie == INT_ENABLE)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
     return 0;
  } 
}

/***************************************************************************************
Description: Get the VEML7700's persistence protect number setting (ALS_PERS)
Parameters:       
            sensorNmuber : Module Number                         
Return:     0-PERS_1  
            1-PERS_2                    
            2-PERS_4
            3-PERS_8
Others:         
****************************************************************************************/
uint8_t BME82M131::getPersistence(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t als_conf;
  PERS_t  pers;
  status = getAlsConfig(sensorNmuber, als_conf); 
  if(status)
  {
    _ConfReg.all = als_conf;
    pers = (PERS_t)_ConfReg.ALS_PERS;
   if(pers==PERS_1)
   {
    return 0;
   }
   else if(pers==PERS_2)
   {
    return 1;
   }
   else if(pers==PERS_4)
   {
    return 2;
   }
   else if(pers==PERS_8)
   {
    return 3;
   }
  }
  else
  {
    return 0;
  }
  return 0;
}

/***************************************************************************************
Description: Get the VEML7700's integration time setting (ALS_IT)
Parameters:      
          sensorNmuber : Module Number     
Return:       0-IT_25ms        
              1-IT_50ms           
              2-IT_100ms
              3-IT_200ms
              4-IT_400ms
              5-IT_800ms
Others:         
****************************************************************************************/
uint8_t BME82M131::getIntegrationTime(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t als_conf;
  IT_TIME_t it;
  status = getAlsConfig(sensorNmuber, als_conf);  
  if(status)
  {
    _ConfReg.all = als_conf;
    it = ITTimeFromCONf((CONF_IT_TIME_t)_ConfReg.ALS_IT);
   if(it==IT_25ms)
   {
    return 0;
   }
   else if(it==IT_50ms)
   {
    return 1;
   }
   else if(it==IT_100ms)
   {
    return 2;
   }
   else if(it==IT_200ms)
   {
    return 3;
   }
   else if(it==IT_400ms)
   {
    return 4;
   }
   else if(it==IT_800ms)
   {
    return 5;
   }
  }
  else
  {
    return 0;
  }
  return 0;
}
/***************************************************************************************
Description: Get the VEML7700's gain.
Parameters:      
            sensorNmuber : Module Number                  
Return:     0-GAIN_1          
            1-GAIN_2
            2-GAIN_1_8           
            3-GAIN_1_4
Others:         
****************************************************************************************/
uint8_t BME82M131::getGain(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t als_conf;
  GAIN_t  gain;
  status = getAlsConfig(sensorNmuber, als_conf);  
  if(status)
  {
    _ConfReg.all = als_conf;
     gain = (GAIN_t)_ConfReg.ALS_GAIN;
   if(gain==GAIN_1)
   {
    return 0;
   }
   else if(gain==GAIN_2)
   {
    return 1;
   }
   else if(gain==GAIN_1_8)
   {
    return 2;
   }
   else if(gain==GAIN_1_4)
   {
    return 3;
   }
  }
  else
  {
    return 0;
  }
  return 0;
}
/***************************************************************************************
Description: Get the VEML7700's ALS high threshold window setting.
Parameters:      
            sensorNmuber : Module Number
Return:     threshold: high threshold window
Others:         
****************************************************************************************/
uint16_t BME82M131::getHighThreshold(uint8_t sensorNmuber)
{
  uint16_t threshold=0; 
  _tx_buf[3] = _CMD_R_HIGH_THRESHOLD;
  delay(15);
  sendData(sensorNmuber, 5, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,7))
  { 
    if( _rx_buf[3] == 0)
    {
      threshold = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
      return threshold;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

/***************************************************************************************
Description: Get the VEML7700's ALS low threshold .
Parameters:       
            sensorNmuber : Module Number     
Return:     threshold: low threshold 
Others:         
****************************************************************************************/
uint16_t BME82M131::getLowThreshold(uint8_t sensorNmuber)
{
  uint8_t status;
  uint16_t threshold=0;
  _tx_buf[3] = _CMD_R_LOW_THRESHOLD;

  delay(15);
  sendData(sensorNmuber, 5, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,7))
  {
    status = _rx_buf[3];  
    if(status == 0)
    {
      threshold = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
      return threshold;
    }
    else
    {
       return 0;
    }
  }
  else
  {
    return 0;
  }
}

/***************************************************************************************
Description: set the VEML7700's Power saving mode.
Parameters:       
            sensorNmuber : Module Number
            mode:
              POWER_SAVING_MODE1              
              POWER_SAVING_MODE2              
              POWER_SAVING_MODE3              
              POWER_SAVING_MODE4                 
Return:     ture or false
Others:         
***********************************************************************************/
bool BME82M131::setPowerSavingMode(uint8_t sensorNmuber, PSM_MODE_t mode)
{
  uint8_t status;
  uint16_t psm;
  
  getPsmReg(sensorNmuber, psm);
  _PsmReg.all = psm;
  _PsmReg.PSM_MODE = (uint16_t)mode;
  
  _tx_buf[3] = _CMD_W_POWER_SAVING_MODE;
  _tx_buf[4] = (uint8_t)(_PsmReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_PsmReg.all & 0xff00) >> 8);
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      return true;
    }
    else 
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
/***************************************************************************************
Description: set the VEML7700's power saving mode status.
Parameters:      
            sensorNmuber : Module Number
            psmen : 
                 POWER_SAVING_MODE_DISABLE                            
                 POWER_SAVING_MODE_ENABLE
Return:      true or false
Others:         
***********************************************************************************/
bool BME82M131::setPowerSavingModeStatus(uint8_t sensorNmuber, PSMEN_t psmen)
{
  uint8_t status;
  uint16_t psm;
  
  getPsmReg(sensorNmuber, psm);
  _PsmReg.all = psm;
  _PsmReg.PSM_EN = (uint16_t)psmen;
  
  _tx_buf[3] = _CMD_W_POWER_SAVING_MODE;
  _tx_buf[4] = (uint8_t)(_PsmReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_PsmReg.all & 0xff00) >> 8);
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  } 
}

/***************************************************************************************
Description: Set the VEML7700's power on setting (ALS_SD)
Parameters:   sensorNmuber : Module Number          
Return:      true or false    
Others:         
****************************************************************************************/
bool BME82M131::ALSOn(uint8_t sensorNmuber)
{
  if(setStatus(sensorNmuber,POWER_ON))
  {
    return true;
  }
  else
  {
    return false;
  }
}
/***************************************************************************************
Description: Set the VEML7700's power down setting (ALS_SD)
Parameters:   sensorNmuber : Module Number          
Return:      true or false    
Others:         
****************************************************************************************/
bool BME82M131::ALSDown(uint8_t sensorNmuber)
{
  if(setStatus(sensorNmuber,SHUT_DOWN))
  {
    return true;
  }
  else
  {
    return false;
  }
}
/***************************************************************************************
Description: Set the VEML7700's interrupt(ALS_INT_EN)
Parameters:      
            sensorNmuber : Module Number
            ie :
               INT_DISABLE
               INT_ENABLE     
Return:      true or false
Others:         
****************************************************************************************/
bool BME82M131::setInterruptStatus(uint8_t sensorNmuber, INTEN_t ie)
{
  uint8_t status;
  uint16_t als_conf;
  
  getAlsConfig(sensorNmuber, als_conf);  
  _ConfReg.all = als_conf;
  _ConfReg.ALS_INT_EN = (uint16_t)ie;
  
  _tx_buf[3] = _CMD_W_ALS_CONFIG;
  _tx_buf[4] = (uint8_t)(_ConfReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_ConfReg.all & 0xff00) >> 8);
  
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];  
    if(status == 0)
    {
     return true;  
    }
    else
    {
      return false;
    }
 }
 else
 {
  return false;
 }
}
/***************************************************************************************
Description: Set the VEML7700's persistence protect number setting (ALS_PERS)
Parameters:       
           sensorNmuber : Module Number
           pers :
              PERS_1              
              PERS_2                      
              PERS_4              
              PERS_8   
Return:      true or false
Others:         
****************************************************************************************/
bool BME82M131::setPersistence(uint8_t sensorNmuber, PERS_t pers)
{
  uint8_t status;
  uint16_t als_conf;
  
  getAlsConfig(sensorNmuber, als_conf);  
  _ConfReg.all = als_conf;
  _ConfReg.ALS_PERS = (uint16_t)pers;
  
  _tx_buf[3] = _CMD_W_ALS_CONFIG;
  _tx_buf[4] = (uint8_t)(_ConfReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_ConfReg.all & 0xff00) >> 8);
  
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
     status = _rx_buf[3];
     if(status==0)  
     {
        return true;
     }
     else
     {
        return false;
     }
  }
  else
  {
  return false;
  }
}
/***************************************************************************************
Description: Set the VEML7700's integration time setting (ALS_IT)
Parameters:       
            sensorNmuber : Module Number
            it :
                IT_25ms               
                IT_50ms           
                IT_100ms      
                IT_200ms      
                IT_400ms      
                IT_800ms            
Return:      ture or false
Others:         
****************************************************************************************/
bool BME82M131::setIntegrationTime(uint8_t sensorNmuber, IT_TIME_t it)
{
  uint8_t status;
  uint16_t als_conf;
  
  getAlsConfig(sensorNmuber, als_conf);  
  _ConfReg.all = als_conf;
  _ConfReg.ALS_IT = (uint16_t)configITTime(it);
  
  _tx_buf[3] = _CMD_W_ALS_CONFIG;
  _tx_buf[4] = (uint8_t)(_ConfReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_ConfReg.all & 0xff00) >> 8);
  
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
/***************************************************************************************
Description: Set the VEML7700's gain.
Parameters:      
            sensorNmuber : Module Number
            gain : 
                  GAIN_1              
                  GAIN_2         
                  GAIN_1_8           
                  GAIN_1_4            
Return:     ture or false
Others:         
****************************************************************************************/
bool BME82M131::setGain(uint8_t sensorNmuber, GAIN_t gain)
{
  uint8_t status;
  uint16_t als_conf;
  
  getAlsConfig(sensorNmuber, als_conf);  
  _ConfReg.all = als_conf;
  _ConfReg.ALS_GAIN = (uint16_t)gain;
  
  _tx_buf[3] = _CMD_W_ALS_CONFIG;
  _tx_buf[4] = (uint8_t)(_ConfReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_ConfReg.all & 0xff00) >> 8);
  
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
 else
 {
  return false;
 }
}
/***************************************************************************************
Description: Set the VEML7700's ALS high threshold window setting.
Parameters:       
            sensorNmuber : Module Number
            threshold : 0x0000~0xffff.    
Return:      true or false
Others:         
****************************************************************************************/
bool BME82M131::setHighThreshold(uint8_t sensorNmuber, uint16_t threshold)
{
  uint8_t status=0; 
  _tx_buf[3] = _CMD_W_HIGH_THRESHOLD;
  _tx_buf[4] = (uint8_t)(threshold & 0x00ff);
  _tx_buf[5] = (uint8_t)((threshold & 0xff00) >> 8);
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
/***************************************************************************************
Description: Set the VEML7700's ALS low threshold window setting.
Parameters:       
            sensorNmuber : Module Number
            threshold : 0x0000~0xffff.   
Return:      true or false
Others:         
****************************************************************************************/
bool BME82M131::setLowThreshold(uint8_t sensorNmuber, uint16_t threshold)
{
  uint8_t status;
  
  _tx_buf[3] = _CMD_W_LOW_THRESHOLD;
  _tx_buf[4] = (uint8_t)(threshold & 0x00ff);
  _tx_buf[5] = (uint8_t)((threshold & 0xff00) >> 8);
  delay(15);
  sendData(sensorNmuber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
 else
 {
  return false;
 }  
}

/*----------------------- The following are private functions -----------------------------*/
/***************************************************************************************
Description: Run through initialization procedure for each sensor connected on the bus
Parameters:              
Return:      true or false      
Others:         
****************************************************************************************/
bool BME82M131::initialize()
{
  uint8_t status=0;  
  _ConfReg.all = 0x0000; // Clear the reserved bits
  _ConfReg.ALS_SD = POWER_ON;
  _ConfReg.ALS_INT_EN = INT_DISABLE;
  _ConfReg.ALS_PERS = PERS_1;
  _ConfReg.ALS_IT = configITTime(IT_100ms);
  _ConfReg.ALS_GAIN = GAIN_1;  
  _tx_buf[3] = _CMD_W_ALS_CONFIG;
  _tx_buf[4] = (uint8_t)(_ConfReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_ConfReg.all & 0xff00) >> 8);  
  for(uint8_t i = 1; i <= _SumOfModules; i++)
  {
    delay(15);
    sendData(i, 7, _tx_buf);
    delay(15);
    if(readBytes(_rx_buf,5))
    {
      status=_rx_buf[3];
      if(status==0)
      {
        return true;
      }
      else 
      {
        return false;
      }
     }
     else
     {
      return false;
     }
  }
  return false;
}
/***************************************************************************************
Description: Set the VEML7700's shut down setting (ALS_SD)
Parameters:       
            sensornumber : Module Number
            sd : 
                POWER_ON
                SHUT_DOWN    
Return:     true or false     
Others:         
****************************************************************************************/
bool BME82M131::setStatus(uint8_t sensornumber, SD_t sd)
{
  uint16_t als_conf;
  
  getAlsConfig(sensornumber, als_conf);  
  _ConfReg.all = als_conf;
  _ConfReg.ALS_SD = (uint16_t)sd;
  
  _tx_buf[3] = _CMD_W_ALS_CONFIG;
  _tx_buf[4] = (uint8_t)(_ConfReg.all & 0x00ff);
  _tx_buf[5] = (uint8_t)((_ConfReg.all & 0xff00) >> 8);
  
  delay(15);
  sendData(sensornumber, 7, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,5))
  { 
  return true;
  }
 else
 { 
  return false;
 }
}

/***************************************************************************************
Description: Get the VEML7700's  value of the PSM register.
Parameters:       
            sensornumber : Module Number
            *psm : Stores the value.   
Return:      true or false
Others:         
***********************************************************************************/
bool BME82M131::getPsmReg(uint8_t sensornumber, uint16_t &psm)
{
  uint8_t status;
  
  _tx_buf[3] = _CMD_R_POWER_SAVING_MODE;

  delay(15);
  sendData(sensornumber, 5, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,7))
  {
    status = _rx_buf[3];
    if(status == 0)
    {
      psm = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
      return true;
    }
    else
    {
      psm = 0;
      return false;
    }
  }
  else
  {
    psm = 0;
    return false;
  }  
}
/***************************************************************************************
Description: Get the VEML7700's gain as printable text
Parameters:       
           it:CONF_IT_25ms = 0b1100,      // ALS intetgration time 25ms
              CONF_IT_50ms = 0b1000,      // ALS intetgration time 50ms
              CONF_IT_100ms = 0b0000,     // ALS intetgration time 100ms
              CONF_IT_200ms = 0b0001,     // ALS intetgration time 200ms
              CONF_IT_400ms = 0b0010,     // ALS intetgration time 400ms
              CONF_IT_800ms = 0b0011,     // ALS intetgration time 800ms
Return:      0x00: CMD_TRANSFER_SUCCESS
             0x40: CHECKSUM_ERROR
       0x80: INSTRUCTION_NOT_SUPPOR
       0xA0: SLAVE_NO_RESPONSE
Others:         
****************************************************************************************/
CONF_IT_TIME_t BME82M131::configITTime(IT_TIME_t it)
{
  switch(it)
  {
    case IT_25ms:
    {
      return CONF_IT_25ms;
      break;
    }
    case IT_50ms:
    {
      return CONF_IT_50ms;
      break;
    }
    case IT_100ms:
    {
      return CONF_IT_100ms;
      break;
    }
    case IT_200ms:
    {
      return CONF_IT_200ms;
      break;
    }
    case IT_400ms:
    {
      return CONF_IT_400ms;
      break;
    }
    case IT_800ms:
    {
      return CONF_IT_800ms;
      break;
    }
   default:
   {
      return CONF_IT_INVALID;
      break;
   }
  } 

}
/***************************************************************************************
Description: Get the VEML7700's gain as printable text
Parameters:       
           it:CONF_IT_25ms = 0b1100,      // ALS intetgration time 25ms
              CONF_IT_50ms = 0b1000,      // ALS intetgration time 50ms
              CONF_IT_100ms = 0b0000,     // ALS intetgration time 100ms
              CONF_IT_200ms = 0b0001,     // ALS intetgration time 200ms
              CONF_IT_400ms = 0b0010,     // ALS intetgration time 400ms
              CONF_IT_800ms = 0b0011,     // ALS intetgration time 800ms
             
Return:      true or false.
Others:         
****************************************************************************************/
IT_TIME_t BME82M131::ITTimeFromCONf(CONF_IT_TIME_t it)
{
  switch(it)
  {
    case CONF_IT_100ms:
    {
      return IT_100ms;
      break;
    }
    case CONF_IT_200ms:
    {
      return IT_200ms;
      break;
    }
    case CONF_IT_400ms:
    {
      return IT_400ms;
      break;
    }
    case CONF_IT_800ms:
    {
      return IT_800ms;
      break;
    }
    case CONF_IT_50ms:
    {
      return IT_50ms;
      break;
    }
    case CONF_IT_25ms:
    {
      return IT_25ms;
      break;
    }
   default:
   {
      return IT_INVALID;
      break;
   }
  }
}

/***************************************************************************************
Description: Get register data
Parameters:       
            sensornumber:Module Number      
Return:      true or false       
Others:         
****************************************************************************************/
bool BME82M131::getAlsConfig(uint8_t sensornumber,uint16_t &als_conf)
{
  uint8_t status;
  _tx_buf[3] = _CMD_R_ALS_CONFIG;

  delay(15);
  sendData(sensornumber, 5, _tx_buf);
  delay(15);
  if(readBytes(_rx_buf,7))
  {
    status = _rx_buf[3];
    if(status==0)
    {
      als_conf = (uint16_t)_rx_buf[4] + ((uint16_t)_rx_buf[5] << 8);
      return  true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}
/********************************************************************
Description: Check that the sensor is responding on the I2C bus
Parameters:             
Return:      true or false.    
Others:         
*********************************************************************/
bool BME82M131::isConnected(void)
{
  uint8_t triesBeforeGiveup = 5;
  for (uint8_t x = 0; x < triesBeforeGiveup; x++)
  {
    _i2cPort->beginTransmission(_i2cAddr);
    if (_i2cPort->endTransmission() == 0)
    {
      return true;
    }
    delay(100);
  }
  return false;
}

/*******************************************************************
Description: Write data to the module through I2C
Parameters:      
             id: The module number
             len:Length of data to be written
			       *par: Write to an array of data     
Return:      true or false.    
Others:         
*******************************************************************/
void BME82M131::sendData(uint8_t id, uint8_t len, uint8_t par[])
{  /* Array frame format：
    MID | ID | LEN | CMD/STATUS | DATA0~n | CHECHSUM*/
	par[0] = MODULE_MID;
	par[1] = id;
	par[2] = len - 3;
	par[len - 1] = 0;
	
	for(uint8_t i = 0; i < len - 1; i++)
	{
		par[len - 1] += par[i];
	}
  writeBytes(par,len);
}

/*************************************************
Description:  writeBytes.
Parameters:   wbuf[] : Data to be written.
              wlen : Data length.           
Return:      ture or false       
Others:            
*************************************************/
void BME82M131::writeBytes(uint8_t wbuf[], uint8_t wlen)
{
  while (_i2cPort->available() > 0)
  {
    _i2cPort->read();
  }
  _i2cPort->beginTransmission(_i2cAddr);
  for (uint8_t i = 0; i < wlen; i++)
  {
    _i2cPort->write(wbuf[i]);    
  }
   _i2cPort->endTransmission();
}

/**********************************************************
Description: Read the data of the module through I2C
Parameters:       
          rlen: The length of the data read
    			rbuf[]: Store the read data        
Return:      true or false.    
Others:         
**********************************************************/
bool BME82M131::readBytes( uint8_t rbuf[], uint8_t rlen)
{
  uint8_t i = 0, checkSum = 0;
  _i2cPort->requestFrom(_i2cAddr, rlen);
  if (_i2cPort->available() == rlen)
  {
    for (i = 0; i < rlen; i++)
    {
      rbuf[i] = _i2cPort->read();
    }
  }
  else
  {
    return false;
  }

  /* Check Sum */
  for (i = 0; i < (rlen - 1); i++)
  {
    checkSum += rbuf[i];
  }
  if (checkSum == rbuf[rlen - 1])
  {
    return true; // Check correct
  }
  else
  {
    return false; // Check error
  }
}
