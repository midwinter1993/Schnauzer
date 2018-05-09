#include "Event.h"

#include <sstream>

void Event::storeEvent(std::ofstream &fout, const Event &evt, bool is_binary) {
    assert(12 <= evt.getType() && evt.getType() <= 22);
    if (is_binary)
        fout.write((char *)(&evt), sizeof(evt));
}

Event Event::loadEvent(std::ifstream &fin, bool is_binary) {
    Event evt;
    if (is_binary)
        fin.read((char *)(&evt), sizeof(Event));
    return evt;
}

size_t Event::load(Event *buf, std::ifstream &fin) {
    fin.read((char *)(buf), sizeof(Event));
    return sizeof(Event);
}

std::string Event::stringfy() const {
    std::stringstream ss;
#ifdef COMPACT_EVENT
    ss << "[Compact event] " << (int)getThread() << ' ';
    ss << typeStr(getType()) << ' ';
    if (isThreadOpEvent())
        ss << (int)getRemoteThread();
    else
        ss << getAddress();
#else
    ss << "[ " << getTimeStamp() << " ]: ";
    ss << "#" << (int)getThread() << ' ';
    ss << typeStr(getType()) << ' ';
    if (isThreadOpEvent())
        ss << (int)getRemoteThread();
    else
        ss << getAddress();
#endif
    return ss.str();
}

const char *Event::typeStr(op_t op) {
    static const char *op_table[] = {"OP_CREATE", "OP_JOIN",      "OP_LD",
                                     "OP_ST",     "OP_TRYLOCK",   "OP_LOCK",
                                     "OP_UNLOCK", "OP_WAIT",      "OP_AWAKE",
                                     "OP_SIGNAL", "OP_BROADCAST", "OP_CALL"};
    assert(OP_CREATE <= op && op <= OP_BROADCAST);
    return op_table[op - OP_CREATE];
}

Event Event::makeVarAccessEvent(instID_t inst,
                                timestamp_t ts,
                                tid_t tid,
                                int ty,
                                address_t addr) {
    return Event{inst, ts, tid, ty, addr, 1, 0};
}

Event Event::makeArrayAccessEvent(instID_t inst,
                                  timestamp_t ts,
                                  tid_t tid,
                                  int ty,
                                  address_t addr,
                                  size_t len) {
    return Event{inst, ts, tid, ty, addr, len, 0};
}

Event Event::makeSyncOpEvent(instID_t inst,
                             timestamp_t ts,
                             tid_t tid,
                             int ty,
                             address_t addr) {
    return Event{inst, ts, tid, ty, addr, 1, 0};
}

Event Event::makeThreadEvent(instID_t inst,
                             timestamp_t ts,
                             tid_t tid,
                             int ty,
                             tid_t remote_tid) {
    return Event{inst, ts, tid, ty, 0, 0, remote_tid};
}

Event Event::makeFuncCallEvent(instID_t inst,
                               timestamp_t ts,
                               tid_t tid) {
    return Event {inst, ts, tid, OP_CALL, 0, 0, 0};
}

bool Event::isAccessEvent() const {
    auto op = getType();
    if (op == Event::OP_LD || op == Event::OP_ST)
        return true;
    return false;
}

bool Event::isVarAccessEvent() const {
    return isAccessEvent() && getLength() == 1;
}

bool Event::isArrayAccessEvent() const {
    return isAccessEvent() && getLength() > 1;
}

bool Event::isSyncOpEvent() const {
    auto op = getType();
    if (op == Event::OP_LOCK || op == Event::OP_UNLOCK ||
        op == Event::OP_TRYLOCK || op == Event::OP_WAIT ||
        op == Event::OP_AWAKE || op == Event::OP_SIGNAL ||
        op == Event::OP_BROADCAST) {
        return true;
    }
    return false;
}

bool Event::isSyncAcqEvent() const {
    auto op = getType();
    if (op == Event::OP_LOCK || op == Event::OP_TRYLOCK ||
        op == Event::OP_AWAKE) {
        return true;
    }
    return false;
}

bool Event::isSyncRelEvent() const {
    auto op = getType();
    if (op == Event::OP_UNLOCK || op == Event::OP_SIGNAL ||
        op == Event::OP_BROADCAST) {
        return true;
    }
    return false;
}

bool Event::isThreadOpEvent() const {
    auto op = getType();
    if (op == Event::OP_CREATE || op == Event::OP_JOIN)
        return true;
    return false;
}

bool Event::isFuncCallEvent() const {
    return getType() == Event::OP_CALL;
}


bool Event::isConflict(const Event &evt) const {
    assert(isAccessEvent() && evt.isAccessEvent());

    address_t start_1 = getAddress();
    address_t end_1 = start_1 + getLength();

    address_t start_2 = evt.getAddress();
    address_t end_2 = start_2 + evt.getLength();

    if (end_1 <= start_2 || end_2 <= start_1) {
        return false;
    }
    return true;
}

bool Event::isConflict(const Event *evt) const {
    return isConflict(*evt);
}

#ifdef COMPACT_EVENT
Event Event::expandArrayAccessEvent(size_t pos) const {
    assert(isAccessEvent());

    Event evt(*this);
    evt.data_.var_info_.var_len_ = 1;
    evt.data_.var_info_.var_addr_ = getAddress() + pos;
    return evt;
}
#else
Event Event::expandArrayAccessEvent(size_t pos) const {
    assert(isAccessEvent());

    Event evt(*this);
    evt.var_len_ = 1;
    evt.var_addr_ = getAddress() + pos;
    return evt;
}
#endif
