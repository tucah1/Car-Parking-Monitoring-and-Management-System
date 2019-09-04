#include "comunication.h"
#include "mainwindow.h"
#include "user.h"

#include <QMessageBox>

char readBuffer[BUFFER_LENGTH];
char writeBuffer[WRITE_BUFFER_LENGTH];

void resetBuffer(char * buff, int len)
{
    int i = 0;
    for (; i < len; i++)
        buff[i] = 0;
}

int getBufferSize(char arr[]) {
    int i = 0;
    while (arr[i] != '>') {
        i++;
    }
    return i;
}

void receive(char header, MainWindow* mw)
{
    struct sockaddr_in srv;
    int fd;
    int counter = 0, n = 0, total = 0, size = 0;
    int counterOfZeros = 0;
    writeBuffer[0] = header;
    writeBuffer[1] = '>';
    void * p;

    QString id = "";
    QString name = "";
    QString surname = "";

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr(ADDRESS);

    if (connect(fd, (sockaddr *) &srv, sizeof(srv)) < 0)
    {
        perror("connect");
    }

    if (write(fd, writeBuffer, 2) < 0 ) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","Write error!");
            messageBox.setFixedSize(500,200);
    }

    size = sizeof(readBuffer);
    n = 0;
    p = readBuffer;
    do {
        if ((n = read(fd, p, size)) < 0) {
            QMessageBox messageBox;
            messageBox.critical(0,"Error","Read error!");
            messageBox.setFixedSize(500,200);
        }
        p = p + n;
        size -= n;
        total += n;
    } while (n > 0);


    mw->onDeleteAll();

    for (int i = 0; i < BUFFER_LENGTH; i++)
    {
        if (readBuffer[0] == '/')
                break;

        if (readBuffer[i] == 0)
        {
            counterOfZeros++;
        }
        else {
            counterOfZeros = 0;
        }

        if (counterOfZeros == 8)
            break;

        if(readBuffer[i] != ' ')
            {
            if (readBuffer[i] == '\n')
                continue;

            switch(counter)
                {
                    case 0:
                        id = id + readBuffer[i];
                        break;
                    case 1:
                        name = name + readBuffer[i];
                        break;
                    case 2:
                        surname = surname + readBuffer[i];
                        break;
                }
            }
            else {
                counter++;
                if (counter >= 3)
                {
                    mw->addUser(User(id, name, surname));
                    id="";
                    name="";
                    surname="";
                    counter = 0;
                }
            }
    }

    resetBuffer(writeBuffer, WRITE_BUFFER_LENGTH);
    resetBuffer(readBuffer, BUFFER_LENGTH);
}

void sendHeader(char header)
{
    struct sockaddr_in srv;
    int fd;
    writeBuffer[0] = header;
    writeBuffer[1] = '>';

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr(ADDRESS);

    if (connect(fd, (sockaddr *) &srv, sizeof(srv)) < 0)
    {
        perror("connect");
    }

    write(fd, writeBuffer, 2);
    resetBuffer(writeBuffer, WRITE_BUFFER_LENGTH);
}

void sendData(char header, QString id, QString name, QString surname)
{
    struct sockaddr_in srv;
    int fd;
    char __id[8];
    char __name[16];
    char __surname[21];

    strcpy(__id, id.toStdString().c_str());
    strcpy(__name, name.toStdString().c_str());
    strcpy(__surname, surname.toStdString().c_str());

    writeBuffer[0] = header;
    writeBuffer[1] = ' ';
    strcat(writeBuffer, __id);
    strcat(writeBuffer, " ");
    strcat(writeBuffer, __name);
    strcat(writeBuffer, " ");
    strcat(writeBuffer, __surname);
    strcat(writeBuffer, ">");

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr(ADDRESS);

    if (connect(fd, (sockaddr *) &srv, sizeof(srv)) < 0)
    {
        perror("connect");
    }

    write(fd, writeBuffer, getBufferSize(writeBuffer));
    resetBuffer(writeBuffer, WRITE_BUFFER_LENGTH);
}
