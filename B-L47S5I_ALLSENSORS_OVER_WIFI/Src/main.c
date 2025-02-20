/**
  ******************************************************************************
  * @file    Wifi/WiFi_HTTP_Server/src/main.c
  * @author  MCD Application Team
  * @brief   This file provides main program functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license SLA0044,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        http://www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifdef __ICCARM__
#include <LowLevelIOInterface.h>
#endif

/* Private defines -----------------------------------------------------------*/

/*IP AND PORT INITIALIZATION*/ 

uint8_t RemoteIP[] = {192,168,137,1};   //IP ADDRESS OF THE SERVER YOU WANT TO CONNECT 
#define RemotePORT   8002               //PORT ADDRESS OF THE SERVER YOU WANT TO CONNECT 
//#define PASSWORD  "Sheraz12345"         //PASSWORD OF THE WIFI (IF NOT OPEN) YOU WANT TO CONNECT 
#define PASSWORD  "lums12345"

#define PORT           80
#define TERMINAL_USE
#define WIFI_WRITE_TIMEOUT 10000
#define WIFI_READ_TIMEOUT  10000
#define SOCKET                 0

#ifdef  TERMINAL_USE
#define LOG(a) printf a
#else
#define LOG(a)
#endif

#define SSID_SIZE     100
#define PASSWORD_SIZE 100
#define USER_CONF_MAGIC                 0x0123456789ABCDEFuLL
#define CONNECTION_TRIAL_MAX          10
////////////////////////////

/*VARIABLES DEFINED FOR SENSORS USED*/ 

int32_t RecBuf[256];
int32_t PlayBuf[256];

uint8_t halfbufcheck=0;
uint8_t fullbufcheck=0;

uint8_t sound[1024];
//////////////////////
uint8_t address_magnetometer=0;
uint8_t i2c_data_magnetometer[6];
uint8_t config_magnetometer[2];

int16_t magno_x_raw=0;
int16_t magno_y_raw=0;
int16_t magno_z_raw=0;

float magno_x=0;
float magno_y=0;
float magno_z=0;
////////////////
uint8_t address_humidity_tempe=0;
uint8_t i2c_data_humidity_tempe[4];
uint8_t config_humidity_tempe[2];
int T_C0;
int T_C1;
uint8_t T_C0_ini;
uint8_t T_C1_ini;
int16_t T_C0_lsb;
int16_t T_C1_lsb;
float m;
uint8_t H_0=0;
uint8_t H_1=0;
int16_t H_0_lsb=0;
int16_t H_1_lsb=0;
float mh=0;


int16_t humidity_raw=0;
int16_t temperature_raw=0;

float humidity=0;
float temperature=0;
/////////////////
uint8_t address_pressure=0;
uint8_t i2c_data_pressure[3];
uint8_t config_pressure[2];
int32_t pressure_raw=0;

float pressure=0;
/////////////////
uint8_t address_gyro_acc=0;
uint8_t config_gyro_acc[2];
uint8_t i2c_data_gyro_acc[12];
int16_t acc_x_raw=0;
int16_t acc_y_raw=0;
int16_t acc_z_raw=0;
int16_t gyro_x_raw=0;
int16_t gyro_y_raw=0;
int16_t gyro_z_raw=0;

float acc_x=0;
float acc_y=0;
float acc_z=0;
float gyro_x=0;
float gyro_y=0;
float gyro_z=0;
/////////////////
uint8_t address_toF=0;
int8_t config_toF[2];
uint8_t i2c_data_tof[2];
uint8_t i2c_data_tof[2];
uint16_t distance=0;
///////////////////



int process=0;
/* Private typedef------------------------------------------------------------*/

typedef struct {
  char ssid[SSID_SIZE];
  char password[PASSWORD_SIZE];
  uint8_t security;
} wifi_config_t;

typedef struct {
  uint64_t      wifi_config_magic;        /**< The USER_CONF_MAGIC magic word signals that the wifi config
                                               (wifi_config_t) is present in Flash. */
  wifi_config_t wifi_config;
} user_config_t;

   int32_t Socket = -1;
  uint16_t Datalen;
  int32_t ret;
  int16_t Trials = CONNECTION_TRIAL_MAX;
   uint8_t Tim1[115];
  // uint8_t Tim2[91] = " ";
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined (TERMINAL_USE)
extern UART_HandleTypeDef hDiscoUart;
#endif /* TERMINAL_USE */

