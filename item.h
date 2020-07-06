#ifndef ITEM_H
#define ITEM_H

#include <QString>

struct Item
{
    const QString type;
    const QString icon;

    bool isNull() const {
        return type.isEmpty() || icon.isEmpty();
    }
};

inline Item* makeApple()
{
    return new Item{ "Apple", ":/img/apple.png" };
}

#endif // ITEM_H
