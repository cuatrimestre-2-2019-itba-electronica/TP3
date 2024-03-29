#ifndef CMP_H_
#define CMP_H_


/*

            ,ggg,                   gg                   ,ggg,
           d8P""8b                ,d88b,                d8""Y8b
           Y8b,__,,aadd88888bbaaa,888888,aaadd88888bbaa,,__,d8P
            "88888888888888888888I888888I88888888888888888888"
            /|\`""YY8888888PP""""`888888'""""YY8888888PP""'/|\
           / | \                  `WWWW'                  / | \
          /  |  \                 ,dMMb,                 /  |  \
         /   |   \                I8888I                /   |   \
        /    |    \               `Y88P'               /    |    \
       /     |     \               `YP'               /     |     \
      /      |      \               88               /      |      \
     /       |       \             i88i             /       |       \
    /        |        \            8888            /        |        \
"Y88888888888888888888888P"       i8888i       "Y88888888888888888888888P"
  `""Y888888888888888P""'        ,888888,        `""Y888888888888888P""'
                                 I888888I
                                 Y888888P
                                 `Y8888P'
                                  `WWWW'
                                   dMMb
                                _,ad8888ba,_
                    __,,aaaadd888888888888888bbaaaa,,__
                  d8888888888888888888888888888888888888b
                  Normand Veilleux

 */


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
