#ifndef INVENTORY_H
#define INVENTORY_H

#include <QMultiHash>
#include <QPoint>

#include "item.h"
#include "database.h"

class Inventory
{
public:
    Inventory(QSize size, Database* db = nullptr) : _rows(size.height()), _columns(size.width()), _db(db) {}
    ~Inventory() { deleteItems(); }

    // Добавляет предмет в инвентарь.
    void addItem(const QPoint pos, Item* i);

    // Перемещает все предметы из одной ячейки в другую.
    void transferStack(const QPoint source, const QPoint dest);

    // Убирает один предмет из ячейки инвентаря и возвращает true, или ничего не делает и возвращает false, если ячейка пуста.
    bool removeItem(const QPoint pos);

    // Возвращает инвентарь и базу данных в начальное состояние.
    void reset();

    // Удаляет все предметы в инвентаре.
    void deleteItems();

private:
    int _rows, _columns;
    QMultiHash<QPoint, Item*> _table;
    Database* _db = nullptr;
};


inline void Inventory::addItem(const QPoint dest, Item *i)
{
    assert(dest.y() < _rows);
    assert(dest.x() < _columns);
    assert(!i->isNull());

    if (_table.count(dest))
        assert(i->type == _table.value(dest)->type);

    _table.insert(dest, i);

    if (_db)
        _db->setItems(dest, _table.count(dest), *i);
}

inline void Inventory::transferStack(const QPoint source, const QPoint dest)
{
    assert(source.y() < _rows);
    assert(source.x() < _columns);
    assert(dest.y() < _rows);
    assert(dest.x() < _columns);

    if (_table.count(source) == 0 || source == dest)
        return;

    foreach(auto item, _table.values(source))
        addItem(dest, item);

    _table.remove(source);

    if (_db) {
        _db->setItems(source, 0);
        _db->setItems(dest, _table.count(dest), *_table.value(dest));
    }
}

inline bool Inventory::removeItem(const QPoint pos)
{
    auto areRemoved = _table.remove(pos, _table.value(pos));

    if (_db && areRemoved)
        if (_table.count(pos))
            _db->setItems(pos, _table.count(pos), *_table.value(pos));
        else
            _db->setItems(pos, 0);

    return areRemoved;
}

inline void Inventory::reset()
{
    deleteItems();
    _table.clear();
    if (_db)
        _db->reset();
}

inline void Inventory::deleteItems()
{
    foreach(auto item, _table) delete item;
}


#endif // INVENTORY_H
