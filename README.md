# dataflow-cpp
Prototype dataflow programming with C++

Dataflow graph is built of computational nodes. Each node can have inputs and outputs.

Simplest example:
```cpp
df::Graph g;

// Data nodes
auto& a = g.variable<float>(1.0f);
auto& b = g.variable<float>(2.0f);
auto& c = g.variable<float>(3.0f);

// Build (a + b) * c graph
auto& add = g.add<float>();
auto& mul = g.mul<float>();

a >> add.in<0>();
b >> add.in<1>();
add.out<0>() >> mul.in<0>();
c >> mul.in<1>();

// Capture output, which holds the result
auto &out = mul.out<0>();

// Evaluate the graph
g.evaluate();
std::cout << "Result: " << out() << "\n";
```

Creating a custom node
```cpp
// Compute maximum value of two integers
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

// Usage
void example()
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
```