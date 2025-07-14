#include "TransactionManager.h"

TransactionManager::TransactionManager(WAL& wal, BufferPool& bp)
    : m_Wal(wal), m_BufferPool(bp)
{}

uint32_t TransactionManager::begin() {
    std::lock_guard<std::mutex> lock(m_Mutex);

    uint32_t txnID = m_NextTxnID++;
    m_Transactions[txnID] = TxnState::ACTIVE;

    m_Wal.logRecord(LogType::BEGIN, txnID, 0, nullptr, 0);

    return txnID;
}

void TransactionManager::commit(uint32_t txnID) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Transactions.find(txnID) == m_Transactions.end() ||
        m_Transactions[txnID] != TxnState::ACTIVE) {
        throw std::runtime_error("Transaction not active or not found: " + std::to_string(txnID));
    }

    m_Wal.logRecord(LogType::COMMIT, txnID, 0, nullptr, 0);
    m_Wal.flush();
    m_BufferPool.flushAll();

    m_Transactions[txnID] = TxnState::COMMITTED;
}

void TransactionManager::abort(uint32_t txnID) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Transactions.find(txnID) == m_Transactions.end() ||
        m_Transactions[txnID] != TxnState::ACTIVE) {
        throw std::runtime_error("Transaction not active or not found: " + std::to_string(txnID));
    }

    m_Wal.logRecord(LogType::ABORT, txnID, 0, nullptr, 0);
    m_Wal.flush();

    m_Transactions[txnID] = TxnState::ABORTED;
    
}