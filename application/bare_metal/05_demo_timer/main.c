/**
 * @file main.c
 * @brief Professional Timer Interrupt Application
 * 
 * This example demonstrates:
 * 1. Hardware Timer configuration (Timer 4 set to 1000ms).
 * 2. Interrupt Service Routine (ISR) registration and handling.
 * 3. Thread-safe variable sharing using the 'volatile' keyword.
 * 4. Periodic telemetry reporting via UART.
 */

#include "hal/hal_gpio.h"
#include "hal/hal_uart.h"
#include "hal/hal_clock.h"
#include "hal/hal_timer.h"
#include "hal/hal_irq.h"
#include "hal/hal_sdram.h"
#include <stdint.h>

/* Boot helper defined in relocate.c */
extern void hal_system_init(void);

#define HEARTBEAT_LED   GPF4
#define UART_BAUD_RATE  115200
#define TIMER_INTERVAL_MS 1000

/* 
 * 'volatile' is mandatory here because this variable is modified asynchronously 
 * inside the ISR and read synchronously in the main loop. Without 'volatile', 
 * the compiler's optimizer might cache the value in a register, causing the 
 * main loop to never see updates.
 */
static volatile uint32_t g_timer_ticks = 0;

/**
 * @brief Interrupt Service Routine (ISR) for Timer 4
 * 
 * Executes exactly once every TIMER_INTERVAL_MS. 
 * Keep ISRs as short and fast as possible: increment counters, set flags, 
 * or flip GPIOs. Do NOT use blocking functions (like hal_delay or UART prints) here.
 */
static void timer4_isr(void) {
    g_timer_ticks++;
    hal_gpio_toggle(HEARTBEAT_LED);
}

/**
 * @brief Initialize basic hardware peripherals
 */
static void hw_init(void) {
    /* Initialize UART for telemetry */
    hal_uart_init(UART_BAUD_RATE);
    
    /* Initialize heartbeat LED (Active Low, default Off) */
    hal_gpio_init_output(HEARTBEAT_LED);
    hal_gpio_set(HEARTBEAT_LED, GPIO_HIGH);
}

/**
 * @brief Configure and enable the Timer and Interrupt Controller
 */
static void timer_irq_init(void) {
    /* 1. Initialize the global interrupt controller */
    hal_irq_init();
    
    /* 2. Register our custom ISR for Timer 4 */
    hal_timer4_set_handler(timer4_isr);
    
    /* 3. Configure hardware timer to trigger every 1000ms */
    hal_timer4_init(TIMER_INTERVAL_MS); 
    
    /* 4. Start the timer counting */
    hal_timer4_start();
    
    /* 5. Enable interrupts globally at the CPU (CPSR register) */
    hal_irq_global_enable();
}

/**
 * @brief Simple utility to print a 3-digit number via UART
 */
static void print_ticks(uint32_t ticks) {
    hal_uart_puts("[Tick: ");
    hal_uart_putc('0' + (ticks / 100) % 10);
    hal_uart_putc('0' + (ticks / 10) % 10);
    hal_uart_putc('0' + ticks % 10);
    hal_uart_puts("] LED Toggled!\r\n");
}

/**
 * @brief Application entry point.
 */
int main(void) {
    /* 1. Low-level Hardware Setup (Clock/SDRAM if needed) */
#ifdef TARGET_SDRAM
    hal_clock_init();
    hal_sdram_init();
#endif

    /* 2. Initialize C Runtime (Relocate data, clear BSS) */
    hal_system_init();
    
    /* Explicitly reset ticks to guarantee clean state regardless of BSS */
    g_timer_ticks = 0;

    /* 3. Peripheral Initialization */
#ifndef TARGET_SDRAM
    hal_clock_init();
#endif
    hw_init();

    hal_uart_puts("\r\n========================================\r\n");
    hal_uart_puts("      Professional Timer Interrupt      \r\n");
    hal_uart_puts("========================================\r\n");

    /* 4. Initialize Interrupts and Timer */
    timer_irq_init();

    /* 5. Main Telemetry Loop */
    uint32_t last_ticks = 0;
    while (1) {
        /* Safely read the volatile variable */
        uint32_t current_ticks = g_timer_ticks;
        
        if (current_ticks != last_ticks) {
            last_ticks = current_ticks;
            print_ticks(last_ticks);
        }
    }
    return 0;
}
