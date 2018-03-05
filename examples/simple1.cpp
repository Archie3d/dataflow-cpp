#include <iostream>
#include <cmath>
#include "../df.h"

/*
    Build graph that computes (a + b) * c expression
*/

void example_simple1()
{
    df::Graph g;

    // Data nodes
    auto& a = g.variable<float>(1.0f);
    auto& b = g.variable<float>(2.0f);
    auto& c = g.variable<float>(3.0f);

    // Build (a + b) * c graph
    auto& add = g.add<float>();
    auto& mul = g.mul<float>();

    // Connect nodes
    a >> add.in<0>();
    b >> add.in<1>();
    add.out<0>() >> mul.in<0>();
    c >> mul.in<1>();

    // Capture output, which holds the result
    auto &out = mul.out<0>();

    // Evaluate graph
    g.evaluate();
    std::cout << "Result: " << out() << "\n";
}
