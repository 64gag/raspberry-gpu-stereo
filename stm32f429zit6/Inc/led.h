#ifndef __LED_H
#define __LED_H

typedef enum 
{
  LED3 = 0,
  LED4 = 1
} Led_TypeDef;

void      BSP_LED_Init(Led_TypeDef Led);
void      BSP_LED_On(Led_TypeDef Led);
void      BSP_LED_Off(Led_TypeDef Led);
void      BSP_LED_Toggle(Led_TypeDef Led);

#endif