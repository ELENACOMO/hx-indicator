#ifndef TransactionDB_H
#define TransactionDB_H

#include <QObject>

#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/slice.h>

class TransactionStruct;
class TransactionTypeId;
typedef QVector<TransactionTypeId>  TransactionTypeIds;

class TransactionDB
{
public:
    TransactionDB();
    ~TransactionDB();

    bool init();

    void insertTransactionStruct(QString _transactionId, TransactionStruct _struct);
    TransactionStruct getTransactionStruct(QString _transactionId);
    QVector<TransactionStruct>  lookupTransactionStruct(QString _address, int _type = 0);   // 查找地址(不一定是本钱包账户)相关的交易

    void insertAccountTransactionTypeIds(QString _accountAddress, TransactionTypeIds _transactionTypeIds);
    void addAccountTransactionId(QString _accountAddress, TransactionTypeId _transactionTypeId);
    TransactionTypeIds getAccountTransactionTypeIds(QString _accountAddress);
    TransactionTypeIds getAccountTransactionTypeIdsByType(QString _accountAddress, int _type);

    bool writeToDB(leveldb::DB* _db, QString _key, QByteArray _value);
    QByteArray readFromDB(leveldb::DB* _db, QString _key);

    QStringList getKeys(leveldb::DB* _db);

private:
    leveldb::DB* m_transactionStructDB;
    leveldb::DB* m_accountTransactionIdsDB;
};

#endif // TransactionDB_H