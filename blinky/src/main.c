/* -------------------------------------------------------------------------- 
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    BLinky.c
 *      Purpose: RTX example program
 *
 *---------------------------------------------------------------------------*/

#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS

#include "interface_elevador.h"
#include "controleElevador.h"

//eventoElevador_t event_aux;
/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {
//  tid_phaseA = osThreadNew(phaseA, NULL, NULL);
//  tid_phaseB = osThreadNew(phaseB, NULL, NULL);
//  tid_phaseC = osThreadNew(phaseC, NULL, NULL);
//  tid_phaseD = osThreadNew(phaseD, NULL, NULL);
//  tid_clock  = osThreadNew(clock,  NULL, NULL);
//
//  phases_mut_id = osMutexNew(&Phases_Mutex_attr);
//  
//  osThreadFlagsSet(tid_phaseA, 0x0001);          /* set signal to phaseA thread   */
  


  uint8_t retorno = controleElevadorInit();
  retorno = retorno && interfaceElevadorInit();
  if(retorno == 0)
  {
    while(1);
  }
//uint8_t i =0;
//while(1)
//{
//  osMessageQueueGet(queueEventosElevadorID,&event_aux,NULL,osWaitForever);
//
//  i++;
//}
  osDelay(osWaitForever);
  while(1);
}

int main (void) {

  // System Initialization
  SystemInit(); 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, NULL);    // Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }

  while(1);
}
