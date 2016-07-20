/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   board_msba2
 * @{
 *
 * @file
 * @brief     cc110x board specific configuration
 *
 * @author    Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef CC110X_PARAMS_H
#define CC110X_PARAMS_H

#include <arduino_pinmap.h>
#include "cc110x.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name CC110X configuration
 */
const cc110x_params_t cc110x_params[] = {
    {
        .spi  = 0,
        .cs   = ARDUINO_PIN_40,  //GPIO_PIN(PA, 29), para CS:4,10 y 52. UPDATE: Se probo con el PIN 40 y tambien funciona.
        .gdo0 = ARDUINO_PIN_27, //GPIO_PIN(PD, 02),
        .gdo1 = GPIO_PIN(PA,25),//PIN MISO DE ARDUINO_DUE / ANTES: PIN_52
        .gdo2 = ARDUINO_PIN_28,//GPIO_PIN(PD, 03)/
    },
};
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* CC110X_PARAMS_H */
/** @} */
