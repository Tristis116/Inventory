#ifndef DATABASE_H
#define DATABASE_H


#include <QSqlDatabase>
#include <QSqlQuery>
#include <QPoint>
#include <QVariant>
#include <QSize>

#include <QSqlError>
#include <QDebug>

#include "item.h"

class Database
{
public:
    Database(QSize size);

    // Устанавливает заданное количество предметов в указанную ячейку инвентаря.
    void setItems(QPoint pos, int n = 0, const Item& item = Item());

    // Возвращает информацию о содержимом ячейки (тип, количество).
    std::pair<QString, int> getCellData(QPoint pos);

    // Возвращает путь к изображению указанного предмета.
    QString getIcon(QString type);

    // Возвращает базу данных в начальное состояние.
    void reset();

private:

    // Добавляет предмет нового типа в таблицу Items, если ещё нет записи о нём.
    void addItemType(const Item& item);

    // Создаёт таблицы.
    void createTables();

    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE");

    QSize _size;
};


inline Database::Database(QSize size) : _size(size)
{
    if (!_db.open())
         assert(false);

    createTables();
}

inline void Database::setItems(QPoint pos, int n, const Item &item)
{
    const auto where = QString(" WHERE row = '%1' AND column = '%2'").arg(pos.y()).arg(pos.x());

    if (item.isNull() || n == 0)
        _db.exec("UPDATE Inventory SET itemType = NULL, nItems = 0" + where);
    else{
        addItemType(item);
        _db.exec(QString(
                     " UPDATE Inventory SET itemType = '%1', nItems = '%2'")
                 .arg(item.type).arg(n) + where);
    }
}

inline std::pair<QString, int> Database::getCellData(QPoint pos)
{
    const auto where = QString(" WHERE row = %1 AND column = %2").arg(pos.y()).arg(pos.x());

    auto result = _db.exec("SELECT itemType, nItems FROM Inventory" + where);
    result.next();

    return { result.value(0).value<QString>(), result.value(1).value<int>() };
}

inline QString Database::getIcon(QString type)
{
    auto result =_db.exec(QString("SELECT icon FROM Items WHERE type = '%1'").arg(type));
    result.next();
    return result.value(0).value<QString>();
}

inline void Database::reset()
{
    _db.exec("DELETE FROM Items");
    _db.exec("DELETE FROM Inventory");

    for (int i = 0; i < _size.height(); ++i)
        for (int j = 0; j < _size.width(); ++j)
            _db.exec(QString(
                         " INSERT INTO Inventory VALUES (%1, %2, NULL, 0)"
                         ).arg(i).arg(j));
}


inline void Database::addItemType(const Item &item)
{
    _db.exec(QString("INSERT OR IGNORE INTO Items VALUES ('%1', '%2') ")
             .arg(item.type).arg(item.icon));
}

inline void Database::createTables()
{
    _db.exec(
                "CREATE TABLE Inventory ("
                "row       INTEGER     NOT NULL,"
                "column    INTEGER     NOT NULL,"
                "itemType  TEXT,"
                "nItems    INTEGER     NOT NULL    DEFAULT 0 ,"
                "PRIMARY KEY (row, column)"
                ")"
                );


    for (int i = 0; i < _size.height(); ++i)
        for (int j = 0; j < _size.width(); ++j)
            _db.exec(QString(
                         " INSERT INTO Inventory VALUES (%1, %2, NULL, 0)"
                         ).arg(i).arg(j));

    _db.exec(
                "CREATE TABLE Items ("
                "type   TEXT    PRIMARY KEY     NOT NULL  UNIQUE,"
                "icon   TEXT"
                ")"
                );
}


inline uint qHash (const QPoint & key)
{
    return qHash (QPair<int,int>(key.x(), key.y()) );
}

#endif // DATABASE_H


