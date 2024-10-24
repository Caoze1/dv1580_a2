// Viktor Fransson DVAMI22h

#include "linked_list.h"

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Initializes a linked list by setting up a memory pool and the head node.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @param size The size of the memory pool to initialize.
 *
 * Behavior:
 * - Initializes a memory pool of the given size using `mem_init`.
 * - Sets the head pointer of the linked list to `NULL`, indicating an empty list.
 */
void list_init(Node** head, size_t size){
  mem_init(size);
  *head = NULL;
};


/**
 * Inserts a new node with the specified data at the end of the linked list.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @param data The integer data to store in the new node.
 *
 * Behavior:
 * - Allocates memory for a new node using `mem_alloc`.
 * - If the list is empty, the new node becomes the head.
 * - Otherwise, the new node is added to the end of the list.
 */
void list_insert(Node** head, uint16_t data){
  pthread_mutex_lock(&list_mutex);
  Node* node = (Node*) mem_alloc(sizeof(Node));
 
  node->data = data;
  node->next = NULL;

  if (*head == NULL){
    *head = node;
  }
  else{
    Node* current = *head;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = node;
  }
  pthread_mutex_unlock(&list_mutex);
};


/**
 * Inserts a new node with the specified data after the given node.
 *
 * @param prev_node The node after which the new node will be inserted.
 * @param data The integer data to store in the new node.
 *
 * Behavior:
 * - Allocates memory for a new node.
 * - Inserts the new node immediately after the given `prev_node`.
 * - If `prev_node` is `NULL`, the function does nothing.
 */
void list_insert_after(Node* prev_node, uint16_t data){
  pthread_mutex_lock(&list_mutex);
  
  Node* node = (Node*) mem_alloc(sizeof(Node));
  node->data = data;
  
  node->next = prev_node->next;
  prev_node->next = node;
  pthread_mutex_unlock(&list_mutex);
};


/**
 * Inserts a new node with the specified data before the given node.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @param next_node The node before which the new node will be inserted.
 * @param data The integer data to store in the new node.
 *
 * Behavior:
 * - Allocates memory for a new node.
 * - If the list is empty or the next_node is the head, the new node becomes the new head.
 * - Otherwise, the new node is inserted before the given `next_node`.
 */
void list_insert_before(Node** head, Node* next_node, uint16_t data){
  pthread_mutex_lock(&list_mutex);
  
  Node* node = (Node*) mem_alloc(sizeof(Node));
  node->data = data;

  node->next = next_node;

  if (*head == next_node){ // If next_node is the first node
    *head = node;
  }
  else {
    Node* current = *head;
    while (current->next != next_node) { // Go through the list from the front
      current = current->next;
    }
    current->next = node; // Then insert the node where the next_node used to be
  }
  pthread_mutex_unlock(&list_mutex);
};


/**
 * Deletes the first node with the specified data from the linked list.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @param data The data of the node to delete.
 *
 * Behavior:
 * - Searches for the node with the specified data and removes it from the list.
 * - Frees the memory associated with the node.
 * - If the node is not found, the function does nothing.
 */
void list_delete(Node** head, uint16_t data){
  pthread_mutex_lock(&list_mutex);
  
  Node* current = *head;
  Node* prev_node = NULL;

  while (current != NULL){
    if (current->data == data){
      if (prev_node != NULL){
        prev_node->next = current->next;
      }
      if (current == *head){
        *head = current->next;
      }
      mem_free(current);
      pthread_mutex_unlock(&list_mutex);
      return;
    }
    prev_node = current;
    current = current->next;
  }
  pthread_mutex_unlock(&list_mutex);
};


/**
 * Searches for a node with the specified data in the linked list.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @param data The data to search for in the list.
 * @return Pointer to the node containing the data, or `NULL` if not found.
 *
 * Behavior:
 * - Traverses the linked list to find the first node that contains the given data.
 * - If the node is found, it is returned; otherwise, the function returns `0`.
 */
Node* list_search(Node** head, uint16_t data){
  pthread_mutex_lock(&list_mutex);
  
  Node* current = *head;

  while (current != NULL){
    if (current->data == data){
      pthread_mutex_unlock(&list_mutex);
      return current;
    }
    current = current->next;
  }
  printf("data not in list");
  pthread_mutex_unlock(&list_mutex);
  return 0;
};


/**
 * Displays the data of nodes between the specified start and end nodes, inclusive.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @param start_node The node from which to start displaying.
 * @param end_node The node at which to stop displaying.
 *
 * Behavior:
 * - Prints the data of each node starting from `start_node` to `end_node`.
 * - If `start_node` is `NULL`, the function starts from the head of the list.
 */
void list_display_range(Node** head, Node* start_node, Node* end_node){
  pthread_mutex_lock(&list_mutex);
  
  if (start_node == NULL){
    start_node = *head;
  }

  printf("[");
  while (start_node != end_node){
    printf("%d", start_node->data);
    if (start_node->next == NULL){
      break;
    }
    printf(", ");
    start_node = start_node->next;
  }

  if (start_node == end_node){
    printf("%d", end_node->data);
  }
  printf("]");
  pthread_mutex_unlock(&list_mutex);
};


/**
 * Displays the entire linked list by printing the data of all nodes.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 *
 * Behavior:
 * - Calls `list_display_range` to display all nodes in the list.
 */
void list_display(Node** head){
  list_display_range(head, NULL, NULL);
};


/**
 * Counts the number of nodes in the linked list.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 * @return The number of nodes in the list.
 *
 * Behavior:
 * - Traverses the linked list and counts the nodes.
 */
int list_count_nodes(Node** head){
  pthread_mutex_lock(&list_mutex);
  
  Node* current = *head;
  int i = 0;
  while (current != NULL){
    i++;
    current = current->next;
  }
  pthread_mutex_unlock(&list_mutex);
  return i;
};


/**
 * Frees all nodes in the linked list and cleans up memory.
 *
 * @param head Pointer to a pointer to the head node of the linked list.
 *
 * Behavior:
 * - Traverses the list and frees each node using `mem_free`.
 * - Sets the head pointer to `NULL` after the cleanup is complete.
 */
void list_cleanup(Node** head){
  pthread_mutex_lock(&list_mutex);
  
  Node* current = *head;
  Node* next;

  while (current != NULL){
    next = current->next;
    mem_free(current);
    current = next;
  }
  *head = NULL;
  mem_deinit();
  pthread_mutex_unlock(&list_mutex);
};
