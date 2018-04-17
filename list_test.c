#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define TEST(fn)\
  if(!fn()) {\
    puts(#fn" passed!");\
  } else {\
    puts(#fn" failed.");\
  }

#define TYPE int
#include "list.h"
#undef TYPE

#define TYPE char*
#define TYPE_PTR str 
#include "list.h"
#undef TYPE_PTR
#undef TYPE

#ifdef ERROR_TEST
//won't compile
#include "list.h" 
#endif

int definitions_test(){
  // This just needs to compile
  struct list_sentinal_int l;
  (void)l;
  struct list_int i;
  (void)i;
  (void)new_list_int;

  struct list_sentinal__str ls;
  (void)ls;
  struct list__str is;
  (void)is;
  (void)new_list__str;
  return 0;
}

int int_test(){
  int list[] = {1, 2, 3, 4};
  struct list_sentinal_int my_list = new_list_int(list, 4, NULL);
  assert(my_list.head->entry == 1);
  assert(my_list.tail->entry == 4);
  assert(LIST_APPEND(&my_list, 5) == 5);
  assert(LIST_PREPEND(&my_list, 0) == 6);
  struct list_int *p = my_list.head;
  int counter = 0;
  while(p){
    assert(p->entry == counter);
    counter++;
    p = p->next;
  }
  assert(LIST_POPF(&my_list) == 0);
  assert(my_list.length == 5);
  assert(LIST_POPF(&my_list) == 1);
  assert(my_list.length == 4);
  assert(LIST_POPF(&my_list) == 2);
  assert(my_list.length == 3);

  counter = 3;
  LIST_FOR_EACH(&my_list, elem, {
      elem->entry++;
      assert(elem->entry == counter+1);
      counter++;
  });

  LIST_FOR_EACH_SAFE(&my_list, elem, temp, {
      LIST_DEL(&my_list, elem);
  });

  assert(!my_list.length);
  LIST_DESTROY(&my_list);
  return 0;
}

int str_test(){
  void str_destroy(char *str){ free(str); }
  char *a = strdup("1"); 
  char *b = strdup("2"); 
  char *c = strdup("3"); 
  
  char *clist[] = {a, b, c};
  struct list_sentinal__str my_c_list = new_list__str(clist, 3, str_destroy);
  LIST_APPEND(&my_c_list, strdup("4"));
  LIST_PREPEND(&my_c_list, strdup("0"));

  char *expected[] = {"0", "1", "2", "3", "4"};
  struct list__str *cp = my_c_list.head;
  int counter = 0;
  while(cp){
    assert(!strcmp(cp->entry, expected[counter]));
    counter++;
    cp = cp->next;
  }
  LIST_DESTROY(&my_c_list);
}

int main(){
  puts("Make sure to run all tests with valgrind!");
  TEST(definitions_test);
  TEST(int_test);
  TEST(str_test);
  return 0;
}
