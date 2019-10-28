#ifndef _FTM_H_
#define _FTM_H_


/*
       ,--.-----.--.
       |--|-----|--|
       |--|     |--|
       |  |-----|  |
     __|--|     |--|__
    /  |  |-----|  |  \
   /   \__|-----|__/   \
  /   ______---______   \/\
 /   /  11  1 2 / 1  \   \/
{   /10    ROLEX     2\   }
|  {   ,_.    /  ,_.   }  |-,
|  |9 {   }  O--{-  } 3|  | |
|  {   `-'  /    `-'   }  |-'
{   \8   7 /     5   4/   }
 \   `------_6_------'   /\
  \     __|-----|__     /\/
   \   /  |-----|  \   /
    \  |--|     |--|  /
     --|  |-----|  |--
       |--|     |--|
       |--|-----|--|
       `--'-----`--'
*/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "ports.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef void(*FTM_callback)(void);

typedef enum {  FTM_PSC_x1,
                FTM_PSC_x2,
                FTM_PSC_x4,
                FTM_PSC_x8,
                FTM_PSC_x16,
                FTM_PSC_x32,
                FTM_PSC_x64,
                FTM_PSC_x128 } FTM_prescal_t;

typedef enum {  FTM_CLOCK_NONE,
                FTM_CLOCK_SYSTEM,
                FTM_CLOCK_FIXED_FREQ,
                FTM_CLOCK_EXT } FTM_clockSource_t;

typedef enum {  FTM_MODE_INPUT_CAPTURE,
                FTM_MODE_OUTPUT_COMPARE,
                FTM_MODE_EPWM,
                FTM_MODE_CPWM,
//                FTM_MODE_COMBINE,
//                FTM_MODE_COMPLEMENTARY,
                FTM_MODE_N} FTM_mode_t;

typedef enum {  FTM_CONFIG_CAPTURE_RISING,
                FTM_CONFIG_CAPTURE_FALLING,
                FTM_CONFIG_CAPTURE_BOTH,
                FTM_CONFIG_TOGGLE_OUTPUT,
                FTM_CONFIG_CLEAR_OUTPUT,
                FTM_CONFIG_SET_OUTPUT,
				FTM_CONFIG_HIGH_TRUE,
				FTM_CONFIG_LOW_TRUE,
                FTM_CONFIG_N} FTM_config_t;

typedef struct {
    FTM_mode_t mode;
    FTM_config_t config;
    uint8_t channel;    //Para FTM0 y FTM3: entre 0 y 7.
                        //Para FTM1 y FTM2: entre 1 y 1.
    bool interruptsEnabled;
    FTM_callback inputCallback;
	pin_t pin;
} FTM_inputCaptureData_t;

typedef struct {
    FTM_mode_t mode;
    FTM_config_t config;
    uint8_t channel;    //Para FTM0 y FTM3: entre 0 y 7.
                        //Para FTM1 y FTM2: entre 1 y 1.
    bool interruptsEnabled;
    FTM_callback outputCallback;
	pin_t pin;
    uint8_t value;       //Para CnV
} FTM_outputCompareData_t;

typedef struct {
    FTM_mode_t mode;
    FTM_config_t config;
    uint8_t channel;    //Para FTM0 y FTM3: entre 0 y 7.
                        //Para FTM1 y FTM2: entre 0 y 1.
    bool interruptsEnabled;
    FTM_callback EPWM_callback;
	pin_t pin;
    uint8_t duty;       //0: duty=0%, 255: duty 100%
    bool restartRegisters;	//Habilitar las funciones de PWMLOAD. Debe estar habilitado ademas en todo_ el modulo
} FTM_EPWM_data_t;

typedef struct {
    FTM_mode_t mode;
    FTM_config_t config;
    uint8_t channel;    //Para FTM0 y FTM3: entre 0 y 7.
                        //Para FTM1 y FTM2: entre 0 y 1.
    bool interruptsEnabled;
    FTM_callback CPWM_callback;
	pin_t pin;
    uint8_t duty;       //0: duty=0%, 255: duty 100%
} FTM_CPWM_data_t;

