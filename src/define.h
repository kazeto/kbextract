#pragma once

#include <string>
#include <vector>
#include <ciso646>
#include <unordered_map>
#include <cstdarg>


namespace coref
{

class context_t;
class event_t;
class event_pair_t;

typedef int index_t;
typedef int frequency_t;
typedef std::string predicate_t;
typedef std::string dependency_t;
typedef std::string argument_t;
typedef std::string dir_dep_t; /// direction:dependency
typedef std::unordered_map<dir_dep_t, context_t> context_map_t;

extern const int RET_EQUAL;
extern const int RET_DIFF;

class context_t
{
public:
    context_t() {}
    context_t(const std::string &s);

    operator std::string() const;

    char direction() const { return m_direction; }
    bool is_governer() const { return direction() == 'g'; }
    bool is_dependant() const { return direction() == 'd'; }
    
    dependency_t dependency() const { return m_dependency; }
    argument_t argument() const { return m_argument; }

private:
    char m_direction;
    dependency_t m_dependency;
    argument_t   m_argument;
};


/** A class for a single event. */
class event_t
{
public:    
    event_t() {}
    event_t(const std::vector<std::string> &splitted, index_t begin);

    operator std::string() const;
    
    const predicate_t& predicate() const { return m_predicate; }
    const dependency_t& relation() const { return m_relation; }
    const argument_t& shared() const { return m_shared; }
    const context_map_t& contexts() const { return m_contexts; }
    const context_map_t& xcomp_contexts() const { return m_contexts_xcomp; }
    frequency_t frequency() const { return m_frequency; }
    bool is_in_contradictory_conjunction() const { return m_flag_however; }
    
private:
    predicate_t m_predicate;
    dependency_t m_relation;
    argument_t m_shared;
    context_map_t m_contexts;
    context_map_t m_contexts_xcomp;
    frequency_t m_frequency;
    bool m_flag_however;
};


/** A class for a shared argument pair. */
class event_pair_t
{
public:
    event_pair_t() {}
    event_pair_t(const std::string &line);
    
    operator std::string() const;

    const std::pair<event_t, event_t>& events() const { return m_events; }
    frequency_t frequency() const { return m_frequency; }

private:
    std::pair<event_t, event_t> m_events;
    frequency_t m_frequency;
};


/** Returns how abstract the formar is more than the latter. */
int compare(const event_pair_t&, const event_pair_t&);
int compare(const event_t&, const event_t&);


namespace util
{

std::vector<std::string> split(
    const std::string &str, const char *delims, const int MAX_NUM = -1);
std::string format(const char *format, ...);
std::string time_stamp();

inline bool is_empty(const std::string s) { return s == "*"; }

template <class Container, class Function> std::string join(
    const Container &container, Function func, const std::string &delim)
{
    std::string out;
    for (auto e : container)
        out += (out.empty() ? "" : delim) + func(e);
    return out;
}


}


}
