#ifndef USER_H
#define USER_H
#include <QString>
#include <QList>
#include <QTextStream>


// Struct that contains user data in QString format.
// It also overloads '<<' and '>>' operators, for them to simplify use of the struct information as character streams. (e.g. To easily write struct data to files.)
struct User
{
    QString _id;
    QString _name;
    QString _surname;



    User() = default;

    User(QString id, QString name, QString surname)
        :   _id(id)
        ,   _name(name)
        ,   _surname(surname)
    {

    }

    friend QTextStream &operator <<(QTextStream &out, const User  &s)
    {
        out << s._id << ' ' << s._name << ' ' << s._surname << ' ' << '\n';
        return out;
    }

    friend QTextStream &operator >>(QTextStream &in, User &s)
    {
        in >> s._id >> s._name>> s._surname;
        return in;
    }

};


using UserList = QList<User>;

#endif
