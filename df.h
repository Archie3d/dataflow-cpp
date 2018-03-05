/*
                          dataflow-cpp

    Copyright (C) 2018 Arthur Benilov,
    arthur.benilov@gmail.com

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.
*/

#ifndef DF_H_INCLUDED
#define DF_H_INCLUDED

#include <memory>
#include <random>

namespace df {

/**
 * @brief Input port.
 * Input can be either connected to an output, or to an internal default value.
 */
template <typename T>
class Input
{
public:

    Input()
        : m_defaultValue(),
          m_pConnectedValue(&m_defaultValue)
    {
    }

    const T& value() const { return *m_pConnectedValue; }
    const T& operator()() const { return value(); }

    void connect(T *pValue) { m_pConnectedValue = pValue; }
    void disconnect() { m_pConnectedValue = &m_defaultValue; }

    Input<T>& operator =(const T& value)
    {
        *m_pConnectedValue = value;
        return *this;
    }

private:

    Input(const Input<T>&) = delete;
    Input<T>& operator =(const Input<T>&) = delete;

    T m_defaultValue;
    T* m_pConnectedValue;
};

/**
 * @brief Output port.
 * Output port holds a value, which can be referenced by connected inputs
 */
template <typename T>
class Output
{
public:

    Output()
        : m_value()
    {
    }

    const T& value() const { return m_value; }
    const T& operator()() const { return value(); }

    void connect(Input<T> &input)
    {
        input.connect(&m_value);
    }

    Output<T>& operator =(const T& value)
    {
        m_value = value;
        return *this;
    }

    Output<T>& operator >>(Input<T> &input)
    {
        connect(input);
        return *this;
    }

private:
    Output(const Output<T>&) = delete;
    Output<T>& operator =(const Output<T>&) = delete;

    T m_value;
};

/**
 * @brief List of input ports
 */
template <typename... Ts>
class Inputs;

template<>
class Inputs<>
{
public:
    Inputs() {}
};

template <typename T, typename... Ts>
class Inputs<T, Ts...> : private Inputs<Ts...>
{
public:

    using OtherInputs = Inputs<Ts...>;

    template <int N, typename E> struct Iterator;
    template <typename E> struct Iterator<0, E>
    {
        static decltype(auto) get(E &in) { return in.firstInput(); }
    };
    template <int N, typename E> struct Iterator
    {
        using Next = Iterator<N - 1, typename E::OtherInputs>;
        static decltype(auto) get(E &in) { return Next::get(in.otherInputs()); }
    };

    template <int N>
    decltype(auto) in()
    {
        return Iterator<N, Inputs<T, Ts...> >::get(*this);
    }

    template <int N>
    decltype(auto) inValue()
    {
        return Iterator<N, Inputs<T, Ts...> >::get(*this)();
    }

    Input<T>& firstInput() { return m_thisInput; }
    OtherInputs& otherInputs() { return *this; }

private:

    Input<T> m_thisInput;
};

/**
 * @brief List of outputs.
 */
template <typename... Ts>
class Outputs;

template<>
class Outputs<>
{
public:
    Outputs() {}
};

template <typename T, typename... Ts>
class Outputs<T, Ts...> : private Outputs<Ts...>
{
public:

    using ThisOutputType = T;
    using OtherOutputs = Outputs<Ts...>;

    template <int N, typename E> struct Iterator;
    template <typename E> struct Iterator<0, E>
    {
        static decltype(auto) get(E &in) { return in.firstOutput(); }
    };
    template <int N, typename E> struct Iterator
    {
        using Next = Iterator<N - 1, typename E::OtherOutputs>;
        static decltype(auto) get(E &in) { return Next::get(in.otherOutputs()); }
    };

    template <int N>
    decltype(auto) out()
    {
        return Iterator<N, Outputs<T, Ts...> >::get(*this);
    }

    template <int N>
    decltype(auto) outValue()
    {
        return Iterator<N, Outputs<T, Ts...> >::get(*this)();
    }

    Output<T>& firstOutput() { return m_thisOutput; }
    OtherOutputs& otherOutputs() { return *this; }

private:

    Output<T> m_thisOutput;
};

// Forward declaration
class Graph;

/**
 * @brief Processing node.
 * A node cannot exist outside of a graph.
 */
class Node
{
public:

    friend class df::Graph;
    using Ptr = std::shared_ptr<Node>;

    Node(Graph &g)
        : m_graph(g)
    {}

    virtual ~Node() {}

    virtual void evaluate() {}

protected:

    double timeStep() const;
    double sampleRate() const;

private:
    Node(const Node&) = delete;
    Node& operator =(const Node&) = delete;

