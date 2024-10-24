// Viktor Fransson DVAMI22h

#include "memory_manager.h"

// Mutex for synchronizing memory allocation operations
pthread_mutex_t memory_mutex = PTHREAD_MUTEX_INITIALIZER;

// // Used for one-time initialization of the memory pool
// pthread_once_t init_once = PTHREAD_ONCE_INIT;


/**
 * Defines a block of memory in the custom memory manager.
 *
 * Each block contains a pointer to the memory it manages, the size of the block, 
 * a flag indicating if it's free or in use, and a pointer to the next memory block.
 */
struct memory_block{
    void* ptr; // Point to memory
    size_t size;
    int free;

    struct memory_block* next; // Point to next block
};


// Global variables for managing the memory pool and block list
static char* memory_pool = NULL; // Pointer to memory_pool
static struct memory_block* first_block = NULL; // Pointer to first block in pool


/**
 * Creates a new memory block in the memory manager.
 *
 * @param ptr Pointer to the start of the memory block.
 * @param size Size of the memory block in bytes.
 * @param free Indicates whether the block is free (1) or in use (0).
 * @param next Pointer to the next memory block.
 * @return Pointer to the newly created memory block.
 *
 * Behavior:
 * - Allocates memory for a new memory block structure.
 * - Initializes the block with the provided values for `ptr`, `size`, `free`, and `next`.
 */
struct memory_block* make_block(void* ptr, size_t size, int free, struct memory_block* next){ // Makes a new block and room for its header
    struct memory_block* new_block = (struct memory_block*)malloc(sizeof(struct memory_block));
    *new_block = (struct memory_block){ptr, size, free, next};

    return new_block;
};


/**
 * Initializes the memory pool with the specified size.
 *
 * @param size Size of the memory pool to allocate.
 *
 * Behavior:
 * - Allocates memory of the specified size for the pool.
 * - Creates the first memory block in the pool, marking the entire pool as free.
 */
void mem_init(size_t size){
    memory_pool = malloc(size); // Allocate memory pool

    first_block = make_block(memory_pool, size, 1, NULL); // Make header for first block pointing to the pool
};


/**
 * mem_alloc without lock
 */
void* no_lock_alloc(size_t size){
    struct memory_block* current = first_block;

    while (current != NULL) {
        if (current->free && current->size >= size) {
            
            current->free = 0;
            
            struct memory_block* new_block = make_block((char*)current->ptr + size, current->size - size, 1, current->next);
            
            current->size = size;
            current->next = new_block;

            return current->ptr; // Return pointer to the data part
        }
        current = current->next;
    }
    return NULL;
}


/**
 * Allocates a block of memory of the requested size from the pool.
 *
 * @param size The size of the block to allocate.
 * @return Pointer to the allocated memory, or `NULL` if allocation fails.
 *
 * Behavior:
 * - Searches for a free memory block large enough to satisfy the request.
 * - If a suitable block is found, it is split into two blocks: one for the allocated memory, 
 *   and the remaining part becomes a new free block.
 * - The function returns a pointer to the allocated memory or `NULL` if no suitable block is found.
 */
void* mem_alloc(size_t size){
    pthread_mutex_lock(&memory_mutex);
    void* ptr = no_lock_alloc(size);
    pthread_mutex_unlock(&memory_mutex);
    return ptr;
};


void no_lock_free(void* block){
    struct memory_block* current = first_block;
    
    while (current != NULL){
        if (current->ptr == block){
            current->free = 1;
            if (current->next != NULL && current->next->free == 1){
                struct memory_block* next = current->next;
                current->next = next->next;
                current->size += next->size;
                free(next);
            }
            return;
        }
        current = current->next;
    }
}


/**
 * Frees a previously allocated block of memory, making it available for reuse.
 *
 * @param block Pointer to the block of memory to free.
 *
 * Behavior:
 * - Marks the block as free in the memory manager.
 * - If adjacent memory blocks are also free, they are merged to form a larger block.
 */
void mem_free(void* block){
    pthread_mutex_lock(&memory_mutex);
    no_lock_free(block);
    pthread_mutex_unlock(&memory_mutex);
};


/**
 * Resizes an allocated block of memory to the specified size.
 *
 * @param block Pointer to the block of memory to resize.
 * @param size The new size for the block.
 * @return Pointer to the resized memory block, or a new block if the current block cannot be resized.
 *
 * Behavior:
 * - If the block is large enough for the new size, the function returns the same block.
 * - If the block is too small, a new block is allocated, and the contents of the old block are copied to the new one.
 * - The old block is freed after the data is copied.
 */
void* mem_resize(void* block, size_t size){
    pthread_mutex_lock(&memory_mutex);

    struct memory_block* current_block = first_block;

    while (current_block != NULL){
        if (current_block->ptr == block){
            break;
        }
        current_block = current_block->next;
    }

    if (current_block->size >= size){
        pthread_mutex_unlock(&memory_mutex);
        return block;
    };

    char* new_ptr = no_lock_alloc(size); // Allocate new block with new size

    if (new_ptr != NULL){
    
        memcpy(new_ptr, block, current_block->size); 
        no_lock_free(block); // Free old block
    }

    pthread_mutex_unlock(&memory_mutex);
    return new_ptr;
};


/**
 * Deinitializes the memory pool and frees all memory.
 *
 * Behavior:
 * - Frees the entire memory pool.
 * - Resets the pointers for the memory pool and the first block to `NULL`.
 */
void mem_deinit(){
    pthread_mutex_lock(&memory_mutex);
    free(memory_pool);
    memory_pool = NULL;
    first_block = NULL;
    pthread_mutex_unlock(&memory_mutex);
}
