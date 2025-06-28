#include <iostream>
#include "SharedString.h"


int main() {

    SharedString str = "abc abc";
    SharedString preallocated = SharedString(std::integral_constant<std::size_t, 15>());

    std::cout << str.c_str() << std::endl;
    str.make_mutable();
    str.data()[0] = 'b';
    std::cout << str.c_str() << std::endl;

    SharedString str2 = str;

    std::cout << str.references_count() << " " << str2.references_count() << std::endl;
    str2.reserve();
    std::cout << str.references_count() << " " << str2.references_count() << std::endl;
    SharedString substr = str.substr(2, 3);
    std::cout << str.references_count() << " " << str2.references_count() << std::endl;
    std::cout << substr.c_str() << std::endl;
    std::cout << str.references_count() << " " << str2.references_count() << std::endl;


    return 0;
}
