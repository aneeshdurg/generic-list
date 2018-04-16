/**
 * @author: Aneesh Durg
 *
 * This file contains a generic type-safe linked list implementation generator
 * along with some linked list utitlites that don't depend on the type of the 
 * list. 
 *
 * To use, define the parameters described below, and include <stdlib.h> to
 * define malloc which is used by the constructor. stdlib only needs to be 
 * included in the file that defines the implementation (details below).
 *
 * PARAMETERS:
 *  TYPE
 *  TYPE_PTR
 *  _LIST_HEADER
 *  _LIST_IMPLEMENTATION
 *
 * To compile this library you must define the following macros:
 *   TYPE
 *   TYPE_PTR (optional)
 * 
 * TYPE controls the type of the link list implementation to be generated
 * TYPE_PTR should be a name that TYPE can be safely typedef'd to to prevent 
 *   invalid syntax in the case that TYPE contains a '*'
 * 
 * Note: you can create lists for multiple types by redefining TYPE and
 * re-including list.h
 *
 * The macros _LIST_HEADER and _LIST_IMPLEMENTATION are useful for including
 * this library in header files. Defining _LIST_HEADER before including will
 * only define everything except function bodies (creating only a prototype 
 * declaration). _LIST_IMPLEMENTATION will declare the function implementations
 * but will not declare struct definitions thereby allowing the program to
 * compile.
 *
 * If included in a source file with no accompanying header file, these
 * parameters can be safely omitted.
 *
 * e.g. Creating and using an int type'd list:
 *
 * ```
 * #include <stdio.h>
 * #include <stlib.h>
 *
 * #define TYPE int
 * #include "list.h"
 * #undef TYPE
 *
 * int main(){
 *  // Create an empty linked list
 *  list_sentinal_int list = new_list_int(NULL, 0, NULL);
 *  // Append 5 elements
 *  for(int i = 0; i < 5; i++)
 *    LIST_APPEND(&list, 0);
 *  // print each element
 *  LIST_FOR_EACH(&list, elem, {
 *    printf("Current element is %d\n", elem->entry);
 *  });
 *  // Cleanup list
 *  LIST_DESTROY(&list);
 *  return 0;
 * }
 * ```
 */

// Check that type is defined
#ifndef TYPE
#error "TYPE not defined! Cannot instantiate list implementation!"
#endif

/**
 * The following macros are not exposed outside this file
 * EXPAND is a hack needed to expand the arg before passing it to fn
 */
#define EXPAND(fn, arg) fn(arg)
#define EXPAND1(fn, arg, arg1) fn(arg, arg1)

// Internal macros for mangling type names
#define TYPEHACK_H(x) _##x
#define TYPEHACK(x) TYPEHACK_H(x)

/**
 * Special case for TYPE being a pointer.
 *
 * We need to rename type to avoid invalid syntax
 */
#ifdef TYPE_PTR
#define PTR_HACK(T, T1) typedef T _##T1;
EXPAND1(PTR_HACK, TYPE, TYPE_PTR)
#undef PTR_HACK

#undef TYPE
#define TYPE TYPEHACK(TYPE_PTR)
#endif

#ifndef _LIST_IMPLEMENTATION
/**
 * Defines parametrized list type
 *
 * @param T type parameter for list
 */
#define LIST_DEFN(T)                                                           \
  struct list_##T {                                                            \
    struct list_##T *next;                                                     \
    struct list_##T *prev;                                                     \
    T entry; /* Variable length struct */                                      \
  };
EXPAND(LIST_DEFN, TYPE)
#undef LIST_DEFN

/**
 * Define destructor type
 *
 * @param T type parameter for list
 */
#define LIST_DESTRUCTOR(T) typedef void (*list_destructor_##T)(T);
EXPAND(LIST_DESTRUCTOR, TYPE)
#undef LIST_DESTRUCTOR

/**
 * Define list metadata container struct
 *
 * @param T type parameter for list
 */
#define LIST_SENTINALS(T)                                                      \
  struct list_sentinal_##T {                                                   \
    struct list_##T *head;                                                     \
    struct list_##T *tail;                                                     \
    size_t length;                                                             \
    list_destructor_##T destructor;                                            \
  };
EXPAND(LIST_SENTINALS, TYPE)
#undef LIST_SENTINALS

