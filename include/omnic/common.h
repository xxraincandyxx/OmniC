// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_COMMON_H
#define OMNIC_COMMON_H

#include <stdbool.h>  // For bool
#include <stddef.h>   // For size_t

/* -------------------------------------------------------------------------- */

/**
 * @file common.h
 * @brief Common definitions, error codes, and types for the OmniC library.
 */

/**
 * @brief Universal error codes for OmniC operations.
 */
typedef enum {
  OC_SUCCESS = 0,              /**< Operation was successful. */
  OC_ERROR_ALLOC = -1,         /**< Failed to allocate memory. */
  OC_ERROR_INVALID_ARG = -2,   /**< An invalid argument was provided. */
  OC_ERROR_OUT_OF_BOUNDS = -3, /**< Index or access was out of bounds. */
} oc_error_code_t;

#endif  // OMNIC_COMMON_H
