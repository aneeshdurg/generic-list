# Generic List

A generic implementation of a templetized linked list in C. See list.h for usage.

The advantages of this implementation lies in its type saftey. Only element of the same type can be inserted and removed from the list, and since each type uses a unique definition of it's underlying structs, any type errors can be caught at compile time. Of course, this can always be circumvented using casts and other tricks, but that would defeat the purpose of using this library.

## Quickstart guide

To get started, there is a simple program written in the first comment of `list.h` or you can see the contents of `list_test.c`.

To include this library in your project, use:

```c
#define TYPE int
#include "path/to/list.h"
#undef TYPE
```

You can re-include `list.h` after redefining `TYPE` if you'd like to define other types of lists.

If you'd like to include `list.h` in a header file, use:

```c
#define _LIST_HEADER
#define TYPE ...
#include "path/to/list.h"
#undef TYPE
#undef _LIST_HEADER
```

and in any corresponding `.c` files

```c
#define _LIST_IMPLEMENTATION
#define TYPE ...
#include "path/to/list.h"
#undef TYPE
#undef _LIST_IMPLEMENTATION
```

