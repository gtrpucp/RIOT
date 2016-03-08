/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_msp430fxyz
 * @{
 *
 * @file
 * @brief       Low-level GPIO driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "cpu.h"
#include "bitarithm.h"
#include "periph/gpio.h"

/**
 * @brief   Number of possible interrupt lines: 2 ports * 8 pins
 */
#define ISR_NUMOF           (16U)

/**
 * @brief   Number of pins on each port
 */
#define PINS_PER_PORT       (8U)

/**
 * @brief   Interrupt context for each interrupt line
 */
static gpio_isr_ctx_t isr_ctx[ISR_NUMOF];


static msp_port_t *_port(gpio_t pin)
{
    switch (pin >> 8) {
        case 1:
            return PORT_1;
        case 2:
            return PORT_2;
        case 3:
            return PORT_3;
        case 4:
            return PORT_4;
        case 5:
            return PORT_5;
        case 6:
            return PORT_6;
        default:
            return NULL;
    }
}

static inline msp_port_isr_t *_isr_port(gpio_t pin)
{
    msp_port_t *p = _port(pin);
    if ((p == PORT_1) || (p == PORT_2)) {
        return (msp_port_isr_t *)p;
    }
    return NULL;
}

static inline uint8_t _pin(gpio_t pin)
{
    return (uint8_t)(pin & 0xff);
}

static int _ctx(gpio_t pin)
{
    int i = bitarithm_lsb(_pin(pin));
    return (_port(pin) == PORT_1) ? i : (i + 8);
}

int gpio_init(gpio_t pin, gpio_dir_t dir, gpio_pp_t pullup)
{
    msp_port_t *port = _port(pin);
    /* check if port is valid */
    if (port == NULL) {
        return -1;
    }

    /* set pin direction */
    port->DIR &= ~(_pin(pin));
    port->DIR |= (dir & _pin(pin));

    /* set pullup/pulldown resistor */
    if(port == PORT_1) {
        P1REN &= ~(_pin(pin));
        P1REN |= (pullup & _pin(pin));
    }
    else if(port == PORT_2) {
        P2REN &= ~(_pin(pin));
        P2REN |= (pullup & _pin(pin));
    }
    else if(port == PORT_3) {
        P3REN &= ~(_pin(pin));
        P3REN |= (pullup & _pin(pin));
    }
    else if(port == PORT_4) {
        P4REN &= ~(_pin(pin));
        P4REN |= (pullup & _pin(pin));
    }
    else if(port == PORT_5) {
        P5REN &= ~(_pin(pin));
        P5REN |= (pullup & _pin(pin));
    }
    else if(port == PORT_6) {
        P6REN &= ~(_pin(pin));
        P6REN |= (pullup & _pin(pin));
    }

    /* Set output direction */
    if(pullup == GPIO_PULLUP)
        /* if pullup selected then output must be high*/
        port->OD |= _pin(pin);
        /* for other cases the output is low*/
    else
        port->OD &= ~(_pin(pin));

    return 0;
}

int gpio_init_int(gpio_t pin, gpio_pp_t pullup, gpio_flank_t flank,
                    gpio_cb_t cb, void *arg)
{
    msp_port_isr_t *port = _isr_port(pin);

    /* check if port, pull resistor and flank configuration are valid */
    if ((port == NULL) || (pullup != GPIO_NOPULL) || (flank == GPIO_BOTH)) {
        return -1;
    }

    /* disable any activated interrupt */
    port->IE &= ~(_pin(pin));
    /* configure as input */
    gpio_init(pin, GPIO_DIR_IN, GPIO_NOPULL);
    /* save ISR context */
    isr_ctx[_ctx(pin)].cb = cb;
    isr_ctx[_ctx(pin)].arg = arg;
    /* configure flank */
    port->IES &= ~(_pin(pin));
    port->IES |= (flank & _pin(pin));
    /* clear pending interrupts and enable the IRQ */
    port->IFG &= ~(_pin(pin));
    gpio_irq_enable(pin);
    return 0;
}

void gpio_periph_mode(gpio_t pin, bool enable)
{
    REG8 *sel;
    msp_port_isr_t *isrport = _isr_port(pin);
    if (isrport) {
        sel = &(isrport->SEL);
    }
    else {
        msp_port_t *port = _port(pin);
        if (port) {
            sel = &(port->SEL);
        }
        else {
            return;
        }
    }
    if (enable) {
        *sel |= _pin(pin);
    }
    else {
        *sel &= ~(_pin(pin));
    }
}

void gpio_irq_enable(gpio_t pin)
{
    msp_port_isr_t *port = _isr_port(pin);
    if (port) {
        port->IE |= _pin(pin);
    }
}

void gpio_irq_disable(gpio_t pin)
{
    msp_port_isr_t *port = _isr_port(pin);
    if (port) {
        port->IE &= ~(_pin(pin));
    }
}

int gpio_read(gpio_t pin)
{
    msp_port_t *port = _port(pin);
    if (port->DIR & _pin(pin)) {
        return (int)(port->OD & _pin(pin));
    }
    else {
        return (int)(port->IN & _pin(pin));
    }
}

void gpio_set(gpio_t pin)
{
    _port(pin)->OD |= _pin(pin);
}

void gpio_clear(gpio_t pin)
{
    _port(pin)->OD &= ~(_pin(pin));
}

void gpio_toggle(gpio_t pin)
{
    _port(pin)->OD ^= _pin(pin);
}

void gpio_write(gpio_t pin, int value)
{
    if (value) {
        _port(pin)->OD |= _pin(pin);
    }
    else {
        _port(pin)->OD &= ~(_pin(pin));
    }
}

static inline void isr_handler(msp_port_isr_t *port, int ctx)
{
    for (int i = 0; i < PINS_PER_PORT; i++) {
        if ((port->IE & (1 << i)) && (port->IFG & (1 << i))) {
            port->IFG &= ~(1 << i);
            isr_ctx[i + ctx].cb(isr_ctx[i + ctx].arg);
        }
    }
}

ISR(PORT1_VECTOR, isr_port1)
{
    __enter_isr();
    isr_handler((msp_port_isr_t *)PORT_1, 0);
    __exit_isr();
}

ISR(PORT2_VECTOR, isr_port2)
{
    __enter_isr();
    isr_handler((msp_port_isr_t *)PORT_2, 8);
    __exit_isr();
}
