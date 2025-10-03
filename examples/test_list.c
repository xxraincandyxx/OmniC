// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <stdio.h>
#include <string.h>

#include <omnic/list.h>

// 1. Define the structure that will be stored in the list.
//    It MUST contain an `oc_list_node_t` member.
typedef struct {
  int id;
  const char* name;
  oc_list_node_t link;  // The member name can be anything
} user_data_t;

// A "dumper" function for printing a user_data_t element.
void print_user_data(const void* entry, FILE* stream) {
  const user_data_t* data = (const user_data_t*)entry;
  fprintf(stream, "user_data_t { id: %d, name: \"%s\" }", data->id, data->name);
}

int main(void) {
  printf("--- OmniC Intrusive List Example ---\n\n");

  // 2. Declare and initialize the list head.
  oc_list_t my_list;
  oc_list_init(&my_list);

  printf("Is list empty? %s\n", oc_list_empty(&my_list) ? "Yes" : "No");
  printf("Initial size: %zu\n\n", oc_list_size(&my_list));

  // 3. Add elements using emplace macros.
  //    Note the use of designated initializers (C99+).
  printf("Adding elements...\n");
  oc_list_emplace_back(&my_list, user_data_t, link, .id = 101, .name = "Alice");
  oc_list_emplace_back(&my_list, user_data_t, link, .id = 102, .name = "Bob");
  oc_list_emplace_front(&my_list, user_data_t, link, .id = 100, .name = "Eve");

  printf("List after additions:\n");
  oc_list_dump(&my_list, user_data_t, link, print_user_data, stdout);
  printf("Current size: %zu\n\n", oc_list_size(&my_list));

  // 4. Find an element.
  //    This is done by manually iterating with the for_each macro.
  printf("Finding user with ID 101...\n");
  user_data_t* found_item = NULL;
  user_data_t* iter;  // The cursor for the loop
  oc_list_for_each(iter, &my_list, user_data_t, link) {
    if (iter->id == 101) {
      found_item = iter;
      break;
    }
  }

  if (found_item) {
    printf("Found: ");
    print_user_data(found_item, stdout);
    printf("\n\n");
  } else {
    printf("Item not found.\n\n");
  }

  // 5. Erase the found element.
  if (found_item) {
    printf("Erasing user with ID 101...\n");
    oc_list_erase(&my_list, found_item, link);
    printf("List after erase:\n");
    oc_list_dump(&my_list, user_data_t, link, print_user_data, stdout);
    printf("Current size: %zu\n\n", oc_list_size(&my_list));
  }

  // 6. Demonstrate safe iteration while erasing.
  printf("Erasing all users with an ID >= 102...\n");
  user_data_t *safe_iter, *temp;
  oc_list_emplace_back(&my_list, user_data_t, link, .id = 103,
                       .name = "Charlie");
  oc_list_for_each_safe(safe_iter, temp, &my_list, user_data_t, link) {
    if (safe_iter->id >= 102) {
      printf("  - Erasing user %s\n", safe_iter->name);
      oc_list_erase(&my_list, safe_iter, link);
    }
  }
  printf("List after safe erase loop:\n");
  oc_list_dump(&my_list, user_data_t, link, print_user_data, stdout);
  printf("Current size: %zu\n\n", oc_list_size(&my_list));

  // 7. Destroy the list, freeing all remaining elements.
  printf("Destroying list...\n");
  oc_list_destroy(&my_list, user_data_t, link);
  printf("Is list empty? %s\n", oc_list_empty(&my_list) ? "Yes" : "No");
  printf("Final size: %zu\n\n", oc_list_size(&my_list));

  return 0;
}
