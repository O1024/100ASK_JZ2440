#ifndef __LL_IRQ_H__
#define __LL_IRQ_H__

#include <stdint.h>

void ll_irq_init(void);
void ll_irq_register(int irq_num, void (*handler)(void));
void ll_irq_enable(int irq_num);
void ll_irq_disable(int irq_num);
void ll_irq_global_enable(void);
void ll_irq_global_disable(void);

#endif /* __LL_IRQ_H__ */
