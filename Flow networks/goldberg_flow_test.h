#ifndef __GOLBER_FLOW_TEST__
#define __GOLBER_FLOW_TEST__

#include "goldberg_flow.h"
#include "random.h"
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <queue>

class Golberg_flow_tester
{
private:
    int m_random_seed;
    bool is_target_reachable(Goldberg_flow& g) const;
public:
    Golberg_flow_tester(int seed) : m_random_seed(seed) {}
    ~Golberg_flow_tester(){}

    void random_graph(int vertices, int max_capacity);
    void test_random();
    void simple_graph_1();
    void simple_graph_2();
};

bool Golberg_flow_tester::is_target_reachable(Goldberg_flow& g) const
{
    std::queue<int> q;
    q.push(1);
    bool used [g.number_of_vertices()];
    int target = g.number_of_vertices();

    while (!q.empty())
    {
        int cur = q.front();
        q.pop();

        for (const auto edge : g.vertex_neighbours(cur)){
            int neighbor = edge->get_end()->get_ID();

            if (neighbor == target)
                return true;

            if (!used[neighbor]){
                q.push(neighbor);
                used[neighbor] = true;
            }
        }
    }
    return false;
}


void Golberg_flow_tester::random_graph(int vertices, int max_capacity) 
{
    RandomGen random(m_random_seed);
    float probability = (1 + log(vertices)) / vertices;
    int start = 1, 
        end = vertices;

    Goldberg_flow g(vertices, start, end);

    for (int i = 1; i <= vertices; i++)
    {
        for (int j = 1; j <= vertices; j++)
        {
            if (i == j ||g.edge_exist(i, j))
                continue;

            float p = (float)random.next_range(100) / 100;
            if (probability < p){
                unsigned int capacity = (1 + random.next_range(max_capacity)) % max_capacity;
                g.add_edge(i, j, capacity);
            }
        }
    }
    
    if (is_target_reachable(g))
        g.get_max_flow();
    else
        printf("Target is not reachable\n");
}

void Golberg_flow_tester::test_random() 
{
    for (int v : {10, 20, 40, 50}){
        for (int cap: {10, 50, 1000}){
            printf("- Test vertices: %d, max capacity: %d\n", v, cap);
            random_graph(v, cap);
        }
    }
}

void Golberg_flow_tester::simple_graph_1() 
{
    Goldberg_flow g (6, 1, 6);
    g.add_edge(1, 2, 10);
    g.add_edge(1, 3, 13);
    g.add_edge(2, 3, 3);
    g.add_edge(3, 6, 7);
    g.add_edge(3, 4, 6);
    g.add_edge(4, 5, 10);
    g.add_edge(5, 6, 5);

    assert (is_target_reachable(g));
    assert (g.get_max_flow() == 12);
}

void Golberg_flow_tester::simple_graph_2() 
{
    Goldberg_flow g (4, 1, 4);
    g.add_edge(3, 1, 17);
    g.add_edge(1, 2, 10);
    g.add_edge(1, 3, 5);
    g.add_edge(3, 2, 3);
    g.add_edge(2, 3, 1);
    g.add_edge(2, 4, 2);
    g.add_edge(3, 4, 7);

    assert (is_target_reachable(g));
    assert(g.get_max_flow() == 8);
}



#ifndef NDEBUG

void Goldberg_flow::test_height_diff() 
{
    int residual = 0;
    for(const auto& e : m_edges){
        residual = e.second.get_residual(e.second.m_start);
        if (residual > 0){
            assert((e.second.get_start()->get_height() - e.second.get_end()->get_height()) <= 1);
        }
    }
}

void Goldberg_flow::test_excess_flow() 
{
    for(const Vertex& vertex : m_vertices){      
        int e_flow = 0;
        for (const auto& edge : vertex.m_in_edges)
        {
            e_flow += edge->get_flow(edge->m_start);
        }
        
        for (const auto& edge : vertex.m_out_edges)
        {
            e_flow -= edge->get_flow(edge->m_start);
        }

        if (vertex != *m_source){
            assert(vertex.get_excess_flow() >= 0);    
            assert(vertex.get_excess_flow() == e_flow);    
        }
    }
}

void Goldberg_flow::test_height_limit() 
{
    int limit = m_vertices.size() - 1;

    for(const Vertex& vertex : m_vertices){
        if (vertex == *m_source){
            assert(vertex.get_height() == limit);
        } else if (vertex == *m_target){
            assert(vertex.get_height() == 0);
        } else {
            assert(vertex.get_height() <= 2*limit);   
        } 
    }
}

void Goldberg_flow::test_flow() 
{
    int flow = 0;
    for(const auto& e : m_edges){
        flow = e.second.get_flow(e.second.m_start);
        assert(0 <= flow && flow <= e.second.get_capacity());
    }
}

#endif // NDEBUG

#endif // __GOLBER_FLOW_TEST__