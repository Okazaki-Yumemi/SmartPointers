#include <cassert>
#include "Weak_ptr.hpp"

int main() {
    SharedPtr<int> sp(new int(42));

    WeakPtr<int> wp(sp);
    assert(wp.use_count() == 1);
    assert(!wp.expired());

    {
        SharedPtr<int> sp2 = wp.lock();
        assert(sp2.use_count() == 2);
        assert(*sp2 == 42);
    }

    assert(wp.use_count() == 1);

    sp.reset();

    assert(wp.use_count() == 0);
    assert(wp.expired());
    assert(!wp.lock());

    wp.reset();
}