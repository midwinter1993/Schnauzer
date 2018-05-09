#include "VectorClock.h"
#include <sstream>

std::string VectorClock::stringfy(const VectorClock &vc) {
    std::stringstream ss;
    for (auto it = vc.clock_.begin(), end = vc.clock_.end(); it != end; ++it) {
        ss << *it << ' ';
    }
    return ss.str();
}

std::string VectorClock::stringfy(const VectorClock *vc) {
    return stringfy(*vc);
}

unsigned long VectorClock::getTick(size_t tid_idx) const {
    assert(tid_idx < clock_.size());
    return clock_[tid_idx];
}

void VectorClock::incTick(size_t tid_idx) {
    assert(tid_idx < MAX_THREAD);
    ++clock_[tid_idx];
}

void VectorClock::merge(const VectorClock &vc) {
    size_t index = 0;
    for (auto it = clock_.begin(), end = clock_.end(); it != end; ++it) {
        *it = std::max(*it, vc.getTick(index));
        ++index;
    }
}

void VectorClock::merge(const VectorClock *vc) {
    merge(*vc);
}

VectorClock& VectorClock::operator=(const VectorClock &vc) {
    if (this != &vc) {
        for (size_t i = 0; i < clock_.size();  ++i) {
            clock_[i] = vc.getTick(i);
        }
    }
    return *this;
}

bool VectorClock::operator==(const VectorClock &other) const {
    for (size_t i = 0; i < VectorClock::MAX_THREAD; ++i) {
        if (getTick(i) != other.getTick(i)) {
            return false;
        }
    }
    return true;
}

bool VectorClock::operator<(const VectorClock &other) const {
    for (size_t i = 0; i < VectorClock::MAX_THREAD; ++i) {
        if (getTick(i) >= other.getTick(i)) {
            return false;
        }
    }
    return true;
}

//
// <= is NOT same with (< || ==)
//
bool VectorClock::operator<=(const VectorClock &other) const {
    for (size_t i = 0; i < VectorClock::MAX_THREAD; ++i) {
        if (getTick(i) > other.getTick(i)) {
            return false;
        }
    }
    return true;
}

//
// > is NOT same with ! <=
//
bool VectorClock::operator>(const VectorClock &other) const {
     for (size_t i = 0; i < VectorClock::MAX_THREAD; ++i) {
        if (getTick(i) <= other.getTick(i)) {
            return false;
        }
    }
    return true;
}

//
// >= is NOT same with ! <
//
bool VectorClock::operator>=(const VectorClock &other) const {
      for (size_t i = 0; i < VectorClock::MAX_THREAD; ++i) {
        if (getTick(i) < other.getTick(i)) {
            return false;
        }
    }
    return true;
}

bool VectorClock::isConcurrentWith(const VectorClock &other) const {
    return !(*this <= other || other <= *this);
}

bool VectorClock::isConcurrentWith(const VectorClock *other) const {
    return isConcurrentWith(*other);
}

void VectorClock::dump() const {
    std::cout << stringfy(*this) << "\n";
}
