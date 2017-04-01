# cpp-slice
Set of containers that implement various useful modern forms of arrays, inspired by the D language.

There are 3 types provided:

### `Array<T[, N]>`

Array container that owns its elements. Similar to `std::vector`, but much leaner, and with more useful syntax and operations.

`Array` may optionally take a second `size_t` argument; `Array<T, N>`, which reserves `N` elements inside the `Array` structure. These local elements will be used for short arrays, avoiding allocations. This is particularly useful when storing working data on the stack, which may reserve some stack space for short arrays, but overflow (allocate) to the heap of the workload becomes large.

### `SharedArray<T>`

Ref-counted version of `Array<T>` for shared data, making copies very efficient.

`SharedArray` can be move-constructed from `Array` to allow efficient promotion of a resource to a shared resource.

`SharedArray` is 'structurally immutable', that is, the length of the array may not change after construction. `push`/`pop` methods are disabled for `SharedArray`'s.

It is a common pattern to use `Array` as a working unit to build some data, and then transfer it to a `SharedArray` when it is to be distribution throughout an application.

### `Slice<T>`

Slice is an un-owned array, similar to `std::array_view` as introduced to C++17.

Slices are a reference to someone elses memory, which makes them efficient for passing as parameters to functions (like passing pointers/references), but they should generally not be retained, as the data is un-owned, and might be lost at some later time.

## String specialisation

All types are specialised for character element types; `char`, `wchar_t`, `char16_t`, `char32_t`, introducing an additional set of string methods for working with string data.

Functionality becomes similar to `std::string`/`std::string_view`, except with many extra useful methods.

Strings are not required to be zero-terminated, which makes working on sub-strings, or tokenised strings extremely efficient. This is a major advantage in terms of speed and convenience compared to standard C or STL string apis. No additional allocation or modification of strings for zero-termination is required to perform any string manipulation. Parsing complex string data is typically possible with a single allocation and read-only access.

Strings have Unicode awareness, and support Unicode format transcoding when assigning from one Unicode string type to another. Comprehensive encode and decode functionality also exists.

Convenient typedef's are defined for string types, with the naming pattern 'String', 'WString', 'DString' to indicate utf-8. utf-16, utf-32 strings respectively.

Light-weight string references (not mutable since memory is borrowed):
 - `String` -> `Slice<const char>`
 - `WString` -> `Slice<const char16_t>`
 - `DString` -> `Slice<const char32_t>`

Mutable strings:
 - `MutableString<N>` -> `Array<char, N>`
 - `MutableWString<N>` -> `Array<char16_t, N>`
 - `MutableDString<N>` -> `Array<char32_t, N>`


Mutable strings with small local string buffers to avoid allocations:

 - `MutableString64` -> `Array<char, 64 - sizeof(Slice<char>)>`
 - `MutableString128` -> `Array<char, 128 - sizeof(Slice<char>)>`
 - `MutableString256` -> `Array<char, 256 - sizeof(Slice<char>)>`

Note: sizes refer to _total size of the struct_; helpful to maintain alignment, precisely fit cache lines, etc. String buffer is slightly smaller depending on architecture.

Shared strings (not mutable):
 - `SharedString` -> `SharedArray<const char>`
 - `SharedWString` -> `SharedArray<const char16_t>`
 - `SharedDString` -> `SharedArray<const char32_t>`


## STL interoperability

These types attempt to conform with standard STL api naming conventions and patterns where it doesn't conflict with the design goals.

All types can efficiently construct from `std::vector`, `std::string`, etc, types.
Some users may just like to use `Slice<T>` in an application that uses `std::string` for storage to gain access to efficient string manipulation.

In MSVC, use of STL introduces a set of linkage and C-Runtime compatibility problems which many users are trying to avoid. Users of this library as an alternative to STL may like to inhibit this library's support for STL to avoid those headers being included and types being defined.

Users may simply `#define NO_STL` prior to including any of these headers to inhibit any STL interactivity.

## Custom allocation

STL's pattern of mangling types with an allocator as a template argument is generally considered to be a failure, and a major point of complexity for those types.

To customise the allocator used by `Array`, and `SharedArray`, users should define `SLICE_ALLOC` and `SLICE_FREE` prior to including the headers to supply allocator functions for the library to call.

The default is defined to use `malloc` and `free`:
```C++
#if !defined(SLICE_ALLOC)
# define SLICE_ALLOC(bytes) malloc(bytes)
#endif
#if !defined(SLICE_FREE)
# define SLICE_FREE(ptr) free(ptr)
#endif
```

## Usage examples

Slice is a reference type that can refer to sub-ranges of data.

```C++
#include <slice.h>

int c_array[] = { 1, 2, 3, 4 };
Slice<int> s1 = c_array;         // s1 == [ 1, 2, 3, 4 ]

Slice<int> s2 = s1.slice(2, 4);  // s2 == [ 3, 4 ] <- capture a sub-range of the data

Slice<const int> cs = s2;        // promotion to const slices works as expected

assert (cs == s2);               // slice comparison compares the references are the same
                                 // NOT element-wise comparison (use Slice::eq())

std::vector<int> vec = { 1, 2, 3 };
Slice<int> sv = vec;             // sv == [ 1, 2, 3 ], points at the memory owned by `vec`

String str = "abc";              // reference to a c-string (note: String == Slice<const char>)
assert (str.length == 3);        // String's are not null-terminated, length excludes the null

// tokenising strings is efficient and very convenient, no allocations are required
String[10] tokens;
size_t numTokens = String("tokenise this string").tokenise(tokens, " \t");
assert (numTokens == 3);
assert (tokens[0].eq("tokenise"));
assert (tokens[1].eq("this"));
assert (tokens[2].eq("string"));
```

Arrays work like `std::vector`, except may store some local data for short arrays:
```C++
#include <slice.h>

Array<int> arr = { 1, 2, 3 };                     // arr == [ 1, 2, 3 ]
arr.push_back(4);                                 // arr == [ 1, 2, 3, 4 ]

Array<int, 4> sa = arr;   // sa = [ 1, 2, 3 ]       <- elemtns are on the stack
sa.push_back(4);          // sa = [ 1, 2, 3, 4 ]    <- elemtns are still on the stack
sa.push_back(5);          // sa = [ 1, 2, 3, 4, 5 ] <- elemtns have overflowed to an allocation

// build arrays by concatination of other arrays, or single elements
Array<int> a2(Concat, arr, 10, arr.slice(0, 2));  // a2 = [ 1, 2, 3, 4, 10, 1, 2 ]

// conveniently build strings, unicode format automatically transcodes to target
MutableString<> str(Concat, "Hello ", U"cruel ", L"world", '!');
```

And SharedString's are useful for sharing and keeping for long times:
```C++
SharedArray<int> sa = { 1, 2, 3 }; // allocation with ref-count == 1
SharedArray<int> sa2 = sa;         // ref-count == 2
sa = nullptr;                      // sa2 ref-count == 1
sa2 = nullptr;                     // array is unreferenced; de-allocated

Array<int> f()
{
  return Array<int>{ 1, 2, 3 };
}

SharedArray<int> sa = f(); // functions build and return data; automatically promote to 'shared'
```
