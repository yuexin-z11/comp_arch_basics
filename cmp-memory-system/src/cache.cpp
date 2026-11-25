///////////////////////////////////////////////////////////////////////////////
// You will need to modify this file to implement part A and, for extra      //
// credit, parts E and F.                                                    //
///////////////////////////////////////////////////////////////////////////////

// cache.cpp
// Defines the functions used to implement the cache.

#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
// You may add any other #include directives you need here, but make sure they
// compile on the reference machine!

///////////////////////////////////////////////////////////////////////////////
//                    EXTERNALLY DEFINED GLOBAL VARIABLES                    //
///////////////////////////////////////////////////////////////////////////////

/**
 * The current clock cycle number.
 * 
 * This can be used as a timestamp for implementing the LRU replacement policy.
 */
extern uint64_t current_cycle;

/**
 * For static way partitioning, the quota of ways in each set that can be
 * assigned to core 0.
 * 
 * The remaining number of ways is the quota for core 1.
 * 
 * This is used to implement extra credit part E.
 */
extern unsigned int SWP_CORE0_WAYS;

///////////////////////////////////////////////////////////////////////////////
//                           FUNCTION DEFINITIONS                            //
///////////////////////////////////////////////////////////////////////////////

// As described in cache.h, you are free to deviate from the suggested
// implementation as you see fit.

// The only restriction is that you must not remove cache_print_stats() or
// modify its output format, since its output will be used for grading.

/**
 * Allocate and initialize a cache.
 * 
 * This is intended to be implemented in part A.
 *
 * @param size The size of the cache in bytes.
 * @param associativity The associativity of the cache.
 * @param line_size The size of a cache line in bytes.
 * @param replacement_policy The replacement policy of the cache.
 * @return A pointer to the cache.
 */
Cache *cache_new(uint64_t size, uint64_t associativity, uint64_t line_size,
                 ReplacementPolicy replacement_policy)
{
    // TODO: Allocate memory to the data structures and initialize the required
    //       fields. (You might want to use calloc() for this.)
     // one zeroed Cache struct, so all stats start at 0
    Cache *c = (Cache *)calloc(1, sizeof(Cache));

    c->num_ways = associativity;
    c->replacement_policy = replacement_policy;

    // sets = total bytes / (bytes per line * lines per set)
    c->num_sets = size / (line_size * associativity);

    // zeroed sets, so every line starts invalid
    c->sets = (CacheSet *)calloc(c->num_sets, sizeof(CacheSet));

    return c;
}

/**
 * Access the cache at the given address.
 * 
 * Also update the cache statistics accordingly.
 * 
 * This is intended to be implemented in part A.
 * 
 * @param c The cache to access.
 * @param line_addr The address of the cache line to access (in units of the
 *                  cache line size, i.e., excluding the line offset bits).
 * @param is_write Whether this access is a write.
 * @param core_id The CPU core ID that requested this access.
 * @return Whether the cache access was a hit or a miss.
 */
CacheResult cache_access(Cache *c, uint64_t line_addr, bool is_write,
                         unsigned int core_id)
{
    // low bits of line_addr pick the set, the rest is the tag
    uint64_t set_index = line_addr % c->num_sets;
    uint64_t tag = line_addr / c->num_sets;
    CacheSet *set = &c->sets[set_index];

    if (is_write)
        c->stat_write_access++;
    else
        c->stat_read_access++;

    for (unsigned int way = 0; way < c->num_ways; way++)
    {
        CacheLine *line = &set->lines[way];

        // hit: valid line with a matching tag
        if (line->valid && line->tag == tag)
        {
            // touch for LRU (timestamp = now)
            line->last_access_time = current_cycle;
            if (is_write)
                line->dirty = true;
            return HIT;
        }
    }

    // miss: count it against the right stat
    if (is_write)
        c->stat_write_miss++;
    else
        c->stat_read_miss++;
    return MISS;
}

/**
 * Install the cache line with the given address.
 * 
 * Also update the cache statistics accordingly.
 * 
 * This is intended to be implemented in part A.
 * 
 * @param c The cache to install the line into.
 * @param line_addr The address of the cache line to install (in units of the
 *                  cache line size, i.e., excluding the line offset bits).
 * @param is_write Whether this install is triggered by a write.
 * @param core_id The CPU core ID that requested this access.
 */
