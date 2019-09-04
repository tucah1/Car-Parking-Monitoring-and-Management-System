#ifndef COMUNICATION_H
#define COMUNICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <zconf.h>
#include <string.h>
#include "mainwindow.h"

#define PORT 8090                       // Port of the server on Raspberry Pi
#define ADDRESS "192.168.1.102"         // Address of the server on the Raspberry Pi
#define BUFFER_LENGTH 4096              // Length of the read buffer
#define WRITE_BUFFER_LENGTH 64          // Length of the send buffer


// Retruns length of the contents in the buffer that is passed in as parameter.
int getBufferLength(char[]);

// Receives and reads data from the server on Raspberry Pi.
void receive(char, MainWindow*);

// Sends single character header to the Raspberry Pi server.
void sendHeader(char);

// Sends header and user data to the Rapsberry Pi server.
// Besides the header it sends user ID, name and surname to the server.
void sendData(char, QString, QString, QString);


#endif // COMUNICATION_H
