#include <cassert>
#include "Shared_ptr2.hpp"


int main() {
    SharedPtr<int> a(new int(42));
    SharedPtr<int> b;

    b = a;
    assert(a.use_count() == 2);
    assert(b.use_count() == 2);

    a.reset();
    assert(a.use_count() == 0);
    assert(b.use_count() == 1);
    assert(*b == 42);

    b.reset(nullptr);
    assert(!b);
    assert(b.use_count() == 0);
}