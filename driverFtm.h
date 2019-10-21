/*

 */

 /*
 * driverFtm.h
 *
 *  Created on: Oct 16, 2019
 *      Author: mlste
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
|  {     ,_.    /  ,_. }  |-,
|  |9  {   }  O--{- } 3|  | |
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

#ifndef DRIVERFTM_H_
#define DRIVERFTM_H_
#include <stdint.h>
void driverFtmInit(int witchFtm);
void driverFtmSetPwmDutyChanel(uint8_t chanel,uint8_t pwm);


#endif /* DRIVERFTM_H_ */
