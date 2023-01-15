This directory contains a series of small local-side test programs
to validate certain platform-independent concepts before scaling it to production
(those concepts which I, the eponymous author, did not learn included)

- `struct_passing` tests a skeleton framework of struct storage, its automation of ser/des, and dynamic value updates.

- `refs` contains tests involving the manipulation of references and pointers, especially the unsafe operations:
  - Test 1 pertains to reference-pointer conversions.
  - Test 2 explores structs of pointers, their bit representations, and their manipulation thereof.

- `refs_2` expands upon Test 2 of the above, more closely mimicking the conditions used in the overall program.