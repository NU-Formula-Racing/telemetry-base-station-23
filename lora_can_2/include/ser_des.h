/**
 * @file ser_des.h
 * @author Derek Guo
 * @brief Serialization and Deserialization functions
 * @version 1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef SER_DES_H
#define SER_DES_H

/********** FUNCTION PROTOTYPES **********/

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

#endif