/* configuration storage in Flash memory */
#if defined(__ICCARM__)
/* IAR */
extern void __ICFEDIT_region_FIXED_LOC_start__;
const  user_config_t    *lUserConfigPtr = &__ICFEDIT_region_FIXED_LOC_start__;
#elif defined(__CC_ARM)
/* Keil / armcc */
user_config_t __uninited_region_start__ __attribute__((section("UNINIT_FIXED_LOC"), zero_init));
const  user_config_t    *lUserConfigPtr = &__uninited_region_start__;
#elif defined(__GNUC__)
/* GNU compiler */
user_config_t __uninited_region_start__ __attribute__((section("UNINIT_FIXED_LOC")));
const  user_config_t    *lUserConfigPtr = &__uninited_region_start__;
#endif


/* Private function prototypes -----------------------------------------------*/
#if defined (TERMINAL_USE)
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */
#endif /* TERMINAL_USE */

static void SystemClock_Config(void);
static int wifi_start(void);
static int wifi_connect(void);

static void Button_ISR(void);
static void Button_Reset(void);
static uint8_t Button_WaitForPush(uint32_t delay);
static void MX_I2C2_Init(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_DFSDM1_Init(void);
static void MX_DMA_Init(void);

static volatile uint8_t button_flag = 0;
static user_config_t user_config;

static  uint8_t  IP_Addr[4];
static  int     LedState = 0; 

I2C_HandleTypeDef hi2c2;
TIM_HandleTypeDef htim2;
DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
DFSDM_Channel_HandleTypeDef hdfsdm1_channel2;
DMA_HandleTypeDef hdma_dfsdm1_flt0;
/* Private functions ---------------------------------------------------------*/


void HTS221_init()
{
   ////initialize here according to your needs
	 config_humidity_tempe[0]=0x10;
   config_humidity_tempe[1]=0x1B;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 2, 10);//set temp  and hum bits
	
   config_humidity_tempe[0]=0x20;
   config_humidity_tempe[1]=0x86;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 2, 10);//Power on

   //For Raw Temperature to celcius
   config_humidity_tempe[0]=0x32;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C0=config_humidity_tempe[0];

   config_humidity_tempe[0]=0x33;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C1=config_humidity_tempe[0];

   config_humidity_tempe[0]=0x35;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C0|=((config_humidity_tempe[0]&0x03)<<8);
   T_C1|=(((config_humidity_tempe[0]&0x0C)>>2)<<8);
   T_C0=T_C0>>3;
   T_C1=T_C1>>3;

   config_humidity_tempe[0]=0x3C;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C0_ini=config_humidity_tempe[0];
   config_humidity_tempe[0]=0x3D;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C0_lsb=(int16_t)(config_humidity_tempe[0]<<8 |T_C0_ini);

   config_humidity_tempe[0]=0x3E;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C1_ini=config_humidity_tempe[0];
   config_humidity_tempe[0]=0x3F;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   T_C1_lsb=(int16_t)(config_humidity_tempe[0]<<8 |T_C1_ini);

   m=(((float)(T_C1-T_C0))/((float)(T_C1_lsb-T_C0_lsb)));

   //For Raw Humidity to Relative Humidity (%RH)
   config_humidity_tempe[0]=0x30;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   H_0=config_humidity_tempe[0];

   config_humidity_tempe[0]=0x31;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   H_1=config_humidity_tempe[0];

   H_0=H_0>>1;
   H_1=H_1>>1;

   config_humidity_tempe[0]=0x36;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   H_0_lsb=config_humidity_tempe[0];

   config_humidity_tempe[0]=0x37;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   H_0_lsb|=config_humidity_tempe[0]<<8;

   config_humidity_tempe[0]=0x3a;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   H_1_lsb=config_humidity_tempe[0];

   config_humidity_tempe[0]=0x3b;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, config_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, config_humidity_tempe, 1, 10);
   H_1_lsb|=config_humidity_tempe[0]<<8;

   mh=((float)(H_1-H_0))/((float)(H_1_lsb-H_0_lsb));


   /*
   //Availability check
   address_humidity_tempe=0x0F;//Who am I register
   HAL_I2C_Master_Transmit(&hi2c2,  0xBE, &address_humidity_tempe, 1, 10);  ///(It should give 188 for HTS221)
   HAL_I2C_Master_Receive(&hi2c2,  0xBF, &address_humidity_tempe, 1, 10);
   */
}



