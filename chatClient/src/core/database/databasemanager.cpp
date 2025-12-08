#include "databasemanager.h"
#include <QCoreApplication>
#include <QDebug>

DatabaseManager::DatabaseManager()
    : m_isOpen(false)
{
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initDatabase(const QString& dbPath)
{
    if (m_isOpen) {
        return true;
    }

    // 使用应用程序所在目录
    QString dataPath = QCoreApplication::applicationDirPath();
    QString fullPath = dataPath + "/" + dbPath;

    // 检查是否已存在默认连接，避免重复添加
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        m_database = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    } else {
        m_database = QSqlDatabase::addDatabase("QSQLITE");
    }
    m_database.setDatabaseName(fullPath);

    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }

    m_isOpen = true;
    qDebug() << "Database opened at:" << fullPath;

    return createTables();
}

void DatabaseManager::closeDatabase()
{
    if (m_isOpen && m_database.isOpen()) {
        m_database.close();
        m_isOpen = false;
    }
}

QSqlDatabase& DatabaseManager::database()
{
    return m_database;
}

bool DatabaseManager::isOpen() const
{
    return m_isOpen;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // 用户基本信息表
    QString createUserInfo = R"(
        CREATE TABLE IF NOT EXISTS user_info (
            ssid INTEGER PRIMARY KEY,
            username TEXT,
            avatar_path TEXT
        )
    )";

    if (!query.exec(createUserInfo)) {
        qWarning() << "Failed to create user_info table:" << query.lastError().text();
        return false;
    }

    // 好友关系表
    QString createFriendship = R"(
        CREATE TABLE IF NOT EXISTS friendship (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ssid INTEGER,
            friend_ssid INTEGER,
            grouping_name TEXT,
            friend_name TEXT,
            status TEXT,
            friend_type INTEGER,
            FOREIGN KEY(friend_ssid) REFERENCES user_info(ssid)
        )
    )";

    if (!query.exec(createFriendship)) {
        qWarning() << "Failed to create friendship table:" << query.lastError().text();
        return false;
    }

    // 群组信息表
    QString createGroupInfo = R"(
        CREATE TABLE IF NOT EXISTS group_info (
            ssid_group INTEGER PRIMARY KEY,
            group_name TEXT,
            avatar_path TEXT,
            create_ssid INTEGER
        )
    )";

    if (!query.exec(createGroupInfo)) {
        qWarning() << "Failed to create group_info table:" << query.lastError().text();
        return false;
    }

    // 群成员表
    QString createGroupMember = R"(
        CREATE TABLE IF NOT EXISTS group_member (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ssid_group INTEGER,
            ssid_member INTEGER,
            FOREIGN KEY(ssid_group) REFERENCES group_info(ssid_group),
            FOREIGN KEY(ssid_member) REFERENCES user_info(ssid)
        )
    )";

    if (!query.exec(createGroupMember)) {
        qWarning() << "Failed to create group_member table:" << query.lastError().text();
        return false;
    }

    qDebug() << "All tables created successfully";
    return true;
}
