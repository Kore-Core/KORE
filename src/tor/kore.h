/* Copyright (c) 2014, KoreCoin 2.0 Developers */
/* See LICENSE for licensing information */

/**
 * \file kore.h
 * \brief Headers for kore.cpp
 **/

#ifndef TOR_KORECOIN_H
#define TOR_KORECOIN_H

#ifdef __cplusplus
extern "C" {
#endif

    char const* kore_tor_data_directory();

    char const* kore_service_directory();

    int check_interrupted();

    void set_initialized();

    void wait_initialized();

    extern int coin_port_num;

#ifdef __cplusplus
}
#endif

#endif

