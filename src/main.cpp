#include <list>
#include <string>
#include <iostream>
#include <algorithm>

#include "./define.h"



int main(int argc, char *argv[])
{
    using namespace coref;
    
    char line[1024];
    std::list<event_pair_t> inputs;

    // READ INPUT FROM STDIN.
    while (not std::cin.eof())
    {
        std::cin.getline(line, 1024);
        std::string str(line);

        if (not str.empty())
            inputs.push_back(event_pair_t(str));        
    }

    std::cerr << util::time_stamp()
              << "# of event pairs = " << inputs.size() << std::endl;

    std::list<std::pair<event_pair_t, event_pair_t>> neighbors;
    
    for (auto it1 = inputs.begin(); it1 != inputs.end(); ++it1)
    for (auto it2 = inputs.begin(); it2 != it1; ++it2)
    {
        int cmp = compare(*it1, *it2);
        if (cmp == 1)
            neighbors.push_back(std::make_pair(*it1, *it2));
        if (cmp == -1)
            neighbors.push_back(std::make_pair(*it2, *it1));
    }

    auto cond_prob =
        [](const std::pair<event_pair_t, event_pair_t> &p)
        {
            int freq1 = p.first.frequency();  // F(e)
            int freq2 = p.second.frequency(); // F(e^c)
            
            /* P(c|e) = P(e^c) / P(e) = F(e^c) / F(e) */
            return (double)freq2 / (double)freq1;
        };
    
    neighbors.sort(
        [&cond_prob](const std::pair<event_pair_t, event_pair_t> &p,
           const std::pair<event_pair_t, event_pair_t> &q)
        { return cond_prob(p) > cond_prob(q); });

    for (auto p : neighbors)
    {
        if (p.second.frequency() < 100) continue;
        
        std::cout << "ABSTRACT:\t" << (std::string)p.first << std::endl;
        std::cout << "SPECIFIC:\t" << (std::string)p.second << std::endl;
        std::cout << "P(ctx|ev) = " << cond_prob(p) << std::endl << std::endl;
    }
    
    return 0;
}
