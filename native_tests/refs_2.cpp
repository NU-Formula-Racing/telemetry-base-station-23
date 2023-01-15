#include <iostream>

/* Structs and Types */
// Each struct is given the following:
// - its value representation (the struct itself) as `*_t`
// - the number of members, in macro form as `_NUM`
// - an array with the sizes of each member, in order, as `_size`

// Each of these structs contains references to other data values

/* Struct 1 */
typedef struct COMPLEX {
  short* re;
  short* im;
} complex_t;

#define COMPLEX_NUM 2
const size_t complex_size[COMPLEX_NUM] = {
  sizeof(short),
  sizeof(short)
};

/* Struct 2 */
typedef struct POLAR {
  float* mod;
  float* arg;
  uint8_t* sign;
} polar_t;

#define POLAR_NUM 3
const size_t polar_size[POLAR_NUM] = {
  sizeof(float),
  sizeof(float),
  sizeof(uint8_t)
};

/* Composite struct */
// This contains multiple structs of pointers (the primitve "reference")
typedef struct STRUCT_OF_REFS {
  complex_t z;
  polar_t w;
} sor_t;

// Size of each pointer, which is platform-specific
#define PTR_LEN sizeof(size_t)

// Custom function to automate data copying for testing, NOT the cstdlib memcpy
void memcpy(uint8_t* a, uint8_t* b, size_t s);

int main(int argc, char* argv[]) {
  /* Initialize values */
  // These will be referenced for the structs
  short re = 1, im = -1;
  float mod = 3.14, arg = 1;
  uint8_t sign = 1;

  /* Initialize struct */
  // Get references and consolidate
  sor_t vals = {
    .z = {
      .re = &re,
      .im = &im
    },
    .w = { 
      .mod = &mod, 
      .arg = &arg, 
      .sign = &sign
    }
  };

  // Pointer aliases
  // vals_ptr   - Access each pointer within each member struct indirectly
  // vals_bytes - Access the byte data for the values from the pointers within the members
  // uint8_t* must be used for the arithmetic to work, cannot use void or ** directly
  uint8_t *vals_ptr, *vals_bytes;

  /* Serialization buffer */
  // Simulates the process of serializing into binary given references
  uint8_t buf[2 * sizeof(sor_t)];
  uint8_t* buf_ptr = buf; // Alias for buffer
  size_t buf_len = 0; // Length of buffer, TBD

  // Used for iteration
  size_t i, j;

  /* Serialize */
  // Based off of the last part of Test 2 in `refs.cpp`

  vals_ptr = (uint8_t*) &vals.z;
  for (i = 0; i < COMPLEX_NUM; ++i) {
    vals_bytes = *((uint8_t**) vals_ptr);
    for (j = 0; j < complex_size[i]; ++j) {
      *(buf_ptr++) = *(vals_bytes++);
      ++buf_len;
    }
    vals_ptr += PTR_LEN;
  }

  vals_ptr = (uint8_t*) &vals.w;
  for (i = 0; i < POLAR_NUM; ++i) {
    vals_bytes = *((uint8_t**) vals_ptr);
    for (j = 0; j < polar_size[i]; ++j) {
      *(buf_ptr++) = *(vals_bytes++);
      ++buf_len;
    }
    vals_ptr += PTR_LEN;
  }

  /* Print buffer */
  std::cout << "Data:";
  for (i = 0; i < buf_len; ++i) {
    std::cout << " " << unsigned(buf[i]);
  }
  std::cout << std::endl;

  /* Check output */
  // Check that the values in the overall buffer match the representation and order
  std::cout << "Manual byte order" << std::endl;
  uint8_t ref_buf[8];

  // complex_t->short z.re
  memcpy((uint8_t*) &re, ref_buf, complex_size[0]);
  std::cout << "re:";
  for (i = 0; i < complex_size[0]; ++i) {
    std::cout << " " << unsigned(ref_buf[i]);
  }
  std::cout << std::endl;

  // complex_t->short z.im
  memcpy((uint8_t*) &im, ref_buf, complex_size[1]);
  std::cout << "im:";
  for (i = 0; i < complex_size[1]; ++i) {
    std::cout << " " << unsigned(ref_buf[i]);
  }
  std::cout << std::endl;

  // polar_t->float w.mod
  memcpy((uint8_t*) &mod, ref_buf, polar_size[0]);
  std::cout << "mod:";
  for (i = 0; i < polar_size[0]; ++i) {
    std::cout << " " << unsigned(ref_buf[i]);
  }
  std::cout << std::endl;

  // polar_t->float w.arg
  memcpy((uint8_t*) &arg, ref_buf, polar_size[1]);
  std::cout << "arg:";
  for (i = 0; i < polar_size[1]; ++i) {
    std::cout << " " << unsigned(ref_buf[i]);
  }
  std::cout << std::endl;

  // polar_t->uint8_t w.sign
  memcpy((uint8_t*) &sign, ref_buf, polar_size[2]);
  std::cout << "sign:";
  for (i = 0; i < polar_size[2]; ++i) {
    std::cout << " " << unsigned(ref_buf[i]);
  }
  std::cout << std::endl;

  return 0;
}

void memcpy(uint8_t* a, uint8_t* b, size_t s) {
  for (size_t i = 0; i < s; ++i) {
    *(b++) = *(a++);
  }
}