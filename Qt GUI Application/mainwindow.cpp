#include "mainwindow.h"
#include "comunication.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _lblID("ID:")
    , _lblName("Name:")
    , _lblSurname("Surname:")
    , _id()
    , _name()
    , _surname()
    , _layA()
    , _centralWidget(new QWidget(this))

{
    this->setFixedSize(390,500);
    _id.setInputMask("9999999");
    _name.setInputMask("aaaaaaaaaaaaaaa");
    _surname.setInputMask("aaaaaaaaaaaaaaaaaaaa");

    _view.setModel(&_model);
    _view.setSelectionMode(QAbstractItemView::SingleSelection);
    _view.setSelectionBehavior(QAbstractItemView::SelectRows);
    _view.setEditTriggers(QAbstractItemView::NoEditTriggers);
    _model.setHorizontalHeaderLabels({"ID", "Name", "Surname"});


    placeWidgets();
    makeConnects();
}

MainWindow::~MainWindow()
{
}

void MainWindow::placeWidgets()
{

    auto menu = menuBar()->addMenu("Menu");
    menu->addAction("Load All Users", this, &MainWindow::loadData);
    menu->addAction("Load Active Users", this, &MainWindow::loadActiveUsers);
    menu->addAction("Save", this, &MainWindow::onSave);
    menu->addAction("Insert", this, &MainWindow::onInsert);
    menu->addAction("Delete", this, &MainWindow::onDelete);

    auto menu2 = menuBar()->addMenu("Manual control");
    menu2->addAction("Open entrance", this, &MainWindow::openEntrance);
    menu2->addAction("Open exit", this, &MainWindow::openExit);
    menu2->addAction("Open entrance and stay", this, &MainWindow::openEntranceAndStay);
    menu2->addAction("Open exit and stay", this, &MainWindow::openExitAndStay);
    menu2->addAction("Close entrance", this, &MainWindow::closeEntrance);
    menu2->addAction("Close exit", this, &MainWindow::closeExit);
    menu2->addAction("Emergency", this, &MainWindow::emergency);

    _layA.addWidget(&_lblID, 0, 0);
    _layA.addWidget(&_id, 0, 1);

    _layA.addWidget(&_lblName, 1, 0);
    _layA.addWidget(&_name, 1, 1);
    _layA.addWidget(&_lblSurname, 2, 0);
    _layA.addWidget(&_surname, 2, 1);

    _layA.addWidget(&_view, 3, 0, 1, 4);


    _centralWidget->setLayout(&_layA);
    setCentralWidget(_centralWidget);


}


void MainWindow::makeConnects()
{
    //double click on grid to transfer data to widgets
    connect(&_view, &QTableView::activated, this, &MainWindow::putDataIntoWidgets);
}



void MainWindow::onSave()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save users", "D:", "Text files(*.txt)");
    QFile file(fileName);
    if(!file.open(QFile::WriteOnly))
    {
        QMessageBox::information(this, "Information", "No file selected...", QMessageBox::Ok);
        return;
    }


    QTextStream stream(&file);
    for(auto user : _data)
    {
        stream << user;
    }
}

void MainWindow::onDelete()
{
    QItemSelectionModel *select = _view.selectionModel();

    if(select->hasSelection())
    {
         auto rowsForRemoving = select->selectedRows();
         for(auto row : rowsForRemoving)
         {
            _data.erase(_data.begin() + row.row());
         }

         for(auto row : rowsForRemoving)
         {
            _model.removeRow(row.row());
         }
    }
    sendData('k', _id.text(), _name.text(), _surname.text());
}

void MainWindow::onInsert()
{
    if(_id.text().length() == 7){
        if ((_name.text().length()>=3) && (_surname.text().length() >= 3)){
            addUser(User(_id.text(), _name.text(), _surname.text()));
        }
        else {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","Name and Surname must be at least 3 characters long!");
            messageBox.setFixedSize(500,200);
            return;
        }
    }
    else {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","ID must be 7 digits long!");
        messageBox.setFixedSize(500,200);
        return;
    }
    sendData('j', _id.text(), _name.text(), _surname.text());
}

void MainWindow::addUser(const User& s)
{
    _data.push_back(s);

    QList<QStandardItem*> itemsToAdd;
    itemsToAdd.push_back(new QStandardItem(s._id));
    itemsToAdd.push_back(new QStandardItem(s._name));
    itemsToAdd.push_back(new QStandardItem(s._surname));

    _model.appendRow(itemsToAdd);

}


void MainWindow::putDataIntoWidgets(const QModelIndex& index)
{
    auto user = _data.at(index.row());
    _id.setText(user._id);
    _name.setText(user._name);
    _surname.setText(user._surname);

}

void MainWindow::onDeleteAll()
{
     _model.removeRows(0, _model.rowCount());
     _data.clear();
}

void MainWindow::openEntrance()
{
    sendHeader('i');
}

void MainWindow::openExit()
{
    sendHeader('h');
}

void MainWindow::openEntranceAndStay()
{
    sendHeader('g');
}

void MainWindow::openExitAndStay()
{
    sendHeader('f');
}

void MainWindow::closeEntrance()
{
    sendHeader('e');
}

void MainWindow::closeExit()
{
    sendHeader('d');
}

void MainWindow::emergency()
{
    sendHeader('c');
}

void MainWindow::loadActiveUsers()
{
    receive('b', this);
}

void MainWindow::loadData()
{
    receive('a', this);
}
