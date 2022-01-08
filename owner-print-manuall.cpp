#include <stdio.h>
#include <stdlib.h>

struct Owner {
    void* owned;

    Owner() {
        owned = malloc(69);
        printf("Owner constructed: %p\n", owned);
    }

    void destruct() {
        printf("Owner destructed: %p\n", owned);
        free(owned);
    }
};

int main() {
    {
        Owner a;
        Owner b = a;
        printf("b ownes %p\n", b.owned);

        b.destruct();
    }
    printf("Hello, World!\n");

    return 0; 
}
