#include <iostream>
#include <cstdlib>

/* Test 1 */
// Ser/des test functions
// Copy value in reference to a buffer, then copy to pointer
void ser(int& x, uint8_t* d);
void des(int* y, uint8_t* d);

/* Test 2 */
// Struct containing pointers to various values
typedef struct STRUCT_OF_PTRS {
  uint16_t* u16;
  float* f32;
  double** vec_f64;
} stars_t;

// Function to access pointers in struct
void starclan(stars_t* death);

// Pointer size, assuming all object pointers are equally sized
#define PTR_SIZE sizeof(void*)

int main() {
  /* Test 1 */
  // Reference
  int x = 0x2048; // 8264
  int& x_val_ref = x;

  // Buffer
  uint8_t d[4];

  // Second value;
  int y;

  // Mock data transfer
  ser(x_val_ref, d);
  des(&y, d);

  // Print data
  std::cout << x << std::endl;
  std::cout << y << std::endl;

  /* Test 2 */
  // Test data, some stack, some other
  // Dynamically allocated primitive
  uint16_t* val1 = new uint16_t;
  *val1 = 9;

  // Stack allocated primitive
  float val2 = 13.37;

  // Dynamically allocated array
  double* val3 = (double*) malloc(69 * sizeof(double));
  val3[2] = 3.14;

  // Put everything into struct
  stars_t many_ptrs = {
    .u16 = val1,
    .f32 = &val2,
    .vec_f64 = &val3,
  };

  // Check access validity
  starclan(&many_ptrs);

  // Release data
  delete val1;
  free(val3);

  return 0;
}

/* Test 1 cont */
void ser(int& x, uint8_t* d) {
  // Pointer abuse to access bytes directly
  uint8_t* x_ptr = (uint8_t*) &x;

  // Borrowing buffer
  uint8_t* d_ptr = d;

  // Serialization 
  for (size_t i = 0; i < 4; ++i) {
    *(d_ptr++) = *(x_ptr++);
  }
}

void des(int* y, uint8_t* d) {
  // Pointer abuse to access bytes directly
  uint8_t* y_ptr = (uint8_t*) y;

  // Borrowing buffer
  uint8_t* d_ptr = d;

  // Serialization 
  for (size_t i = 0; i < 4; ++i) {
    *(y_ptr++) = *(d_ptr++);
  }
}

/* Test 2 cont */
void starclan(stars_t* death) {
  // Pointer abuse to access pointers independent of name
  // This is to improve automation of access for structs

  // This actually is supposed to be void** (ptr to struct with pointers to data)
  // but must be treated as a byte pointer to allow for the arithmetic to work
  uint8_t* ptr = (uint8_t*) death;

  // Retrieve target values
  uint16_t cloudkit = **((uint16_t**) ptr); // Must double dereference for arithmetic to work
  ptr += PTR_SIZE;
  float sandstorm =  **((float**) ptr);
  ptr += PTR_SIZE;
  double firestar = *(**((double***) ptr) + 2); // Cannot index as array (gives segfault), must use pointer arithmetic

  // Display targeted values
  std::cout << "u16: " << cloudkit << "\nf32: " << sandstorm << "\nf64: " << firestar << " from dyn array" << std::endl;

  ptr = (uint8_t*) death;
  uint8_t* bytes = *((uint8_t**) ptr);

  std::cout << "Byte rep: " << unsigned(*bytes) << ", " << unsigned(*(bytes + 1)) << std::endl;
}