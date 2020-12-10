#ifndef __GOLBER_FLOW__
#define __GOLBER_FLOW__

#include "vertex.h"
#include "edge.h"
#include <unordered_map>
#include <functional>
#include <utility>
#include <algorithm>
#include <cstdio>

//#define NDEBUG

struct int_pair_hash {
    std::size_t operator () (const std::pair<int,int> &p) const {
        auto h1 = std::hash<int>{}(p.first);
        auto h2 = std::hash<int>{}(p.second);

        return h1 ^ h2;  
    }
};

class Goldberg_flow
{
public:
    Goldberg_flow(){}
    Goldberg_flow(int vertices, int source, int target);
    ~Goldberg_flow();
    
    void add_edge(int from, int to, int capacity);
    int get_max_flow();
    int number_of_edges()const{return m_edges.size();}
    int number_of_vertices()const{return m_vertices.size() - 1;}
    bool edge_exist(int from, int to) const{
        auto edge = std::make_pair(from, to);
        return m_edges.find(edge) != m_edges.end();
        }
    const std::vector<Edge*>& vertex_neighbours(int vertex) { return m_vertices[vertex].m_out_edges;}
    
#ifndef NDEBUG
    void test_height_diff();
    void test_excess_flow();
    void test_height_limit();
    void test_flow();
#else
    void test_height_diff(){}
    void test_excess_flow(){}
    void test_height_limit(){}
    void test_flow(){}
#endif

private:
    Vertex *m_source, *m_target;
    std::vector<Vertex> m_vertices;
    std::unordered_map<std::pair<int, int>, Edge, int_pair_hash> m_edges;

    void init();
    Vertex* get_max_excess_flow_vertex();
    Edge* get_positive_residual_edge(Vertex* vertex);
    void push (Vertex* vertex, Edge* edge);
    void relable (Vertex* vertex);
    int count_max_flow();
};
/**
 * initialization constructor
 * 
 * @param  {int} edges    : Number of edges
 * @param  {int} vertices : Number of vertices
 * @param  {int} source   : Index of source vertex
 * @param  {int} target   : Index of target vertex
 */
Goldberg_flow::Goldberg_flow(int vertices, int source, int target) : 
        m_vertices(vertices + 1) 
{
    m_source = &m_vertices[source];
    m_target = &m_vertices[target];
}

Goldberg_flow::~Goldberg_flow() 
{
    m_vertices.clear();
    m_edges.clear();
    Vertex::vertex_number = 0;
}

/**
 * Add new edge from the vertex to another vertex
 * 
 * @param  {int} from     : Index of the vertex where the edge starts from
 * @param  {int} to       : Index of the vertex where the edge comes to
 * @param  {int} capacity : Capacity of the edge
 */
void Goldberg_flow::add_edge(int from, int to, int capacity) 
{
    auto edge = std::make_pair(from, to);

    if (m_edges.find(edge) != m_edges.end())
        return;

    m_edges[edge] = Edge(&m_vertices[from], &m_vertices[to], capacity);
    
    m_vertices[from].m_out_edges.push_back(&m_edges[edge]);
    m_vertices[to].m_in_edges.push_back(&m_edges[edge]);
}

/**
 * Find the maximum flow and returns it
 * 
 * @return {int}  : The possible maximum flow
 */
int Goldberg_flow::get_max_flow() 
{
    init();
    Vertex* vertex = get_max_excess_flow_vertex();
    Edge* edge;    

    while (vertex != nullptr && vertex->m_excess_flow > 0)
    {
        edge = get_positive_residual_edge(vertex);       

        if (edge != nullptr)
            push(vertex, edge);
        else
            relable(vertex);

        vertex = get_max_excess_flow_vertex();
    }

    int max_flow = count_max_flow();

#ifndef NDEBUG
    std::printf("finish, max flow %d\n", max_flow);
#endif

    return count_max_flow();
}

/**
 * Initialization of Goldberg flow algorithm
 * 
 */
