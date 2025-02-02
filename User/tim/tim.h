#ifndef __TIM_H
#define __TIM_H
#ifdef __cplusplus
extern "C" {
#endif

void TIM3_Init(uint16_t arr, uint16_t psc);
void TIM4_Init(uint16_t arr, uint16_t psc);

#ifdef __cplusplus
}
#endif
#endif // !__TIM_H