// IMPORTANTE: TODOS LOS ELEMENTOS DE LA UNION TIENE QUE EMPEZAR
// CON LAS MISMAS VARIABLES!
typedef union {
    struct{
    	FTM_mode_t mode;
    	FTM_config_t config;
    	uint8_t channel;    //Para FTM0 y FTM3: entre 0 y 7.
                        	//Para FTM1 y FTM2: entre 0 y 1.
    	bool interruptsEnabled;
    	FTM_callback callback;
    	pin_t pin;
    };
    FTM_inputCaptureData_t  inputCaptureData;
    FTM_outputCompareData_t outputCompareData;
    FTM_EPWM_data_t         EPWM_data;
    FTM_CPWM_data_t         CPWM_data;
} FTM_channelData_t;

typedef struct {
    uint8_t FTM_n;  //Entre 0 y 3
    FTM_clockSource_t clockSource;
    FTM_prescal_t prescaler;
    uint16_t cntin; //cntin es el valor inicial del contador.
    uint16_t mod;   //mod-1 es el valor maximo del contador. Al superarlo, se llama al overflowCallback.
    FTM_channelData_t * channelConfigsBuff;
    uint8_t channelConfigsBuffLen;
    FTM_callback overflowCallback;
    bool interruptsEnabled;
    bool restartRegisters;	//Habilitar las funciones de PWMLOAD. Cada canal debe indicar si lo quiere utilizar o no
} FTM_initData_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Funcion de inicializacion de FTM
 *
 * @param data estructura con los parametros de configuracion del periferico.
 */
void FTM_init(FTM_initData_t * data);

/**
 * @brief Setea el duty de la funcion PWM. Solo tiene efecto en los canales
 * configurados como EPWM
 *
 * @param FTM_n Entre 0 y 3
 * @param channel //Para FTM0 y FTM3: entre 0 y 7.
                  //Para FTM1 y FTM2: entre 0 y 1.
 * @param duty 0: duty=0%, 0xFF: duty 100%
 */
void FTM_setDuty8bits(uint8_t FTM_n, uint8_t channel, uint8_t duty);


/**
 * @brief Setea el duty de la funcion PWM. Solo tiene efecto en los canales
 * configurados como EPWM
 *
 * @param FTM_n Entre 0 y 3
 * @param channel //Para FTM0 y FTM3: entre 0 y 7.
                  //Para FTM1 y FTM2: entre 0 y 1.
 * @param duty 0: duty=0%, 0x0FF0 a 0x0FFF: duty 100%. Los 4 bits menos
 * significativos son ignorados.
 */
void FTM_setDuty12bits(uint8_t FTM_n, uint8_t channel, uint16_t duty);

/**
 * @brief Deshabilita el modulo deseleccionando los clocks.
 *
 * @param FTM_n Entre 0 y 3.
 */
void FTM_shutdown(uint8_t FTM_n);

/**
 * @brief Deshabilita un canal
 *
 * @param FTM_n Entre 0 y 3.
 * @param channel //Para FTM0 y FTM3: entre 0 y 7.
                  //Para FTM1 y FTM2: entre 0 y 1.
 */
void FTM_shutdownChannel(uint8_t FTM_n, uint8_t channel);

/**
 * @brief Devuelve el ultimo periodo detectado. Solo tiene efecto en los canales
 * configurados como input capture.
 *
 * Devuelve el tiempo en unidades de 10e-8 s
 *
 * @param FTM_n Entre 0 y 3.
 * @param channel //Para FTM0 y FTM3: entre 0 y 7.
                  //Para FTM1 y FTM2: entre 0 y 1.
 */
int32_t FTM_getPeriod(uint8_t FTM_n, uint8_t channel);



/*******************************************************************************
 ******************************************************************************/

#endif // _FTM_H_
