#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <functional>

#include <QList>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QSound>

#include <item.h>


MainWindow::MainWindow(QWidget *parent,  QSize inventorySize) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _db(inventorySize),
    _inv(inventorySize, &_db)
{
    ui->setupUi(this);

    setupTable();

    auto itemSample = makeItem();
    ui->appleSource->setPixmap(itemSample->icon);
    delete itemSample;

    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(beginGame()));
    connect(ui->menuButton, SIGNAL(clicked()), this, SLOT(returnToMenu()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::beginGame()
{
    toggleMenu();
    toggleControls();
    _inv.reset();
    updateTable();
}

void MainWindow::returnToMenu()
{
    toggleMenu();
    toggleControls();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && event->type() && ui->inventoryTable->isEnabled())
    {
        auto e = static_cast<QMouseEvent*>(event);

        switch (e->button()) {
        case Qt::LeftButton:
                startDragFromTable(e->pos());
            break;
        case Qt::RightButton:
            eatApple(e->pos());
        }
       return true;
    }
    else
        return QMainWindow::eventFilter(object, event);
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    updateTable();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}


void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();

    ui->inventoryTable->setCurrentIndex(getItemIndex(event->pos()));
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto inv = ui->inventoryTable;

    if (inv->geometry().contains(event->pos())) {
        auto mime = event->mimeData();
        auto data = mime->data(mimeItTr);
        QDataStream ds(&data, QIODevice::ReadOnly);

        int nItems;
        ds >> nItems;
        int source;
        ds >> source;

        switch (source) {

        case Place::inventory:
            int row, column;
            ds >> row;
            ds >> column;

            transferWithinInventory(inv->item(row, column), getItem(event->pos()));
            break;

        case Place::source:
            addFromSource(getItem(event->pos()));
            break;
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        if (
                ui->appleSource->isEnabled()
                && childAt(event->pos()) == ui->appleSource)
            startDragFromSource();
}

void MainWindow::setupTable()
{
    auto inv = ui->inventoryTable;

    for (int i = 0; i < inv->rowCount(); ++i)
        for (int j = 0; j < inv->columnCount(); ++j)
            inv->setItem(i, j, new QTableWidgetItem());

    inv->viewport()->installEventFilter(this);

    updateTable();
}


void MainWindow::toggleControls()
{
    const bool status = ui->inventoryTable->isEnabled();

    QList<QWidget*> widgetsToEnable = {
        ui->inventoryTable,
        ui->menuButton,
        ui->appleSource
    };

    foreach(auto w, widgetsToEnable){
        w->setEnabled(!status);
    }

    ui->inventoryTable->setFocus();
    ui->inventoryTable->setCurrentIndex(QModelIndex());
}

void MainWindow::toggleMenu()
{
    const auto& curGeom = ui->menu->maximumSize();
    auto a = new QPropertyAnimation(ui->menu, "maximumSize");
    a->setDuration(1000);
    a->setStartValue(curGeom);

    const auto newGeom = curGeom.height() == 0 ?
                QSize(curGeom.width(), ui->menu->baseSize().height()) :
                QSize(curGeom.width(), 0);

    a->setEndValue(newGeom);
    a->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::updateTable()
{
    auto inv = ui->inventoryTable;
    const auto nRows = inv->rowCount();
    const auto nCols = inv->columnCount();

    for (int i = 0; i < nRows; ++i)
        inv->setRowHeight(i, inv->height()/3);

    for (int i = 0; i < nCols; ++i)
        inv->setColumnWidth(i, inv->width()/3);

    for (int i = 0; i < nRows; ++i)
        for (int j = 0; j < nCols; ++j)
        {
            auto item = inv->item(i, j);
            const auto [type, nItems] = _db.getCellData(QPoint(i, j));

            if (nItems == 0)
            {
                item->setData(Qt::DecorationRole, QVariant(QPixmap()));
                continue;
            }

            const auto iconPath = _db.getIcon(type);
            auto p = QPixmap(iconPath).scaled(inv->columnWidth(0), inv->rowHeight(0));
            QPainter painter(&p);
            const QRect place(p.width()*.8, p.height()*.9, p.width()*.1, p.height()*.1);
            painter.drawText(place, Qt::AlignRight, QString::number(nItems));

            inv->item(i, j)->setData(Qt::DecorationRole, QVariant(p));
        }
}

void MainWindow::startDragFromSource()
{
    const auto drag = new QDrag(this);
    const auto data = new QMimeData();
    const auto picSize = ui->appleSource->size();

    QByteArray info;
    QDataStream s(&info, QIODevice::WriteOnly);

    const int nDraggedIntems = 1;
    s << nDraggedIntems;
    s << Place::source;

    data->setData(mimeItTr, info);

    drag->setMimeData(data);
    drag->setPixmap(ui->appleSource->pixmap()->scaled(picSize));
    drag->setHotSpot(QPoint(picSize.width()/2, picSize.height()/2));

    drag->exec();
}

void MainWindow::startDragFromTable(QPoint pos)
{
    const auto i = getItemIndex(pos);
    if (i.data(Qt::DecorationRole).value<QPixmap>().isNull())
        return;

    const auto drag = new QDrag(this);
    const auto data = new QMimeData();
    const QSize picSize(
                ui->inventoryTable->columnWidth(0),
                ui->inventoryTable->rowHeight(0));

    QByteArray info;
    QDataStream s(&info, QIODevice::WriteOnly);
    const int nDraggedIntems = 3;

    s << nDraggedIntems;
    s << Place::inventory;
    s << i.row();
    s << i.column();

    data->setData(mimeItTr, info);

    drag->setMimeData(data);
    drag->setPixmap(
                i.data(Qt::DecorationRole).value<QPixmap>().scaled(picSize)
                );
    drag->setHotSpot(QPoint(picSize.width()/2, picSize.height()/2));

    drag->exec();
}

void MainWindow::eatApple(QPoint pos)
{
    const auto item = getItem(pos);

    if (_inv.removeItem(QPoint(item->row(), item->column())))
        QSound::play("://sounds/Apple-bite-sound-effect.wav");

    updateTable();
}

void MainWindow::transferWithinInventory(QTableWidgetItem *source, QTableWidgetItem *dest)
{
    _inv.transferStack(
                QPoint(source->row(), source->column()),
                QPoint(dest->row(), dest->column())
                );
    updateTable();
}

void MainWindow::addFromSource(QTableWidgetItem *dest)
{
    _inv.addItem(QPoint(dest->row(), dest->column()), makeItem());
    updateTable();
}

QModelIndex MainWindow::getItemIndex(QPoint pos)
{
    const auto& geom = ui->inventoryTable->geometry();
    const auto posInTable = pos - geom.topLeft();

    return ui->inventoryTable->indexAt(posInTable);
}

QTableWidgetItem *MainWindow::getItem(QPoint pos)
{
    const auto& geom = ui->inventoryTable->geometry();
    const auto posInTable = pos - geom.topLeft();

    return ui->inventoryTable->itemAt(posInTable);
}
