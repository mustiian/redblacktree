#ifndef __VERTEX__
#define __VERTEX__

#include <vector>
#include <list>
#include <unordered_map>

class Edge;
class Goldberg_flow;

using edge_pair = std::pair<int, int>;

struct int_pair_hash {
    std::size_t operator () (const edge_pair &p) const {
        auto h1 = std::hash<int>{}(p.first);
        auto h2 = std::hash<int>{}(p.second);

        return h1 ^ h2;  
    }
};

class Vertex
{
    friend class Goldberg_flow;
private:
    const int m_ID;
    int m_height;
    int m_excess_flow;
    std::vector<Edge*> m_edges;
    bool m_excessflow_inserted;
    std::list<Vertex*>::iterator m_excessflow_iterator;
    std::unordered_map<edge_pair, Edge*, int_pair_hash> m_unsaturated;
    static int vertex_number;
public:
    Vertex() : 
        m_ID(vertex_number++), m_height(0), m_excess_flow(0), m_excessflow_inserted(false) {}
    Vertex(int height) : 
        m_ID(vertex_number++), m_height(height), m_excess_flow(0), m_excessflow_inserted(false) {}

    int get_height() const {return m_height;}
    int get_excess_flow() const {return m_excess_flow;}
    int get_ID() const {return m_ID;}

    bool operator ==(const Vertex& v)const{ return this->m_ID == v.m_ID; }
    bool operator !=(const Vertex& v)const{ return this->m_ID != v.m_ID; }
};

int Vertex::vertex_number=0;

#endif // __VERTEX__