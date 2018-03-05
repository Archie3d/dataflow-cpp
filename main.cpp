#include <iostream>
#include <cmath>
#include "df.h"

/// @see examples/ folder

extern void example_simple1();
extern void example_simple2();
extern void example_low_pass_filter();
extern void example_sin_cos_generator();

int main()
{
    example_simple1();
    example_simple2();
    example_low_pass_filter();
    example_sin_cos_generator();

    return 0;
}