void HTS221_GetRead()
{
   ////////////////////////////Humidity and temperature (HTS221) /////////////////////

   // 0x28 =humidity low   0x29 =humidity high  0x2A =temperature low  0x2B =temperature high
   i2c_data_humidity_tempe[0]=0x28; // Now read from  all the registers individually
   HAL_I2C_Master_Transmit(&hi2c2, 0xBE, i2c_data_humidity_tempe, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0xBF, i2c_data_humidity_tempe, 1, 10);
   i2c_data_humidity_tempe[1]=0x29;
   HAL_I2C_Master_Transmit(&hi2c2, 0xBE,  &i2c_data_humidity_tempe[1], 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0xBF, &i2c_data_humidity_tempe[1], 1, 10);
   i2c_data_humidity_tempe[2]=0x2A;
   HAL_I2C_Master_Transmit(&hi2c2, 0xBE,  &i2c_data_humidity_tempe[2], 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0xBF, &i2c_data_humidity_tempe[2], 1, 10);
   i2c_data_humidity_tempe[3]=0x2B;
   HAL_I2C_Master_Transmit(&hi2c2, 0xBE,  &i2c_data_humidity_tempe[3], 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0xBF, &i2c_data_humidity_tempe[3], 1, 10);

   humidity_raw=(int16_t)(i2c_data_humidity_tempe[1]<<8 |i2c_data_humidity_tempe[0]);
   temperature_raw=(int16_t)(i2c_data_humidity_tempe[3]<<8 |i2c_data_humidity_tempe[2]);

   /////interpolate data from
   temperature=T_C0+(m*temperature_raw); //Celcius
   humidity=H_0+mh*humidity_raw; //% relative Humidity
}

void LPS22HB_init()
{
   ////initialize here according to your needs
   config_pressure[0]=0x10;
   config_pressure[1]=0x20;
   HAL_I2C_Master_Transmit(&hi2c2,  0xBA, config_pressure, 2, 10);//Power on

   /*
   //Availability check
   address_pressure=0x0F;//Who am I register
   HAL_I2C_Master_Transmit(&hi2c2,  0xBA, &address_pressure, 1, 10);  ///(It should give 177 for LPS22HB)
   HAL_I2C_Master_Receive(&hi2c2,  0xBB,  &address_pressure, 1, 10);*/
}

void LPS22HB_GetRead()
{
   //////////////////////////// Pressure (LPS22HB) /////////////////////

   // 0x28 =pressure X_low   0x29 =pressure low   0x2A =pressure high

   i2c_data_pressure[0]=0x28; // Now read from all the registers
   HAL_I2C_Master_Transmit(&hi2c2, 0xBA, i2c_data_pressure, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0xBB, i2c_data_pressure, 3, 10);

   pressure_raw=(int32_t)(i2c_data_pressure[2]<<16 |i2c_data_pressure[1]<<8 |i2c_data_pressure[0]);

   /////interpolate data
   if(pressure_raw>8388607)
   {
      pressure_raw=pressure_raw-1;
      pressure_raw=~pressure_raw;
   }
   pressure=((float)pressure_raw)/4096.0; // in mbar
}

void LIS3MDL_init()
{
   ////Initialize here according to your requirements
   config_magnetometer[0]=0x22;
   config_magnetometer[1]=0x00;
   HAL_I2C_Master_Transmit(&hi2c2,  0x3C, config_magnetometer, 2, 10);//Power on

   /*
   //Availability check
   address_magnetometer=0x0F;//Who am I register
   HAL_I2C_Master_Transmit(&hi2c2,  0x3C, &address_magnetometer, 1, 10);  ///(It should give 61 for LIS3MDL)
   HAL_I2C_Master_Receive(&hi2c2,  0x3D, &address_magnetometer, 1, 10);
   */
}