/**
 * Generic list add
 *
 * For internal use only - see LIST_APPEND and LIST_PREPEND
 */
#define _LIST_ADD(list, elem, top, bottom, direction, reverse)                 \
  ({                                                                           \
    typeof(*((list)->head)) *new_entry = malloc(sizeof(*new_entry));           \
    new_entry->next = NULL;                                                    \
    new_entry->prev = NULL;                                                    \
    new_entry->entry = elem;                                                   \
    if (!(list)->bottom)                                                       \
      (list)->bottom = new_entry;                                              \
    if (!(list)->top)                                                          \
      (list)->top = new_entry;                                                 \
    else {                                                                     \
      (list)->top->direction = new_entry;                                      \
      new_entry->reverse = (list)->top;                                        \
      (list)->top = new_entry;                                                 \
    }                                                                          \
    ++(list)->length;                                                          \
  })

/**
 * Append an element to the tail of the list
 *
 * @return size_t length of new list
 *
 * @param list list_sentinal_type storing list metadata
 * @param elem element of the same type as the list to append (not of type list_type)
 * 
 * e.g.
 *
 * struct list_sentinal_int my_list = new_list_int(NULL, 0, NULL);
 * LIST_APPPEND(&my_list, 5);
 **
 */
#define LIST_APPEND(list, elem) _LIST_ADD(list, elem, tail, head, next, prev)

/**
 * Prepend an element to the head of the list
 *
 * @return size_t length of new list
 *
 * @param list pointer to list_sentinal_type storing list metadata
 * @param elem element of the same type as the list to prepend (not of type list_type)
 * 
 * e.g.
 *
 * struct list_sentinal_int my_list = new_list_int(NULL, 0, NULL);
 * LIST_PREPEND(&my_list, 5);
 *
 */
#define LIST_PREPEND(list, elem) _LIST_ADD(list, elem, head, tail, prev, next)

/**
 * Deletes an element from the list
 *
 * @param list pointer to list_sentinal_type storing list metadata
 * @param elem pointer to list_type of element to delete
 */
#define LIST_DEL(list, elem)                                                   \
  ({                                                                           \
    LIST_REMOVE(list, elem);                                                   \
    if ((list)->destructor)                                                    \
      (list)->destructor((elem)->entry);                                       \
    free(elem);                                                                \
  })

/**
 * Removes an element from a list without deleteing it
 *
 * @return size_t new length of list
 * 
 * @param list pointer to list_sentinal_type storing list metadata
 * @param elem pointer to list_type of element to remove
 */
#define LIST_REMOVE(list, elem)                                                \
  ({                                                                           \
    if ((elem)->prev)                                                          \
      (elem)->prev->next = (elem)->next;                                       \
    if ((elem)->next)                                                          \
      (elem)->next->prev = (elem)->prev;                                       \
    if ((elem) == (list)->head)                                                \
      (list)->head = (list)->head->next;                                       \
    if ((elem) == (list)->tail)                                                \
      (list)->tail = (list)->tail->prev;                                       \
    (list)->length--;                                                          \
  })

/**
 * Generic list pop
 *
 * For internal use only - see LIST_APPEND and LIST_PREPEND
 */
#define _LIST_POP(list, sentinal)                                              \
  ({                                                                           \
    typeof((list)->sentinal) __list_internal_temp = (list)->sentinal;          \
    LIST_REMOVE(list, __list_internal_temp);                                   \
    typeof(__list_internal_temp->entry) __list_internal_retval;                \
    __list_internal_retval = __list_internal_temp->entry;                      \
    free(__list_internal_temp);                                                \
    __list_internal_retval;                                                    \
  })

/**
 * List pop from front
 *
 * @return list_type element at head of list
 *
 * @param list pointer to list_sentinal_type storing list metadata
 */
#define LIST_POPF(list) _LIST_POP(list, head)

/**
 * List pop from back 
 *
 * @return list_type element at tail of list
 *
 * @param list pointer to list_sentinal_type storing list metadata
 */
#define LIST_POPB(list) _LIST_POP(list, tail)

/**
 * Generic list safe iterator
 *
 * For internal use only - see LIST_FOR_EACH_SAFE and LIST_FOR_EACH_REV_SAFE
 */
