/**
 * @file ser_des.h
 * @author Derek Guo
 * @brief SerDes and EnDec functions
 * @version 1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SER_DES_H
#define SER_DES_H

/********** INCLUDES **********/
#include <Arduino.h>

/********** FUNCTION PROTOTYPES **********/

/*** Minimal SerDes ***/
// Due to the inability of standard library converters between
// short (integral) / half (floating) to function properly,
// as itoa is not implemented on all platforms and custom
// serialization causes atoi to fail, custom ser/des
// procedures are needed.

// These are minimal-processing methods, directly taking the bit data
// from each of the 2 bytes given and placing them in the respective
// location on the buffer, to be indexed later. For this reason, these
// are very much unsafe and pointers should be checked to make sure
// they encompass sufficient memory.

/* Serializer function */
void stob(char* sh, char* buf);

/* Deserializer function */
void btos(char* sh, char* buf);

/*** Float EnDec functions ***/
// While the CAN library does receive signals in their short equivalents,
// it does not store them in their raw form, instead converting them
// automatically to their designated type.

// One such case is of floating point signals, which are sent as uint16_t
// (unsigned short ints), but are meant to be read as floats (of any precision).
// C does not have 16-bit float support, meaning conversion from short
// to float will double its precision, making them incompatible for direct
// serialization.

// Thus, to format the floats for transfor over LoRa, they must be re-encoded
// in their integral form before transmission. For our purposes, float
// sensors of a relatively low precision (usually up to 0.1) and a limited
// range. However, these parameters are variable, and for any higher precision
// floats the amount of precision loss grows higher. These risks must be 
// kept in mind before using these relatively simple converters.

/* Encoding function */
void ftos(float* fl, uint16_t* sh, float scale, float bias);

/* Decoding function */
void stof(float* fl, uint16_t* sh, float scale, float bias);

#endif