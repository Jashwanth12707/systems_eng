#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
//Metadata stored at the beginning of each allocator block.

typedef struct header {
    size_t size;
    bool free;
    struct header *prev;
    struct header *next;
} header;

unsigned char heap[4096];

// Initialize the memory allocator by creating a single free block that spans the entire heap.
//return type header coz we are returning the pointer to the initial header of the free block.
header *initialize_allocator() {
    header *initial_header = (header *)heap;
    initial_header->size = sizeof(heap) - sizeof(header);
    initial_header->free = true;
    initial_header->next = NULL;
    initial_header->prev = NULL;
    return initial_header;
}

void*my_malloc(size_t size){   
    header*curr=(header*)heap;
    while(curr!=NULL){
        if(curr->free && curr->size >= size +sizeof(header)){//check if the current block is free and has enough space for the requested size plus the size of the header){
            header*new_header=(header*)((unsigned char*)(curr)+sizeof(header)+size);//creating a new header for the remaining free block after allocation
            new_header->size=curr->size-size-sizeof(header);//setting the size of the new free block
            new_header->free=true;//makeing the new block as free coz we gonna allocate the old one
            new_header->prev=curr;//make the new header point to the current header as its previous
            if(curr->next!=NULL){//if the current header has a next header make the new header point to it as its next
                new_header->next=curr->next;
                curr->next->prev=new_header;//make the next header's previous point to the new header
            }
            else{
                new_header->next=NULL;//if the current header has no next header make the new header's next as NULL
            }
            curr->next=new_header;//make the current header's next point to the new header
            new_header->prev=curr;//make the new header's previous point to the current header
            curr->size=size;//make the current header size as the requested size
            curr->free=false;//change the status of the current header to allocated
            return (void*)((unsigned char*)curr)+sizeof(header);//return the address of the allocated memory block (after the header)
        }
        //if he current block is free  but it doesnt have space for a new header and the req size dont split just change the 
        //current headers's metadata to allocated and return the address of the allocated memory block (after the header)
        else if(curr->free && curr->size >= size){
            curr->free=false;
            return (void*)((unsigned char*)curr)+sizeof(header);
        }
        curr=curr->next;
    }
    return NULL;
}

void my_free(void *ptr){
    if(ptr==NULL){
        return;
    }  
    //coalescing (merging) adjacent blocks and adding up the size and header of the next block to the current block and updating the next pointer of the current block to point to the next 
    //block's next pointer and updating the previous pointer of the next block's next pointer to point to the current block.
    header*curr=(header*)((unsigned char*)ptr-sizeof(header));
    curr->free=true;
    while(curr->next!=NULL && curr->next->free==true){
        curr->size+=sizeof(header)+curr->next->size;
        curr->next=curr->next->next;
        if(curr->next!=NULL){
            curr->next->prev=curr;
        }
    }
    while(curr->prev!=NULL && curr->prev->free==true){
        curr->prev->size+=sizeof(header)+curr->size;
        curr->prev->next=curr->next;
        if(curr->next!=NULL){
            curr->next->prev=curr->prev;
        }
        curr=curr->prev;
    }
}

void *my_calloc(size_t size, size_t count){
    
    if(count==0 || size==0){
        return NULL;
    }
    if(size>SIZE_MAX/count){
        return NULL;
    }
    size_t total_size=size*count;
    void*ptr=my_malloc(total_size);
    if(ptr==NULL){
        return NULL;
    }
    for(size_t i=0;i<total_size;i++){
        ((unsigned char*)ptr)[i]=0;
    }
    return ptr;
}

