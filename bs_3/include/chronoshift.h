/**
 * @file chronoshift.h
 * @author Derek Guo
 * @brief Shorthand macros for 
 * @version 2
 * @date 2023-04-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef CHRONOSHIFT_H
#define CHRONOSHIFT_H

/********** CONVENTIONS **********/
// T_SEC = 1 second (slow)
// T_DS  = 1 decisecond = 100 ms = 0.1 s
// T_CS  = 1 centisecond = 10 ms = 0.01 s (practical fast)
// T_MS  = 1 millisecond (the practical minimum for Arduino systems)

/********** DEFINES **********/
// "The way is always the same."

#ifdef _GLIBCXX_EXPERIMENTAL_CHRONO
  // If std library's date/time namespace is included
  #define T_SEC std::chrono::milliseconds{1000}
  #define T_DS  std::chrono::milliseconds{100}
  #define T_CS  std::chrono::milliseconds{10}
  #define T_MS  std::chrono::milliseconds{1}
#else
  // Standard Arduino / bare metal
  #define T_SEC 1000U
  #define T_DS  100U
  #define T_CS  10U
  #define T_MS  1U
#endif

#endif