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

/**
 * @brief Buffer (of chars/bytes with arbitrary starting index) TO Short (16-bit datatype)
 * @param sh  pointer to any short, cast to char* for consistency; caller must guarantee original type
 * @param buf pointer to any char* (byte) buffer; starting index set by caller, must guarantee in-bounds
 */
void btos(char* sh, char* buf) {
	*sh = buf[0];
	*(sh + 1) = buf[1];
}

/**
 * @brief Float (32-bit precision signed) TO Short (16-bit datatype) 
 * @param fl    pointer to float
 * @param sh    pointer to short
 * @param scale scaling parameter, which should be 1 / float (CAN signal factor); multiplying by this should allow a CAN signal float to be int-cast without major data loss
 * @param bias  bias parameter; subtract this from the float for scaling to work properly
 */
void ftos(float* fl, uint16_t* sh, float scale, float bias) {
	*sh = uint16_t(int((*fl - bias) * scale + 0.5));
}

/**
 * @brief Short (16-bit datatype) TO Float (32-bit precision signed)
 * @param fl    pointer to float
 * @param sh    pointer to short
 * @param scale scaling parameter, which should be 1 / float (CAN signal factor); dividing by this should turn a raw CAN signal into its dimensional equivalent, excluding bias
 * @param bias  bias parameter; subtract this from the float for scaling to work properly
 */
void stof(float* fl, uint16_t* sh, float scale, float bias) {
	*fl = static_cast<float>(*sh) / scale + bias ;
}