void LIS3MDL_GetRead()
{
   ////////////////////////////Magnetometer  (LIS3MDL)  /////////////////////

   // 0x28 =Magno X_Low   0x29 =Magno X_High   0x2A =Magno Y_Low   0x2B =Magno Z_High   0x2C =Magno Z_Low   0x2D =Magno Z_High

   i2c_data_magnetometer[0]=0x28; // Now read from all the registers
   HAL_I2C_Master_Transmit(&hi2c2, 0x3C, i2c_data_magnetometer, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0x3D, i2c_data_magnetometer, 6, 10);

   magno_x_raw=(int16_t)(i2c_data_magnetometer[1]<<8 |i2c_data_magnetometer[0]);
   magno_y_raw=(int16_t)(i2c_data_magnetometer[3]<<8 |i2c_data_magnetometer[2]);
   magno_z_raw=(int16_t)(i2c_data_magnetometer[5]<<8 |i2c_data_magnetometer[4]);

   /////interpolate data from raw
   magno_x=magno_x_raw/8.0;
   magno_y=magno_y_raw/8.0;
   magno_z=magno_z_raw/8.0;
}

void LSM6DSL_init()
{
   ////Initialize here according to your requirements
   config_gyro_acc[0]=0x10;
   config_gyro_acc[1]=0x10;
   HAL_I2C_Master_Transmit(&hi2c2, 0xD4, config_gyro_acc, 2, 10);//Accelerometer start
   config_gyro_acc[0]=0x11;
   config_gyro_acc[1]=0x10;
   HAL_I2C_Master_Transmit(&hi2c2, 0xD4, config_gyro_acc, 2, 10);//Gyro start
}

void LSM6DSL_GetRead()
{
   //////////////////////////// Gyroscope and accelerometer (LSM6DSL) /////////////////////

   // 0x22 =GyroX_Low     0x23 =GyroX_High       0x24 =GyroY_Low       0x25 =GyroY_High    0x26 =GyroZ_Low       0x27 =GyroZ_High
   // 0x28 =AccX_Low     0x29 =AccX_High       0x2A =AccY_Low       0x2B =AccY_High    0x2C =AccZ_Low       0x2B =AccZ_High

   i2c_data_gyro_acc[0]=0x22; // Now read from all the registers at once
   HAL_I2C_Master_Transmit(&hi2c2, 0xD4, i2c_data_gyro_acc, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0xD5, i2c_data_gyro_acc, 12, 10);

   gyro_x_raw=(int16_t)(i2c_data_gyro_acc[1]<<8 |i2c_data_gyro_acc[0]);
   gyro_y_raw=(int16_t)(i2c_data_gyro_acc[3]<<8 |i2c_data_gyro_acc[2]);
   gyro_z_raw=(int16_t)(i2c_data_gyro_acc[5]<<8 |i2c_data_gyro_acc[4]);
   acc_x_raw=(int16_t)(i2c_data_gyro_acc[7]<<8 |i2c_data_gyro_acc[6]);
   acc_y_raw=(int16_t)(i2c_data_gyro_acc[9]<<8 |i2c_data_gyro_acc[8]);
   acc_z_raw=(int16_t)(i2c_data_gyro_acc[11]<<8 |i2c_data_gyro_acc[10]);

   /////interpolate data from raw
   acc_x=(acc_x_raw * 244.0 + 500)/1000000; //in g
   acc_y=(acc_y_raw * 244.0 + 500)/1000000;
   acc_z=(acc_z_raw * 244.0 + 500)/1000000;
   gyro_x=(gyro_x_raw * 70000.0 + 500) / 1000000;//in degrees
   gyro_y=(gyro_y_raw * 70000.0 + 500) / 1000000;
   gyro_z=(gyro_z_raw * 70000.0 + 500) / 1000000;
}

void VL53L0X_init()
{
   ////Initialize here according to your requirements
   config_toF[0]=0x00;
   config_toF[1]=0x01;
   HAL_I2C_Master_Transmit(&hi2c2, 0x52, config_toF, 2, 10);//set to continous transition mode
}

void VL53L0X_GetRead()
{
   /////////////////////// Time of flight (VL53L0X) /////////////////////

   //0x01 for a single read     //0x02 for continous read
	 config_toF[0]=0x00;
   config_toF[1]=0x01;
   HAL_I2C_Master_Transmit(&hi2c2, 0x52, config_toF, 2, 10);
 
   i2c_data_tof[0]=0x1E;
   HAL_I2C_Master_Transmit(&hi2c2, 0x52, i2c_data_tof, 1, 10);
   HAL_I2C_Master_Receive(&hi2c2, 0x53, i2c_data_tof, 2, 10);     // Now read from all the register

   /////interpolate data from raw
   distance=(i2c_data_tof[0]<<8)|(i2c_data_tof[1]); // Rest value gives 20 as output so -20
   if(distance<0)
   {
      distance=0;// Value in mm
   }
	 
}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */


