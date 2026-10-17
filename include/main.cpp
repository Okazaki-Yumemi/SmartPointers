#include<bits/stdc++.h>
#include "Unique_ptr.hpp"
#include "Shared_ptr.hpp"
int main(){
    // Test 1: 移动构造
    UniquePtr<int> a(new int(42));
    UniquePtr<int> b(std::move(a));

    // 应满足：a.get() == nullptr，*b == 42


    assert(a.get() == nullptr);
    assert(*b == 42);

    return 0;
}