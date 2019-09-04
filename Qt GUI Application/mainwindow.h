#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QGridLayout>
#include <QStandardItemModel>
#include "user.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    QLabel _lblID, _lblName, _lblSurname;
    QLineEdit _id, _name, _surname;


    QGridLayout _layA;
    QWidget* _centralWidget;

    QTableView _view;
    QStandardItemModel _model;
    UserList _data;

    void placeWidgets();
    void makeConnects();


    void loadData();
    void loadActiveUsers();
    void onSave();
    void onDelete();
    void onInsert();
    void openExit();
    void openEntrance();
    void openEntranceAndStay();
    void openExitAndStay();
    void closeEntrance();
    void closeExit();
    void emergency();
    void putDataIntoWidgets(const QModelIndex&);

public:
    void onDeleteAll();
    void addUser(const User& s);
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

};