int main(void)
{
	int j=0;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure LED2 */
  BSP_LED_Init(LED2);
	
	/* Configure USER push button */
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);  //USER push button is used to ask if reconfiguration is needed
	
	/* Configure all sensors and I/O ports */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
	MX_DMA_Init();
  MX_DFSDM1_Init();
   
  LSM6DSL_init();
  VL53L0X_init();
  LIS3MDL_init();
  LPS22HB_init();
  HTS221_init();
  
	/* Start the sensors and get a reading */
  LSM6DSL_GetRead();
  VL53L0X_GetRead();
  LIS3MDL_GetRead();
  LPS22HB_GetRead();
  HTS221_GetRead();
	HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0,RecBuf,256); // Permanently start sound sensor 
   

  /* WIFI Server */
#if defined (TERMINAL_USE)
  /* Initialize all configured peripherals */
  hDiscoUart.Instance = DISCOVERY_COM1;
  hDiscoUart.Init.BaudRate = 115200;
  hDiscoUart.Init.WordLength = UART_WORDLENGTH_8B;
  hDiscoUart.Init.StopBits = UART_STOPBITS_1;
  hDiscoUart.Init.Parity = UART_PARITY_NONE;
  hDiscoUart.Init.Mode = UART_MODE_TX_RX;
  hDiscoUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hDiscoUart.Init.OverSampling = UART_OVERSAMPLING_16;
  hDiscoUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hDiscoUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;


  BSP_COM_Init(COM1, &hDiscoUart);
  

  printf("\n****** WIFI Server ******\n\n");

#endif /* TERMINAL_USE */

   wifi_connect(); //Establish a wifi connection
  while (Trials--)
  {
      if( WIFI_OpenClientConnection(0, WIFI_TCP_PROTOCOL, "TCP_CLIENT", RemoteIP, RemotePORT, 0) == WIFI_STATUS_OK)
    {
         printf("> TCP Connection opened successfully.\n");
      Socket = 0;
      break;
      }
  }
	
	HAL_TIM_Base_Start_IT(&htim2);



  while(1)
  {
         
      if(halfbufcheck==1)
     {
			  
        for(int i=0;i<256/2;i++)
        {
           PlayBuf[i]=RecBuf[i]>>6;
					 sprintf(&sound[j], "(%.05li)", PlayBuf[i]);
					 j=j+8;

        }
				WIFI_SendData(Socket, sound, sizeof(sound), &Datalen, 0);
        halfbufcheck=0;
				j=0;				
     }
     if(fullbufcheck==1)
     {
			  
        for(int i=256/2;i<256;i++)
        {
           PlayBuf[i]=RecBuf[i]>>6;
					 sprintf(&sound[j], "(%.05li)", PlayBuf[i]);
					 j=j+8;
           
        }
				
				WIFI_SendData(Socket, sound, sizeof(sound), &Datalen, 0);		
				fullbufcheck=0;
				j=0;
     }
		 if(process==1)
		 {
			 WIFI_SendData(Socket, Tim1, sizeof(Tim1), &Datalen, 0);
			 process=0;		 
		 }

  }

}

/**
  * @brief  Start Wifi
  * @param  None
  * @retval None
  */


static int wifi_start(void)
{
  uint8_t  MAC_Addr[6];

 /*Initialize and use WIFI module */
  if(WIFI_Init() ==  WIFI_STATUS_OK)
  {
    printf("eS-WiFi Initialized.\n");
    if(WIFI_GetMAC_Address(MAC_Addr) == WIFI_STATUS_OK)
    {
      LOG(("eS-WiFi module MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
               MAC_Addr[0],
               MAC_Addr[1],
               MAC_Addr[2],
               MAC_Addr[3],
               MAC_Addr[4],
               MAC_Addr[5]));
    }
    else
    {
      LOG(("> ERROR : CANNOT get MAC address\n"));
      return -1;
    }
  }
  else
  {
    return -1;
  }
  return 0;
}



