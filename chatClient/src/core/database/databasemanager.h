#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();

    bool initDatabase(const QString& dbPath = "chatclient.db");
    void closeDatabase();

    QSqlDatabase& database();
    bool isOpen() const;

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();

    QSqlDatabase m_database;
    bool m_isOpen;
};

#endif // DATABASEMANAGER_H
