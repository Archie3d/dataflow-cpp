#include <iostream>
#include <cmath>
#include "../df.h"

/**
 * @brief Single pole low-pass filter.
 * Inputs:
 *      <0> - input sample
 *      <1> - cut-off frequency
 * Outputs:
 *      <0> - output sample
 */
template <typename T>
class LowPassFilter : public df::Node,
                      public df::Inputs<T, T>,
                      public df::Outputs<T>
{
public:

    static constexpr double PI = 3.14159265358979323846;

    LowPassFilter(df::Graph &g) : Node(g) {}

    df::Input<T>& frequency() { return df::Inputs<T, T>::otherInputs().firstInput(); }

    void evaluate()
    {
        T k = 2.0*PI*timeStep()*frequency().value();
        T alpha = k / (k + 1.0);

        df::Outputs<T>::firstOutput() = df::Outputs<T>::firstOutput().value() * (1.0 - alpha)
            + df::Inputs<T, T>::firstInput().value() * alpha;
    }
};

/*

This example builds the following graph:

+-------+     +-----------------+
| Noise |---->| Low-pass filter |-----> out
+-------+  +->|                 |
           |  +-----------------+
           |
        frequency

*/


void example_low_pass_filter()
{
    df::Graph g;
    g.sampleRate(100.0);    // 100Hz

    auto& noise = g.noise<float>(-1.0, 1.0);
    auto& filter = g.node<LowPassFilter<float> >();

    noise.out<0>() >> filter.in<0>();

    auto& freq = filter.frequency();
    auto& out = filter.out<0>();

    constexpr double SimulationTime = 10.0;
    constexpr double FrequencyStep = 0.1;

    double time = 0.0;
    int step = 0;
    while (time < SimulationTime) {
        std::cout << time << ", " << freq() << ", " << out() << "\n";
        g.evaluate();
        step++;
        time = step * g.timeStep();
        freq = time * FrequencyStep;
    }
}
