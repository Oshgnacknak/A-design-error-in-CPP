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

template<typename T>
struct AutoDestruct : public T {
    ~AutoDestruct() {
        this->destruct();
    }
};

int main() {
    {
        AutoDestruct<Owner> a;
        Owner b = a;
        printf("b ownes %p\n", b.owned);
    }
    printf("Hello, World!\n");

    return 0; 
}
