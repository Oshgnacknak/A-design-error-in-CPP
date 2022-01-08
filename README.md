# A design error in C++

Let's begin with a simple C++ Program:
```cpp
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
```
The only thing happening is some memory being managed by a struct.
We could say that instances own the managed memory (hence `Owner`).
Then we give an instance a new name an print the owned address.
Let's run it...
```
$ g++ owner.cpp && ./a.out
b ownes 0x564d1b514eb0
free(): double free detected in tcache 2
zsh: abort (core dumped)  ./a.out
```
What's that? Double Free? But we only created one instance?

A few prints might help us out:
```cpp
#include <stdio.h>
#include <stdlib.h>

struct Owner {
    void* owned;

    Owner() {
        owned = malloc(69);
        printf("Owner constructed: %p\n", owned);
    }

    ~Owner() {
        printf("Owner destructed: %p\n", owned);
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
```
We get:
```
$ g++ owner-print.cpp && ./a.out
Owner constructed: 0x561d5cde4eb0
b ownes 0x561d5cde4eb0
Owner destructed: 0x561d5cde4eb0
Owner destructed: 0x561d5cde4eb0
free(): double free detected in tcache 2
zsh: abort (core dumped)  ./a.out
```

Aha, the destructor actually gets called twice.
The emphasize what C++ compiles that into,
I'm gonna translate it to C:
```c
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
```
Equivalently we get:
```
$ gcc owner-print.c && ./a.out
Owner constructed: 0x55ecdf2152a0
b ownes 0x55ecdf2152a0
Owner destructed: 0x55ecdf2152a0
Owner destructed: 0x55ecdf2152a0
free(): double free detected in tcache 2
zsh: abort (core dumped)  ./a.out
```

Now we can see what the problem really is:
Even if to a programmer it looks like only one instance
that have to be destroyed,
C++ thinks of it as two instances that both have to be cleaned up. 
Problem now is that these two
instances *own* the same memory.

Is this the design error?
C++ make two instances out of one.
Not really, because C++ has a more complex mechanism
when it comes to copying stuff.
More on that can be found on [SO](https://stackoverflow.com/a/4172724).

To clarify, when is say copy,
I mean allocating additional memory
on the heap and copying bytes on the heap.
Copying addresses from on the stack
or in registers is not the issue.

To me, the design error roots where C++ forces
you to submit to complex memory ownership rituals
by always cleaning up after instances go out of scope.

Suddenly you need to implement multiply constructors
just to tell the compiler when to copy memory.
But then because copying is slow your also
supposed to implement even more constructors
hinting to compiler when copies might be avoided.
More detail on these constructors can be found in the SO link above.

Let be suggest a more simple approach:
We only copy when the user explicitly asks for it.
This is the default in so many programming languages,
Java has a arguable messed up but manual clone mechanism,
Python has the `copy` module exactly for that,
and JavaScript has `Object.assign` allowing manual copying.

However, as seen in the first code example
such an approach is not feasible either.
The automatic destruction of instances ruins our day.
We have to get rid of that to,
let's just rename the destructor:
```cpp
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
```
And that actually works!
```
$ g++ owner-print-manuall.cpp && ./a.out
Owner constructed: 0x562180a57eb0
b ownes 0x562180a57eb0
Owner destructed: 0x562180a57eb0
Hello, World!
```

This is what a C-programmer would do.
Sadly, we also don't want to do it like this.
Who ever has done resource management in C knows what I mean:
([Search for `glfwTerminate()`](https://github.com/jjzhang166/GLUS/blob/275698342980922a69ce4057d67bfa56ff0336df/GLUS/src/glus_window_glfw.c#L240A)).

My solution would be to optionally allow for automatic destruction.
For that we introduce the magic `AutoDestruct` struct.
`AutoDestruct<T>` is `T` with the C++-destructor.
```cpp
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
```
A keyword would be more elegant,
but this is the best that can be done
without changing the compiler.
At least it works:
```
Owner constructed: 0x55ef4c242eb0
b ownes 0x55ef4c242eb0
Owner destructed: 0x55ef4c242eb0
Hello, World!
```

As a added bonus we get a fast "return instance from procedure" for free:
```cpp
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
```
And with zero copies:
```
$ g++ owner-print-returned.cpp && ./a.out
Owner constructed: 0x55ba40246eb0
b ownes `Some message`
Owner destructed: 0x55ba40246eb0
Hello, World!
```
