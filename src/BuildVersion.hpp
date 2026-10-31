#pragma once

// BUILD_VERSION values, as configure.py passes them.
#define BUILD_VERSION_008p 0x00081
#define BUILD_VERSION_013 0x00130
#define BUILD_VERSION_013a 0x00131
#define BUILD_VERSION_100 0x01000
#define BUILD_VERSION_101 0x01010
#define BUILD_VERSION_102 0x01020
#define BUILD_VERSION_102b 0x01022
#define BUILD_VERSION_102c 0x01023
#define BUILD_VERSION_102d 0x01024
#define BUILD_VERSION_102f 0x01026
#define BUILD_VERSION_102g 0x01027
#define BUILD_VERSION_102h 0x01028

#ifndef BUILD_VERSION
#define BUILD_VERSION BUILD_VERSION_102h
#endif
#ifndef GAME_VERSION
#define GAME_VERSION 0x0102
#endif

// Release order. The trials' BUILD_VERSION values sort below every 1.0x build, but 0.13 and 0.13a came out
// alongside 1.02d-1.02f and share their code, so changes between builds are tested with BUILT_SINCE and
// BUILT_BEFORE rather than by comparing BUILD_VERSION. TRIALBUILD is only for what the trials alone have.
#define BUILD_ORDER_008p 0 // 2002-06-22
#define BUILD_ORDER_100 1  // 2002-08-10
#define BUILD_ORDER_101 2  // 2002-08-14
#define BUILD_ORDER_102 3  // 2002-08-21, also 1.02a
#define BUILD_ORDER_102b 4 // 2002-08-21
#define BUILD_ORDER_102c 5 // 2002-08-22
#define BUILD_ORDER_013 6  // 2002-08-25
#define BUILD_ORDER_102d 7 // 2002-08-25, also 1.02e
// 1.02f and 0.13a share a date. 0.13a comes second: it has Chain::Cut's debug prints, which 1.02f lacks.
#define BUILD_ORDER_102f 8  // 2002-08-31
#define BUILD_ORDER_013a 9  // 2002-08-31
#define BUILD_ORDER_102g 10 // 2002-12-01
#define BUILD_ORDER_102h 11 // 2002-12-01

#if BUILD_VERSION == BUILD_VERSION_008p
#define BUILD_ORDER BUILD_ORDER_008p
#elif BUILD_VERSION == BUILD_VERSION_013
#define BUILD_ORDER BUILD_ORDER_013
#elif BUILD_VERSION == BUILD_VERSION_013a
#define BUILD_ORDER BUILD_ORDER_013a
#elif BUILD_VERSION == BUILD_VERSION_100
#define BUILD_ORDER BUILD_ORDER_100
#elif BUILD_VERSION == BUILD_VERSION_101
#define BUILD_ORDER BUILD_ORDER_101
#elif BUILD_VERSION == BUILD_VERSION_102
#define BUILD_ORDER BUILD_ORDER_102
#elif BUILD_VERSION == BUILD_VERSION_102b
#define BUILD_ORDER BUILD_ORDER_102b
#elif BUILD_VERSION == BUILD_VERSION_102c
#define BUILD_ORDER BUILD_ORDER_102c
#elif BUILD_VERSION == BUILD_VERSION_102d
#define BUILD_ORDER BUILD_ORDER_102d
#elif BUILD_VERSION == BUILD_VERSION_102f
#define BUILD_ORDER BUILD_ORDER_102f
#elif BUILD_VERSION == BUILD_VERSION_102g
#define BUILD_ORDER BUILD_ORDER_102g
#elif BUILD_VERSION == BUILD_VERSION_102h
#define BUILD_ORDER BUILD_ORDER_102h
#else
#error Unknown BUILD_VERSION
#endif

// Version gates describe release behavior and data differences, not matching-only
// choices of include order, helper placement, or COMDAT survivors.
#define BUILT_BEFORE(version) (BUILD_ORDER < BUILD_ORDER_##version)
#define BUILT_SINCE(version) (BUILD_ORDER >= BUILD_ORDER_##version)
