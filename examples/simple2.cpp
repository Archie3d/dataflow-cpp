#include <iostream>
#include <cmath>
#include "../df.h"

/*
    Custom node that returns maximum value of its two integer inputs
*/

class Max : public df::Node,
            public df::Inputs<int, int>,
            public df::Outputs<int>
{
public:

    Max(df::Graph &g)
        : Node(g)
    {}

    void evaluate() override
    {
        out<0>() = std::max(inValue<0>(), inValue<1>());
    }

};

void example_simple2()
{
    df::Graph g;

    auto& a = g.variable<int>(10);
    auto& b = g.variable<int>(20);

    auto& max = g.node<Max>();

    a >> max.in<0>();
    b >> max.in<1>();

    g.evaluate();
    std::cout << "Maximum value: " << max.out<0>() << "\n";
}