int wifi_connect(void)
{
  wifi_start();
  
  memset(&user_config, 0, sizeof(user_config));
  memcpy(&user_config, lUserConfigPtr, sizeof(user_config));
  if (user_config.wifi_config_magic == USER_CONF_MAGIC)
  {
    /* WiFi configuration is already in Flash. Ask if we want to change it */
    printf("Already configured SSID: %s security: %d\n",
           user_config.wifi_config.ssid, user_config.wifi_config.security);
    printf("Press board User button (blue) within 5 seconds if you want to change the configuration.\n");
    Button_Reset();
    if (Button_WaitForPush(5000))
    {
      /* we want to change the configuration already stored in Flash memory */
      memset(&user_config, 0, sizeof(user_config));
    }
  }

  if (user_config.wifi_config_magic != USER_CONF_MAGIC)
  {
    printf("\nEnter WiFi SSID : ");
    gets(user_config.wifi_config.ssid);
    LOG(("\nYou have entered %s as SSID.\n", user_config.wifi_config.ssid));

    char c;
    do
    {
        printf("\rEnter Security Mode (0 - Open, 1 - WEP, 2 - WPA, 3 - WPA2): ");
        c = getchar();
    }
    while ( (c < '0')  || (c > '3'));
    user_config.wifi_config.security = c - '0';
    LOG(("\nYou have entered %d as the security mode.\n", user_config.wifi_config.security));

    if (user_config.wifi_config.security != 0)
    {
      printf("\nUsing Password as entered in the code: ");
      //gets(user_config.wifi_config.password);
			strcpy(user_config.wifi_config.password,PASSWORD);
    }
    user_config.wifi_config_magic = USER_CONF_MAGIC;
    FLASH_Erase_Size((uint32_t)lUserConfigPtr, sizeof(user_config));
    FLASH_Write((uint32_t)lUserConfigPtr, (uint32_t*)&user_config, sizeof(user_config));
  }
  
  printf("\nConnecting to %s\n", user_config.wifi_config.ssid);
	
  WIFI_Ecn_t security;
	
  switch (user_config.wifi_config.security)
  {
    case 0:
      security = WIFI_ECN_OPEN;
      break;
    case 1:
      security = WIFI_ECN_WEP;
      break;
    case 2:
      security =  WIFI_ECN_WPA_PSK;
      break;
    case 3:
    default:
      security =  WIFI_ECN_WPA2_PSK;
      break;
  }
  if (WIFI_Connect(user_config.wifi_config.ssid, user_config.wifi_config.password, security) == WIFI_STATUS_OK)
  {
    if(WIFI_GetIP_Address(IP_Addr) == WIFI_STATUS_OK)
    {
      LOG(("eS-WiFi module connected: got IP Address : %d.%d.%d.%d\n",
               IP_Addr[0],
               IP_Addr[1],
               IP_Addr[2],
               IP_Addr[3]));
    }
    else
    {
      LOG((" ERROR : es-wifi module CANNOT get IP address\n"));
      return -1;
    }
  }
  else
  {
     LOG(("ERROR : es-wifi module NOT connected\n"));
     return -1;
  }
  return 0;
}


