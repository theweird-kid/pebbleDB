#pragma once

#include <cstdint>

enum class TxnState {
    ACTIVE,
    COMMITTED,
    ABORTED
};

struct Transaction {
    uint32_t txnID;
    TxnState state;
};