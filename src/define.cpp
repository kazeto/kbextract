#include <iostream>
#include <cassert>
#include <cstring>
#include <ctime>
#include <climits>

#include "define.h"


namespace coref
{


context_t::context_t(const std::string &s)
{
    auto spl = util::split(s, ":");
    assert(spl.size() == 3);

    m_direction  = spl[0].front();
    m_dependency = spl[1];
    m_argument   = spl[2];
}


context_t::operator std::string() const
{
    return util::format(
        "%c:%s:%s",
        m_direction, m_dependency.c_str(), m_argument.c_str());
}


event_t::event_t(const std::vector<std::string> &splitted, index_t begin)
{
    auto pred_rel = util::split(splitted.at(begin), ":");
    m_predicate = pred_rel.front();
    m_relation = pred_rel.back();
    m_shared = splitted.at(begin + 1);

    if (splitted.at(begin + 2) != "*")
    for (auto s : util::split(splitted.at(begin + 2), " "))
    {
        context_t ctx(s);
        dir_dep_t key = util::format(
            "%c:%s", ctx.direction(), ctx.dependency().c_str());
        m_contexts[key] = ctx;
    }
    
    if (splitted.at(begin + 3) != "*")
    for (auto s : util::split(splitted.at(begin + 3), " "))
    {
        context_t ctx(s);
        dir_dep_t key = util::format(
            "%c:%s", ctx.direction(), ctx.dependency().c_str());
        m_contexts_xcomp[key] = ctx;
    }

    m_flag_however = (splitted.at(begin + 4) == "True");
    sscanf(splitted.at(begin + 5).c_str(), "%d", &m_frequency);
}


event_t::operator std::string() const
{
    auto ctx2str =
        [](const std::pair<dir_dep_t, context_t> &ctx) -> std::string
        { return (std::string)ctx.second; };

    std::string ctxs =
        m_contexts.empty() ? "*" :
        util::join(m_contexts, ctx2str, " ").c_str();
    std::string ctxs_xcomp =
        m_contexts_xcomp.empty() ? "*" :
        util::join(m_contexts_xcomp, ctx2str, " ").c_str();
    
    return util::format(
        "%s:%s\t%s\t%s\t%s\t%s\t%d",
        m_predicate.c_str(),
        m_relation.c_str(),
        m_shared.c_str(),
        ctxs.c_str(), ctxs_xcomp.c_str(),
        (m_flag_however ? "True" : "False"),
        m_frequency);
}


event_pair_t::event_pair_t(const std::string &line)
{
    auto splitted = util::split(line, "\t");
    m_events = std::make_pair(event_t(splitted, 0), event_t(splitted, 6));
    sscanf(splitted.at(12).c_str(), "%d", &m_frequency);
}


event_pair_t::operator std::string() const
{
    return util::format(
        "%s\t%s\t%d",
        ((std::string)m_events.first).c_str(),
        ((std::string)m_events.second).c_str(),
        m_frequency);
}


const int RET_EQUAL = 0;
const int RET_DIFF  = INT_MAX;


int compare(const event_pair_t &p1, const event_pair_t &p2)
{
    int cmp1 = compare(p1.events().first, p2.events().first);
    int cmp2 = compare(p1.events().second, p2.events().second);

    if (cmp1 == RET_DIFF or cmp2 == RET_DIFF)
        return RET_DIFF;
    else if (cmp1 * cmp2 < 0)
        return RET_DIFF;
    else
        return (cmp1 + cmp2);
}


int compare(const event_t &e1, const event_t &e2)
{
    int out1(0), out2(0);
    
    auto cmp_str =
        [&](const std::string &s1, const std::string &s2) -> bool
        {
            if (s1 != s2)
            {
                if (util::is_empty(s1)) ++out1;
                else if (util::is_empty(s2)) ++out2;
                else return false;
            }
            return true;
        };

    auto cmp_ctxs =
        [](const context_map_t &ctxs1, const context_map_t &ctxs2,
           int *out2) -> bool
        {
            for (auto p : ctxs1)
            {
                const context_t &ctx1 = p.second;
                auto found = ctxs2.find(p.first);
                
                if (found == ctxs2.end())
                    (*out2) += util::is_empty(ctx1.argument()) ? 1 : 2;
                else
                {
                    const context_t &ctx2 = found->second;
                    if (ctx1.argument() != ctx2.argument())
                    {
                        if (util::is_empty(ctx2.argument())) ++(*out2);
                        else return false;
                    }
                }
            }
            return true;
        };

    if (e1.is_in_contradictory_conjunction()
        != e2.is_in_contradictory_conjunction())  return RET_DIFF;
    if (e1.relation() != e2.relation())           return RET_DIFF;
    if (!cmp_str(e1.predicate(), e2.predicate())) return RET_DIFF;
    if (!cmp_str(e1.shared(), e2.shared()))       return RET_DIFF;

    if (!cmp_ctxs(e1.contexts(), e2.contexts(), &out2)) return RET_DIFF;
    if (!cmp_ctxs(e2.contexts(), e1.contexts(), &out1)) return RET_DIFF;
    if (!cmp_ctxs(e1.xcomp_contexts(), e2.xcomp_contexts(), &out2)) return RET_DIFF;
    if (!cmp_ctxs(e2.xcomp_contexts(), e1.xcomp_contexts(), &out1)) return RET_DIFF;

    if (out1 > 0)
        return (out2 > 0) ? RET_DIFF : out1;
    if (out2 > 0)
        return (out1 > 0) ? RET_DIFF : -out2;

    return RET_EQUAL;
}



namespace util
{

/** Splits string into vector of string and returns it. */
std::vector<std::string> split(
    const std::string &str, const char *delims, const int MAX_NUM )
{
    auto _find_split_index =
        [](const std::string &str, const char *delims, int begin=0) -> int
        {
            for (size_t i = begin; i < str.size(); ++i)
            {
                if (strchr(delims, str.at(i)) != NULL)
                    return static_cast<int>(i);
            }
            return -1;
        };
    
    std::vector<std::string> out;
    int idx(0);
    
    while( idx < str.size() )
    {
        int idx2(_find_split_index(str, delims, idx));

        if (idx2 < 0)
            idx2 = str.size();

        if (idx2 - idx > 0)
        {
            if (MAX_NUM > 0 and out.size() >= MAX_NUM)
                idx2 = str.size();
            out.push_back( str.substr(idx, idx2-idx) );
        }

        idx = idx2 + 1;
    }
    return out;
}


std::string format(const char *format, ...)
{
    static const int SIZE = 256 * 256;
    char buffer[SIZE];

    va_list arg;
    va_start(arg, format);
    vsprintf(buffer, format, arg);
    va_end(arg);

    return std::string(buffer);
}


void now(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
    time_t t;
    tm *p_ltm;
    time(&t);
    p_ltm = localtime(&t);

    *year = 1900 + p_ltm->tm_year;
    *month = 1 + p_ltm->tm_mon;
    *day = p_ltm->tm_mday;
    *hour = p_ltm->tm_hour;
    *min = p_ltm->tm_min;
    *sec = p_ltm->tm_sec;
}



std::string time_stamp()
{
    int year, month, day, hour, min, sec;
    now(&year, &month, &day, &hour, &min, &sec);

    time_t t;
    tm *p_ltm;
    time(&t);
    p_ltm = localtime(&t);

    return format(
        "\33[0;34m# %02d/%02d/%04d %02d:%02d:%02d\33[0m] ",
        month, day, year, hour, min, sec);
}



}



}
