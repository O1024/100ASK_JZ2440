#include "hal/hal_irq.h"
#include "ll_irq.h"

void hal_irq_init(void) {
    ll_irq_init();
}

void hal_irq_enable(int irq_num) {
    ll_irq_enable(irq_num);
}

void hal_irq_disable(int irq_num) {
    ll_irq_disable(irq_num);
}

void hal_irq_global_enable(void) {
    ll_irq_global_enable();
}

void hal_irq_global_disable(void) {
    ll_irq_global_disable();
}

void hal_irq_register(int irq_num, void (*handler)(void)) {
    /* Generic: boundary check + table assignment */
    if (irq_num > 31) return;
    ll_irq_register(irq_num, handler);
}