void *my_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return my_malloc(size);

    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    header *oldheader = (header *)((unsigned char *)ptr - sizeof(header));
    size_t old_size = oldheader->size;

    if (size < old_size){
        size_t rem=old_size-size;
        if(rem>=sizeof(header)+1){
            header*newfree=(header*)((unsigned char*)oldheader+sizeof(header)+size);
            newfree->size=rem-sizeof(header);
            newfree->free=true;
            newfree->prev=oldheader;
            newfree->next=oldheader->next;
            if(newfree->next!=NULL){
                newfree->next->prev=newfree;
            }
            oldheader->next=newfree;
            oldheader->size=size;
        }
        return ptr;
    }

    header *next_header = oldheader->next;

    if (next_header != NULL &&
        next_header->free &&
        old_size + sizeof(header) + next_header->size >= size) {

        size_t combined_size = old_size + sizeof(header) + next_header->size;
        size_t remaining = combined_size - size;

        if (remaining >= sizeof(header) + 1) {
            header *new_free = (header *)((unsigned char *)oldheader + sizeof(header) + size);

            new_free->size = remaining - sizeof(header);
            new_free->free = true;
            new_free->prev = oldheader;
            new_free->next = next_header->next;

            if (new_free->next != NULL)
                new_free->next->prev = new_free;

            oldheader->next = new_free;
            oldheader->size = size;
        } else {
            oldheader->size = combined_size;
            oldheader->next = next_header->next;

            if (oldheader->next != NULL)
                oldheader->next->prev = oldheader;
        }

        return ptr;
    }

    void *new_ptr = my_malloc(size);
    if (new_ptr == NULL)
        return NULL;

    for (size_t i = 0; i < old_size; i++)
        ((unsigned char *)new_ptr)[i] = ((unsigned char *)ptr)[i];

    my_free(ptr);
    return new_ptr;
}

int heap_check() {
    header *curr= (header *)heap;
    while(curr!=NULL){
        if(curr->size==0 || curr->size>sizeof(heap)){
            printf("Invalid size detected: %zu\n", curr->size);
            return 0;
        }else if(curr->next!=NULL && curr->next->prev!=curr){
            printf("Invalid next-prev link detected\n");
            return 0;
        }else if(curr->prev!=NULL && curr->prev->next!=curr){
            printf("Invalid prev-next link detected\n");
            return 0;
        }else if(curr->free && curr->next!=NULL && curr->next->free){
            printf("Invalid free block detected\n");
            return 0;
        }else if(curr->prev!=NULL && curr->prev->free && curr->free){
            printf("Invalid free block detected\n");
            return 0;
        }else if((unsigned char*)curr<heap || (unsigned char*)curr>heap+sizeof(heap)){
            printf("Invalid pointer detected: %p\n", (void*)curr);
            return 0;
        }else if(curr->next != NULL &&
        ((unsigned char *)curr->next < heap ||
         (unsigned char *)curr->next > heap + sizeof(heap))){
            printf("Invalid next pointer detected: %p\n", (void *)curr->next);
            return 0;}
        curr=curr->next;
    }
    return 1;
}

int main()
{
    initialize_allocator();

    printf("Testing malloc\n");

    void *a = my_malloc(100);
    void *b = my_malloc(200);
    void *c = my_malloc(300);

    printf("a = %p\n", a);
    printf("b = %p\n", b);
    printf("c = %p\n", c);

    printf("\nTesting free and coalescing\n");

    my_free(b);
    my_free(a);

    void *d = my_malloc(250);
    printf("d = %p\n", d);

    printf("\nTesting calloc\n");

    int *arr = my_calloc(10, sizeof(int));

    for (int i = 0; i < 10; i++)
        printf("%d ", arr[i]);

    printf("\n");

    printf("\nTesting calloc overflow\n");

    void *bad = my_calloc(SIZE_MAX, 2);
    printf("overflow = %p\n", bad);

    printf("\nTesting realloc shrink\n");

    void *r1 = my_malloc(200);
    printf("r1 before = %p\n", r1);

    r1 = my_realloc(r1, 100);
    printf("r1 after = %p\n", r1);

    printf("\nTesting realloc grow in place\n");

    void *r2 = my_malloc(100);
    my_free(c);

    printf("r2 before = %p\n", r2);

    r2 = my_realloc(r2, 300);
    printf("r2 after = %p\n", r2);

    printf("\nTesting realloc move\n");

    void *x = my_malloc(100);
    void *y = my_malloc(200);

    printf("x before = %p\n", x);

    x = my_realloc(x, 500);

    printf("x after = %p\n", x);

    printf("\nTesting realloc NULL\n");

    void *z = my_realloc(NULL, 100);
    printf("z = %p\n", z);

    printf("\nTesting realloc size 0\n");

    z = my_realloc(z, 0);
    printf("z = %p\n", z);
    int hevari=heap_check();
    if(hevari){
        printf("Heap is valid\n");
    }else{
        printf("Heap is invalid\n");
    }

    return 0;
}
   