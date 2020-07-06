#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QTableWidgetItem>

#include "inventory.h"
#include "database.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, QSize inventorySize = QSize(3, 3));
    ~MainWindow() override;

public slots:

    // Запуск игры.
    void beginGame();

    // Возвращение в меню.
    void returnToMenu();

protected:

    // Перехватывает нажатия кнопок мыши на таблицу и вызывает startDragFromTable() или eatApple().
    bool eventFilter(QObject* object, QEvent *event) override;

    // Запускает обновление отображения таблицы.
    void resizeEvent(QResizeEvent *event) override;

    // Принимает event.
    void dragEnterEvent(QDragEnterEvent *event) override;

    // Принимает event и подсвечивает ячейку ивентаря под курсором.
    void dragMoveEvent(QDragMoveEvent *event) override;

    // Изменяет содержимое инвентаря, если dropEvent произошёл, когда курсор был над таблицей.
    void dropEvent(QDropEvent *event) override;

    // Вызывает startDragFromSource(), если ЛКМ нажата на источник.
    void mousePressEvent(QMouseEvent *event) override;


private:
    Ui::MainWindow *ui;

    Database _db;
    Inventory _inv;

    std::function<Item*()> makeItem = makeApple;

    enum Place
    {
        source, inventory
    };


    const QString mimeItTr = "itemTransfer";

    // Инициализация таблицы.
    void setupTable();

    // Вкл/выкл виджеты управления инвентарём.
    void toggleControls();

    // Показать/скрыть меню.
    void toggleMenu();

    // Обновление таблицы.
    void updateTable();

    // Инициализация Drag'n'Drop из источника.
    void startDragFromSource();

    // Инициализация Drag'n'Drop из инвентаря.
    void startDragFromTable(QPoint pos);

    // Удаление одного яблока из ячейки инвентаря на указанных координатах.
    void eatApple(QPoint pos);

    // Перемещение предметов из одной ячейки инвентаря в другую.
    void transferWithinInventory(QTableWidgetItem* source,  QTableWidgetItem* dest);

    // Перемещение предмета из источника в указанную ячейку инвентаря.
    void addFromSource(QTableWidgetItem* dest);

    //Выбор ячейки таблицы инвентаря по координатам относительно главного окна.
    QModelIndex getItemIndex(QPoint pos);

    //Выбор содержимого таблицы инвентаря по координатам относительно главного окна.
    QTableWidgetItem* getItem(QPoint pos);

};

#endif // MAINWINDOW_H
