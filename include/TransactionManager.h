#pragma once

#include <unordered_map>
#include <mutex>
#include <atomic>

#include "WAL.h"
#include "BufferPool.h"
#include "Transaction.h"

class TransactionManager
{
public:
    TransactionManager(WAL& wal, BufferPool& bufferPool);

    uint32_t begin();
    void commit(uint32_t txnID);
    void abort(uint32_t txnID);

private:
    std::atomic<uint32_t> m_NextTxnID{ 1 };
    std::unordered_map<uint32_t, TxnState> m_Transactions;

    std::mutex m_Mutex;

    WAL& m_Wal;
    BufferPool& m_BufferPool;
};