/*!!!!!!!!!!!!!!!!!DISCLAIMER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//SYSTEM CLOCK CONFIGRATIONS MIGHT HAVE BEEN CHANGED TO GET BETTER RESULTS AND THE BRIEF BELOW MIGHT BE INACCURATE
// CHECK  SystemClock_Config(void) TO SEE THE ACTUAL VALUES
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLR_DIV2;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x307075B1;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

static void MX_DFSDM1_Init(void)
{

  /* USER CODE BEGIN DFSDM1_Init 0 */

  /* USER CODE END DFSDM1_Init 0 */

  /* USER CODE BEGIN DFSDM1_Init 1 */

  /* USER CODE END DFSDM1_Init 1 */
  hdfsdm1_filter0.Instance = DFSDM1_Filter0;
  hdfsdm1_filter0.Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
  hdfsdm1_filter0.Init.RegularParam.FastMode = ENABLE;
  hdfsdm1_filter0.Init.RegularParam.DmaMode = ENABLE;
  hdfsdm1_filter0.Init.FilterParam.SincOrder = DFSDM_FILTER_SINC3_ORDER;
  hdfsdm1_filter0.Init.FilterParam.Oversampling = 25;
  hdfsdm1_filter0.Init.FilterParam.IntOversampling = 39;   //  This controls the quality of output sound. Decreasing will decrease the quality but increase the sampling rate
  if (HAL_DFSDM_FilterInit(&hdfsdm1_filter0) != HAL_OK)
  {
    Error_Handler();
  }
  hdfsdm1_channel2.Instance = DFSDM1_Channel2;
  hdfsdm1_channel2.Init.OutputClock.Activation = ENABLE;
  hdfsdm1_channel2.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_SYSTEM;
  hdfsdm1_channel2.Init.OutputClock.Divider = 15;
  hdfsdm1_channel2.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hdfsdm1_channel2.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
  hdfsdm1_channel2.Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
  hdfsdm1_channel2.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
  hdfsdm1_channel2.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hdfsdm1_channel2.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
  hdfsdm1_channel2.Init.Awd.Oversampling = 1;
  hdfsdm1_channel2.Init.Offset = 0;
  hdfsdm1_channel2.Init.RightBitShift = 0x00;
  if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter0, DFSDM_CHANNEL_2, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DFSDM1_Init 2 */

  /* USER CODE END DFSDM1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMAMUX1_OVR_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMAMUX1_OVR_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMAMUX1_OVR_IRQn);

}
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
   halfbufcheck=1;
}
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
   fullbufcheck=1;
}

static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7000- 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000 -1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.RepetitionCounter = 0;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	if (htim->Instance == TIM1) 
	{
		HAL_IncTick();
	}  

  /* USER CODE END Callback 0 */

   
  /* USER CODE BEGIN Callback 1 */
	if (htim->Instance == TIM2) 
	{
			
		HTS221_GetRead();
    LSM6DSL_GetRead();
		VL53L0X_GetRead();
    LIS3MDL_GetRead();
    LPS22HB_GetRead();

		sprintf (Tim1, "|%06.1f|!%06.1f!$%06.1f$*%.5d*^%08.2fx%08.2fx%08.2f^@%08.2fx%08.2fx%08.2f@#%08.2fx%08.2fx%08.2f#", temperature,humidity,pressure,distance,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,magno_x,magno_y,magno_z);
		process=1;
    
	}

  /* USER CODE END Callback 1 */
}


/**
  * @brief Reset button state
  *        To be called before Button_WaitForPush()
  */
void Button_Reset()
{
  button_flag = 0;
}


/**
  * @brief Waiting for button to be pushed
  */
uint8_t Button_WaitForPush(uint32_t delay)
{
  uint32_t time_out = HAL_GetTick() + delay;

  do
  {
    if (button_flag > 0)
    {
      return button_flag;
    }
    HAL_Delay(100);
  }
  while (HAL_GetTick() < time_out);

  return 0;
}

#if defined (TERMINAL_USE)
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&hDiscoUart, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}


#ifdef __ICCARM__
/**
  * @brief  
  * @param  
  * @retval 
  */
size_t __read(int handle, unsigned char * buffer, size_t size)
{
  int nChars = 0;

  /* handle ? */

  for (/* Empty */; size > 0; --size)
  {
    uint8_t ch = 0;
    while (HAL_OK != HAL_UART_Receive(&hDiscoUart, (uint8_t *)&ch, 1, 30000))
    {
      ;
    }

    *buffer++ = ch;
    ++nChars;
  }

  return nChars;
}
#elif defined(__CC_ARM) || defined(__GNUC__)
/**
  * @brief  Retargets the C library scanf function to the USART.
  * @param  None
  * @retval None
  */
GETCHAR_PROTOTYPE
{
  /* Place your implementation of fgetc here */
  /* e.g. read a character on USART and loop until the end of read */
  uint8_t ch = 0;
  while (HAL_OK != HAL_UART_Receive(&hDiscoUart, (uint8_t *)&ch, 1, 30000))
  {
    ;
  }
  return ch;
}
#endif /* defined(__CC_ARM)  */
#endif /* TERMINAL_USE */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif


/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin)
  {
    case (USER_BUTTON_PIN):
    {
      Button_ISR();
      break;
    }
    case (GPIO_PIN_1):
    {
      SPI_WIFI_ISR();
      break;
    }
    default:
    {
      break;
    }
  }
}

/**
  * @brief  SPI3 line detection callback.
  * @param  None
  * @retval None
  */
void SPI3_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi);
}

/**
  * @brief Update button ISR status
  */
static void Button_ISR(void)
{
  button_flag++;
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

