# cpp-slice
C++11 slice class, inspired by the D language

## Usage
Just include `slice.h` and you're good to go!
```
#include <slice.h>

Slice<int> int_slice{ 1, 2, 3};
```

You can inhibit STL interoperability by defining `NO_STL` prior to including the header:
```
#define NO_STL
#include <slice.h>

Slice<const char> string_slice(std::string("hello")); // compile error!
```
