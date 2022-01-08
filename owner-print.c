#include <stdio.h>
#include <stdlib.h>

typedef struct {
    void* owned;
} Owner;

void Owner_constructor(Owner* owner) { // Owner::Owner();
    owner->owned = malloc(69);
    printf("Owner constructed: %p\n", owner->owned);
}

void Owner_destructor(Owner* owner) { // Owner::~Owner();
    printf("Owner destructed: %p\n", owner->owned);
    free(owner->owned);
}

int main() {
    {
        Owner a;
        Owner_constructor(&a);

        Owner b = a;
        printf("b ownes %p\n", b.owned);

        Owner_destructor(&a); // a leaves scope
        Owner_destructor(&b); // b leaves scope
    }
    printf("Hello, World!\n");

    return 0; 
}
