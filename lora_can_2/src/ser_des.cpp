/**
 * @file ser_des.cpp
 * @author Derek Guo
 * @brief Serialization and Deserialization functions
 * @version 1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/********** INCLUDES **********/
#include "ser_des.h"

/********** FUNCTION DEFINITIONS **********/

/**
 * @brief Short (16-bit datatype) TO Buffer (of chars/bytes with arbitrary starting index) 
 * @param sh  pointer to any short, cast to char* for consistency; caller must guarantee original type
 * @param buf pointer to any char* (byte) buffer; starting index set by caller, must guarantee in-bounds
 */
void stob(char* sh, char* buf) {
  buf[0] = *sh;
  buf[1] = *(sh + 1);
}