    // Reference to a graph this node belongs to.
    Graph &m_graph;
};

//----------------------------------------------------------
// Some predefined nodes

namespace node {

/**
 * @brief Variable value node.
 * This node holds a single value to be connected to inputs.
 */
template <typename T>
class Variable : public Node,
                 public Outputs<T>
{
public:

    Variable(Graph &g)
        : Node(g)
    {}

    Variable& operator =(const T &value)
    {
        Outputs<T>::firstOutput() = value;
        return *this;
    }

    Variable& operator >> (Input<T> &input)
    {
        Outputs<T>::firstOutput() >> input;
        return *this;
    }

};

/**
 * @brief White node generator.
 */
template <typename T>
class WhiteNoise : public Node,
                   public Outputs<T>
{
public:

    WhiteNoise(Graph &g, const T min = 0, const T max = 1)
        : Node(g),
          m_randomEngine(),
          m_distribution(min, max)
    {
    }

    void range(const T min, const T max)
    {
        m_distribution = std::uniform_real_distribution<>(min, max);
    }

    void evaluate() override
    {
        Outputs<T>::firstOutput() = m_distribution(m_randomEngine);
    }

private:
    std::default_random_engine m_randomEngine;
    std::uniform_real_distribution<> m_distribution;
};

/**
 * @brief Input sign change node.
 */
template <typename T>
class Neg : public Node,
            public Inputs<T>,
            public Outputs<T>
{
public:

    Neg(Graph &g)
        : Node(g)
    {}

    void evaluate() override
    {
        Outputs<T>::firstOutput() = -Inputs<T, T>::firstInput().value();
    }
};

/**
 * @brief Add two values.
 */
template <typename T>
class Add : public Node,
            public Inputs<T, T>,
            public Outputs<T>
{
public:

    Add(Graph &g)
        : Node(g)
    {}

    void evaluate() override
    {
        Outputs<T>::firstOutput() = Inputs<T, T>::firstInput().value()
            + Inputs<T, T>::otherInputs().firstInput().value();
    }
};

/**
 * @brief Subtract two values.
 */
template <typename T>
class Sub : public Node,
            public Inputs<T, T>,
            public Outputs<T>
{
public:

    Sub(Graph &g)
        : Node(g)
    {}

    void evaluate() override
    {
        Outputs<T>::firstOutput() = Inputs<T, T>::firstInput().value()
            - Inputs<T, T>::otherInputs().firstInput().value();
    }
};

/**
 * @brief Multiply two values.
 */
template <typename T>
class Mul : public Node,
            public Inputs<T, T>,
            public Outputs<T>
{
public:

    Mul(Graph &g)
        : Node(g)
    {}

    void evaluate() override
    {
        Outputs<T>::firstOutput() = Inputs<T, T>::firstInput().value()
            * Inputs<T, T>::otherInputs().firstInput().value();
    }
};

/**
 * @brief Divide two values.
 */
template <typename T>
class Div : public Node,
            public Inputs<T, T>,
            public Outputs<T>
{
public:

    Div(Graph &g)
        : Node(g)
    {}

    void evaluate() override
    {
        Outputs<T>::firstOutput() = Inputs<T, T>::firstInput().value()
            / Inputs<T, T>::otherInputs().firstInput().value();
    }
};

} // namespace node
//----------------------------------------------------------

/**
 * @brief Dataflow graph.
 */
class Graph
{
public:

    Graph()
        : m_evaluationTimeStep(1e-6)
    {
    }

    double timeStep() const { return m_evaluationTimeStep; }
    void timeStep(double dt) { m_evaluationTimeStep = dt; }

    double sampleRate() const { return 1.0 / m_evaluationTimeStep; }
    void sampleRate(double sr) { m_evaluationTimeStep = 1.0 / sr; }

    template <class N>
    N& node()
    {
        auto nodePtr = std::make_shared<N>(*this);
        registerNode(nodePtr);
        return *nodePtr.get();
    }

    void evaluate()
    {
        // Evaluate all the nodes
        for (auto &n : m_nodes) {
            n->evaluate();
        }
    }

    // Default nodes

    template <typename T>
    node::Variable<T>& variable(const T &value = T())
    {
        auto &var = Graph::node<node::Variable<T> >();
        var.firstOutput() = value;
        return var;
    }

    template <typename T>
    node::WhiteNoise<T>& noise(const T min = 0, const T max = 1)
    {
        auto nodePtr = std::make_shared<node::WhiteNoise<T> >(*this, min, max);
        registerNode(nodePtr);
        return *nodePtr.get();
    }

    template <typename T>
    node::Neg<T>& neg()
    {
        return Graph::node<node::Neg<T> >();
    }

    template <typename T>
    node::Add<T>& add()
    {
        return Graph::node<node::Add<T> >();
    }

    template <typename T>
    node::Sub<T>& sub()
    {
        return Graph::node<node::Sub<T> >();
    }

    template <typename T>
    node::Mul<T>& mul()
    {
        return Graph::node<node::Mul<T> >();
    }

    template <typename T>
    node::Div<T>& div()
    {
        return Graph::node<node::Div<T> >();
    }

private:
    Graph(const Graph&) = delete;
    Graph& operator =(const Graph&) = delete;

    void registerNode(Node::Ptr node)
    {
        m_nodes.push_back(node);
    }

    std::vector<Node::Ptr> m_nodes;

    double m_evaluationTimeStep;
};

} // namespace df

#endif // DF_H_INCLUDED