#define _LIST_FOR_EACH(list, var, sentinal, direction, callback)               \
  do {                                                                         \
    typeof((list)->sentinal) var = (list)->sentinal;                           \
    while (var) {                                                              \
      callback;                                                                \
      var = var->direction;                                                    \
    }                                                                          \
  } while (0)

/**
 * List iterator
 *
 * Iterates from head to tail
 * Does not allows deletion of elemnts during iteration
 *
 * @param list pointer to list_sentinal type
 * @param var name for variable to be used inside callback
 * @param callback code to be run on each iteration
 */
#define LIST_FOR_EACH(list, var, callback)                                     \
  _LIST_FOR_EACH(list, var, head, next, callback)

/**
 * List reverse iterator
 *
 * Iterates from tail to head 
 * Does not allows deletion of elemnts during iteration
 *
 * @param list pointer to list_sentinal type
 * @param var name for variable to be used inside callback
 * @param callback code to be run on each iteration
 */
#define LIST_FOR_EACH_REV(list, var, callback)                                 \
  _LIST_FOR_EACH(list, var, tail, prev, callback)

/**
 * Generic list safe iterator
 *
 * For internal use only - see LIST_FOR_EACH_SAFE and LIST_FOR_EACH_REV_SAFE
 */
#define _LIST_FOR_EACH_SAFE(list, var, temp, sentinal, direction, callback)    \
  do {                                                                         \
    typeof((list)->sentinal) var = (list)->sentinal;                           \
    typeof(var) temp = NULL;                                                   \
    if (var)                                                                   \
      temp = var->direction;                                                   \
    while (var) {                                                              \
      callback;                                                                \
      var = temp;                                                              \
      if (var)                                                                 \
        temp = var->direction;                                                 \
    }                                                                          \
  } while (0)

/**
 * List safe iterator
 *
 * Allows deletion of elemnts during iteration
 *
 * @param list pointer to list_sentinal type
 * @param var name for variable to be used inside callback
 * @param temp name for temporary variable - do not modify in callback
 * @param callback code to be run on each iteration
 */
#define LIST_FOR_EACH_SAFE(list, var, temp, callback)                          \
  _LIST_FOR_EACH_SAFE(list, var, temp, head, next, callback)

/**
 * List safe reverse iterator
 *
 * Allows deletion of elemnts during iteration
 *
 * @param list pointer to list_sentinal type
 * @param var name for variable to be used inside callback
 * @param temp name for temporary variable - do not modify in callback
 * @param callback code to be run on each iteration
 */
#define LIST_FOR_EACH_REV_SAFE(list, var, temp, callback)                      \
  _LIST_FOR_EACH_SAFE(list, var, temp, tail, prev, callback)

/**
 * List destructor
 *
 * @param list pointer to list_sentinal type to be destroyed
 */
#define LIST_DESTROY(list)                                                     \
  LIST_FOR_EACH_SAFE(list, __list_internal_var, __list_internal_temp,          \
                     { LIST_DEL(list, __list_internal_var); });
#endif

#ifndef _LIST_HEADER
/**
 * Constructor function
 *
 * @param initializer array to populate new list with
 * @param len length of initializer array
 *
 * if initializer is NULL, then the list will be empty
 */
#define LIST_CONSTRUCTOR(T)                                                    \
  struct list_sentinal_##T new_list_##T(T *initializer, size_t len,            \
                                        list_destructor_##T d) {               \
    struct list_sentinal_##T data;                                             \
    data.head = NULL;                                                          \
    data.tail = NULL;                                                          \
    data.length = 0;                                                           \
    data.destructor = d;                                                       \
    if (!initializer)                                                          \
      return data;                                                             \
    for (size_t i = 0; i < len; i++)                                           \
      LIST_APPEND(&data, initializer[i]);                                      \
    return data;                                                               \
  }
#else
// Only define header
#define LIST_CONSTRUCTOR(T)                                                    \
  struct list_sentinal_##T new_list_##T(T *initializer, size_t len,            \
                                        list_destructor_##T d);
#endif
EXPAND(LIST_CONSTRUCTOR, TYPE)
#undef LIST_CONSTRUCTOR
// Cleaning up expansion macros
#undef EXPAND
#undef EXPAND1

// Remove internal macros
#undef TYPEHACK_H
#undef TYPEHACK

