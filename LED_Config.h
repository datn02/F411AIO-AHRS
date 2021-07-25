#ifndef _LED_CONF_H_
#define _LED_CONF_H_

#define LED_TOGGLE 	GPIOC->ODR ^= (1 << 13) | (1 << 14)
#define LED_ON 			GPIOC->ODR &= ~((1 << 13) | (1 << 14))
#define LED_OFF 		GPIOC->ODR |= (1 << 13) | (1 << 14)

void ledSetup(void);


#endif