void Goldberg_flow::init() 
{
    m_source->m_height = number_of_vertices();
    test_height_limit();

#ifndef NDEBUG
    std::printf("init\n");
#endif

    int flow = 0;
    for (auto edge : m_source->m_out_edges){
        flow = edge->m_capacity;

        edge->m_flow += flow;
        edge->m_reverse_flow -= flow;
        edge->m_end->m_excess_flow += flow; 
        edge->m_start->m_excess_flow -= flow; 
#ifndef NDEBUG
    std::printf("push: from %d to %d flow %d ", m_source->m_ID, edge->m_end->m_ID, flow); 
    std::printf("| new flow %d, new rev flow %d\t", edge->m_flow, edge->m_reverse_flow);
    std::printf("| capacity %d\n", edge->m_capacity);
#endif        
    }

    test_excess_flow();
    test_flow();
    test_height_diff();
}

/**
 * Finds vertex with the maximum excess flow.
 * If there aren't any, then returns null.
 * 
 * @return {Vertex*}  : Vertex with the maximum excess flow
 */
Vertex* Goldberg_flow::get_max_excess_flow_vertex() 
{
    Vertex* max = nullptr;
    int max_excess = 0;
    for(auto& vertex: m_vertices){
        if (vertex != *m_source && vertex != *m_target)
            if (vertex.m_excess_flow > max_excess){
                max = &vertex;
                max_excess = vertex.m_excess_flow;
            }
    }

    return max;
}

/**
 * Finds edge with positive residual.
 * If there aren't any, then returns null.
 * 
 * @param  {Vertex*} vertex : Vertex where the edge comes from
 * @return {Edge*}          : Edge with positive residual
 */
Edge* Goldberg_flow::get_positive_residual_edge(Vertex* vertex) 
{
    int residual = 0, height_diff = 0;
    for(auto e : vertex->m_out_edges){
        residual = e->get_residual(vertex);
        height_diff = vertex->m_height - e->m_end->m_height;
        if (residual > 0 && height_diff > 0)
            return e;
    }
    for(auto e : vertex->m_in_edges){
        residual = e->get_residual(vertex);
        height_diff = vertex->m_height - e->m_start->m_height;
        if (residual > 0 && height_diff > 0 && e->get_flow(vertex) != 0)
            return e;
    }
    return nullptr;
}

/**
 * Push flow from overflowing vertex along the edge
 * 
 * @param  {Vertex*} vertex : Overflowing vertex
 * @param  {Edge*} edge     : Edge along which will be pushed the flow
 */
void Goldberg_flow::push(Vertex* vertex, Edge* edge) 
{
    bool outgoing = edge->is_outgoing(vertex);
    int flow = std::min(vertex->m_excess_flow, edge->get_residual(vertex));
    flow = !outgoing && flow > edge->m_flow? edge->m_flow : flow;

    int actual_flow = outgoing? flow : -flow;
    Vertex* target = edge->get_another_vertex(vertex);

    edge->m_flow += actual_flow;
    edge->m_reverse_flow -= actual_flow;
    
    vertex->m_excess_flow -= flow;
    target->m_excess_flow += flow;

#ifndef NDEBUG
    std::printf("push: from %d to %d actual flow %d, flow %d ", vertex->m_ID, target->m_ID, actual_flow, flow); 
    std::printf("| new flow %d, new rev flow %d\t", edge->m_flow, edge->m_reverse_flow);
    std::printf("| capacity %d\n", edge->m_capacity);
#endif

    test_excess_flow();
    test_flow();
}
/**
 *  Relabel vertex 
 * 
 * @param  {Vertex*} vertex : Given vertex
 */
void Goldberg_flow::relable(Vertex* vertex)
{
    vertex->m_height += 1;

#ifndef NDEBUG
    std::printf("relable: vertex %d, new height %d\n", vertex->m_ID, vertex->m_height);
#endif

    test_height_diff();
    test_height_limit();
}

/**
 * Count flow of all outgoing edges from the source)
 * 
 * @return {int}  : Return sum of all flows
 */
int Goldberg_flow::count_max_flow() 
{
    int flow = 0;
    for (auto edge : m_source->m_out_edges)
    {
        flow += edge->m_flow;
    }

    return flow;
}

#ifndef NDEBUG
#include "goldberg_flow_test.h"
#endif

#endif // __GOLBER_FLOW__