#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Owner makeMessageOwner() {
    Owner o;
    strcpy((char*) o.owned, "Some message");
    return o;
}

template<typename T>
struct AutoDestruct : public T {
    template<typename... Args>
    AutoDestruct(Args&&... args) : T(args...) {}

    ~AutoDestruct() {
        this->destruct();
    }
};

int main() {
    {
        AutoDestruct<Owner> a = (AutoDestruct<Owner>) makeMessageOwner();
        Owner b = a;
        printf("b ownes `%s`\n", b.owned);
    }
    printf("Hello, World!\n");

    return 0; 
}
