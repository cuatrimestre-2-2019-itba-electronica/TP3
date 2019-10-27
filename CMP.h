#ifndef CMP_H_
#define CMP_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {CMP_hysteresisLevel0,
				CMP_hysteresisLevel1,
				CMP_hysteresisLevel2,
				CMP_hysteresisLevel3 } CMP_hysteresisLevel_t;

typedef struct {
	uint8_t CMP_n; 		//Entre 0 y 2
	CMP_hysteresisLevel_t hysteresis;	//
	uint8_t outputPinEnable;
	uint8_t level;	//nivel del comparador. 0: Vmin - 255: Vmax
} CMP_initData_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void CMP_init(CMP_initData_t * data);

/*******************************************************************************
 ******************************************************************************/

#endif /* CMP_H_ */