void cache_install(Cache *c, uint64_t line_addr, bool is_write,
                   unsigned int core_id)
{
    uint64_t set_index = line_addr % c->num_sets;
    uint64_t tag = line_addr / c->num_sets;

    unsigned int way = cache_find_victim(c, set_index, core_id);
    CacheLine *victim = &c->sets[set_index].lines[way];

    // keep a copy of the old line so the caller can write it back
    c->last_evicted_line = *victim;

    // only a valid, dirty line needs a writeback
    if (victim->valid && victim->dirty)
        c->stat_dirty_evicts++;

    // fill the way with the new line
    victim->valid = true;
    victim->dirty = is_write;
    victim->tag = tag;
    victim->core_id = core_id;
    victim->last_access_time = current_cycle;
}

/**
 * Find which way in a given cache set to replace when a new cache line needs
 * to be installed. This should be chosen according to the cache's replacement
 * policy.
 * 
 * The returned victim can be valid (non-empty), in which case the calling
 * function is responsible for evicting the cache line from that victim way.
 * 
 * This is intended to be initially implemented in part A and, for extra
 * credit, extended in parts E and F.
 * 
 * @param c The cache to search.
 * @param set_index The index of the cache set to search.
 * @param core_id The CPU core ID that requested this access.
 * @return The index of the victim way.
 */
unsigned int cache_find_victim(Cache *c, unsigned int set_index,
                               unsigned int core_id)
{
    // an empty way is always the best victim
    for (unsigned int way = 0; way < c->num_ways; way++)
    {
        if (!c->sets[set_index].lines[way].valid)
            return way;
    }

    if (c->replacement_policy == RANDOM)
        return rand() % c->num_ways;

    if (c->replacement_policy == SWP)
    {
        CacheSet *set = &c->sets[set_index];

        // core 0 gets SWP_CORE0_WAYS ways, core 1 gets the rest
        unsigned int quota = (core_id == 0) ? SWP_CORE0_WAYS : (c->num_ways - SWP_CORE0_WAYS);

        // how many ways this core already holds in the set
        unsigned int owned = 0;
        for (unsigned int way = 0; way < c->num_ways; way++)
        {
            if (set->lines[way].core_id == core_id)
                owned++;
        }

        // under quota -> evict from the other core, else replace our own
        unsigned int victim_core = (owned < quota) ? (1 - core_id) : core_id;

        // LRU among the victim core's ways
        int victim = -1;
        for (unsigned int way = 0; way < c->num_ways; way++)
        {
            CacheLine *line = &set->lines[way];
            if (line->core_id != victim_core)
                continue;
            if (victim < 0 || line->last_access_time < set->lines[victim].last_access_time)
                victim = way;
        }
        if (victim >= 0)
            return victim;
        // no line of that core in the set: fall back to plain LRU below
    }

    // LRU: the way touched longest ago loses
    unsigned int victim = 0;
    for (unsigned int way = 1; way < c->num_ways; way++)
    {
        if (c->sets[set_index].lines[way].last_access_time < c->sets[set_index].lines[victim].last_access_time)
            victim = way;
    }
    return victim;

    // TODO(lab4-F): DWP - victimize by dynamic per-core way targets
    // TODO: Find a victim way in the given cache set according to the cache's
    //       replacement policy.
    // TODO: In part A, implement the LRU and random replacement policies.
    // TODO: In part E, for extra credit, implement static way partitioning.
    // TODO: In part F, for extra credit, implement dynamic way partitioning.
}

/**
 * Print the statistics of the given cache.
 * 
 * This is implemented for you. You must not modify its output format.
 * 
 * @param c The cache to print the statistics of.
 * @param label A label for the cache, which is used as a prefix for each
 *              statistic.
 */
void cache_print_stats(Cache *c, const char *header)
{
    double read_miss_percent = 0.0;
    double write_miss_percent = 0.0;

    if (c->stat_read_access)
    {
        read_miss_percent = 100.0 * (double)(c->stat_read_miss) /
                            (double)(c->stat_read_access);
    }

    if (c->stat_write_access)
    {
        write_miss_percent = 100.0 * (double)(c->stat_write_miss) /
                             (double)(c->stat_write_access);
    }

    printf("\n");
    printf("%s_READ_ACCESS     \t\t : %10llu\n", header, c->stat_read_access);
    printf("%s_WRITE_ACCESS    \t\t : %10llu\n", header, c->stat_write_access);
    printf("%s_READ_MISS       \t\t : %10llu\n", header, c->stat_read_miss);
    printf("%s_WRITE_MISS      \t\t : %10llu\n", header, c->stat_write_miss);
    printf("%s_READ_MISS_PERC  \t\t : %10.3f\n", header, read_miss_percent);
    printf("%s_WRITE_MISS_PERC \t\t : %10.3f\n", header, write_miss_percent);
    printf("%s_DIRTY_EVICTS    \t\t : %10llu\n", header, c->stat_dirty_evicts);
}
