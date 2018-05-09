#ifndef __ANALYZE_DEPGRAPH_VECTOR_CLOCK_H__
#define __ANALYZE_DEPGRAPH_VECTOR_CLOCK_H__

#include <cassert>
#include <iostream>
#include <set>
#include <vector>

class VectorClock {
public:
    static std::string stringfy(const VectorClock &vc);
    static std::string stringfy(const VectorClock *vc);

public:
    enum { MAX_THREAD = 50 };

    // static void setNrThread(size_t nr) { MAX_THREAD = nr; }

    VectorClock() : clock_(MAX_THREAD, 0) {}
    VectorClock(const VectorClock &vc) : clock_(vc.clock_) {}
    VectorClock &operator=(const VectorClock &vc);

    void incTick(size_t tid_idx);
    unsigned long getTick(size_t tid_idx) const;

    void merge(const VectorClock &vc);
    void merge(const VectorClock *vc);

    bool isConcurrentWith(const VectorClock &other) const;
    bool isConcurrentWith(const VectorClock *other) const;

    //
    // NOTE: operator overload for happens-before relation
    // NOT for COMPARION
    //
    bool operator==(const VectorClock &vc) const;
    bool operator<(const VectorClock &other) const;
    bool operator<=(const VectorClock &other) const;
    bool operator>(const VectorClock &other) const;
    bool operator>=(const VectorClock &other) const;

    void dump() const;

private:
    std::vector<unsigned long> clock_;
};

//////////////////////////////////////////////
//                ATTENTION
// We we use vector clock as key type for map
// comparion must be the following.
//////////////////////////////////////////////
struct VectorClockCmp {
    bool operator()(const VectorClock &vc_1, const VectorClock &vc_2) const {
        for (size_t i = 0; i < VectorClock::MAX_THREAD; ++i) {
            if (vc_1.getTick(i) < vc_2.getTick(i)) {
                return true;
            } else if (vc_1.getTick(i) > vc_2.getTick(i)) {
                return false;
            }
        }
        return true;
    }
};

#endif /* ifndef VECTORCLOCK_H */
