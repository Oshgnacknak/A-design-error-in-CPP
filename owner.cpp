#include <stdio.h>
#include <stdlib.h>

struct Owner {
    void* owned;

    Owner() {
        owned = malloc(69);
    }

    ~Owner() {
        free(owned);
    }
};

int main() {
    {
        Owner a;
        Owner b = a;
        printf("b ownes %p\n", b.owned);
    }
    printf("Hello, World!\n");

    return 0; 
}
