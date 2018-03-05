#include <iostream>
#include <cmath>
#include "../df.h"

/*

This example builds the following graph:

    +-----------+----------------------------> cos(t)
    |           |
    |   +---+   |
    +-->|   |   |      +---+
        | - |---+----->| x |--+
    +-->|   |    dt--->|   |  |
    |   +---+          +---+  |
    |                         |
    +------------------------ | ---+
                              |    |
    +-------------------------+    |
    |                              |
    |   +---+          +---+       |
    +-->|   |    dt--->|   |       |
        | + |---+----->| x |-------+
    +-->|   |   |      +---+
    |   +---+   |
    |           |
    +-----------+----------------------------> sin(t)

It represents the following differential equations:

    dsin
    ----  = cos(t)
     dt

    dcos
    ----  = -sin(t)
     dt

*/

void example_sin_cos_generator()
{
    df::Graph g;
    g.sampleRate(100.0);    // dt = 1/100.0

    // Integration time step
    auto& dt = g.variable<float>(g.timeStep());

    // Data flow nodes
    auto &csub = g.sub<float>();
    auto &cmul = g.mul<float>();
    auto &sadd = g.add<float>();
    auto &smul = g.mul<float>();

    // Connections between the nodes
    csub.out<0>() >> csub.in<0>() >> cmul.in<0>();
    dt >> cmul.in<1>();
    smul.out<0>() >> csub.in<1>();

    sadd.out<0>() >> sadd.in<0>() >> smul.in<0>();
    dt >> smul.in<1>();
    cmul.out<0>() >> sadd.in<1>();

    // Outputs to capture
    auto &output_cos = csub.out<0>();
    auto &output_sin = sadd.out<0>();

    // Initialize the outputs
    output_cos = 1.0f;
    output_sin = 0.0f;

    // Run simulation
    constexpr double SimulationTime = 10.0; //s
    double time = 0.0;
    int cycle = 0;
    while (time < SimulationTime) {
        std::cout << time << ", " << output_cos()  << ", " << output_sin() << "\n";
        g.evaluate();
        cycle++;
        time = cycle * g.timeStep();
    }
}
