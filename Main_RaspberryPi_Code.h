#ifndef NEW_RAMP_CODE_2_H
#define NEW_RAMP_CODE_2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <netinet/in.h>
#include <zconf.h>
#include <arpa/inet.h>
#include <pigpio.h>
#include <string.h>
#include<pthread.h>


#define MIN_WIDTH 1100								// Minimum width for the servo motor range
#define MAX_WIDTH 2000								// Maximum width for the servo motor range
#define STEP 45										// Step of the servo motors
#define MOTOR_LEFT 5								// GPIO pin of the left motor
#define MOTOR_RIGHT 20								// GPIO pin of the right motor
#define SENSOR_LEFT 17								// GPIO pin of the left sensor
#define SENSOR_RIGHT 18								// GPIO pin of the right sensor
#define READ_BUFFER 64								// Size of the readBuffer
#define WRITE_BUFFER 1024							// Size of the writeBuffer
#define MICROCONTROLLER1 "192.168.1.101"			// IP address of the entrance microcontroller
#define MICROCONTROLLER2 "192.168.1.101"			// IP address of the exit microcontroller


// Struct that represents single node in the linked list data structure.
// Node contains data about single user: user ID, user name and surname, user status(whether user is currently on the parking).
typedef struct __node {
	int _id;
	char _name[20];
	char _surname[25];
	short int _status;
	struct __node* _next;	
} node;


// Struct that represents linked list structure.
// Linked list will store data of all users. 
// Linked list is used to prevent continious accessing to the file database for every single check.
// This will massively speed up the process of checking user data, but COST is more MEMORY USAGE.
typedef struct __linked_list {
	node* _first_node;
	int _counter;
} list;


// Struct that contains parameters that are passed to the 'motorUp' function.
// Parameters need to be passed via struct because the 'motorUp' function is called on seperate thread,
// and function called like that takes only one parameter of type 'void*', in this case that is pointer to this struct.
// _motor -> necessary to distinguish which motor needs to go up (entrance or exit ramp)
// __node -> necessary to update status of the particular user that enters/exits parking
typedef struct __motor_up_parameters {
    int _motor;
    node* __node;
} motorUpParam;


extern struct sockaddr_in cli;						// Structure that saves client information
extern struct sockaddr_in srv;  					// Structure that saves server information
extern char readBuffer[READ_BUFFER];				// Buffer to which server will read incoming message
extern char writeBuffer[WRITE_BUFFER];				// Buffer that will be sent by server
extern int cli_len, n;							    // cli_len will hold size of cli structure, n is just a counter
extern int fd, newfd;								// fd is server (host) socket, newfd is socket from connected client
extern void *p;									    // Pointer which will be used to move trough the buffer

extern int header;									// For storing headers of TCP/IP messages server receives
extern int data;									// For storing data of TCP/IP messages server receives (ID that microcontrollers will send)
extern bool isEntranceOpened;		
extern bool isExitOpened;
extern int id_from_qt;								// For storing user ID that server receives form Qt application
extern char name_from_qt[16];						// For storing user Name that server receives from Qt application
extern char surname_from_qt[21];					// For storing user Surname that server receives from Qt application

extern list database;								// Actual linked list variable created from 'list' struct
extern motorUpParam parameter;
extern pthread_mutex_t lock;


// Creates TCP/IP socket.
void createSocket();


// Populates sockaddr_in struct that contains server information with relevant data.
// Sets server port, address and family.
void populateSrv();


// Binds previously created socket with the sockaddr_in struct containing relevant server information.
void bindAddress();


// Sets server socket in listening mode.
void listenOnSocket();


// Reads message that server receives.
void readMessage();


// Sends message from server to device that is connected to it.
void writeMessage();


// Reads one word from the buffer.
// Takes three arguments: buffer from which to read, buffer to which to save word that is read, index from which function starts reading.
int getWordFromBuffer(char [], char [], int);


// Creates and runs server.
// Function is designed to use with multithreading, it can be ran on seperate thread and it runs on its own.
// Server can take only one connection at the time.
// This server communicates with GUI application and with ESP8266 microcontrollers.
void* server();


// Handles server connection from microcontrollers.
// It reads message that server receives from microcontrollers and saves relevant data to global variables for the main code to use.
// It sets 'header' and 'data' global variables to the values received from microcontroller.
// 'header' variable takes the microcontroller ID ('1' or '2') - ('entrance' or 'exit')
// 'data' variable takes the code that user entered on RFID reader or button pad
void handleMicrocontrollers();


// Handles server connection from GUI application.
// It reads header and data from the message received from GUI app, and performs appropriate action depending on what the header was.
void handleQtConnection();


// Collects all users that are currently on the parking from the linked list, and sends those users and their data to the GUI application.
// Takes one argument which is pointer to the linked list that contains user data.
void sendActiveUsers(list*);


// Reads input from the IR proximity sensor and depending on the sensor input controls the ramp (motor).
// This function is called when ramp is brought up manually via GUI application,
// meaning that this function will not edit status of any user.
// Takes two arguments: IR sensor ID and motor ID, for function to know which IR and motor to read and control.
void irControlOfTheMotorManual(int, int);


// Reads input from the IR proximity sensor and depending on the sensor input controls the ramp (motor).
// This function is called when ramp is brought up automatically by the software,
// meaning that this function will edit status of the user that enters/exits.
// Takes three arguments: IR sensor ID, motor ID and pointer to the linked list node which contains data of the user trying to enter/exit.
void irControlOfTheMotor(int, int, node*);


// Brings ramp up when signal is sent from GUI application. Ramp will go down automatically.
// Function is designed to be called on seperate thread. If this was not the case, whole system would have to wait until this function is over which takes time.
void* motorUpManual(void *);


// Brings ramp up when signal is set from GUI application. Ramp will not go down automatically.
// Function is designed to be called on seperate thread. If this was not the case, whole system would have to wait until this function is over which takes time.
void* motorUpAndStay(void *);


// Brings ramp up when user tries to enter using RFID reader or button pad. Ramp will go down automatically.
// Function is designed to be called on seperate thread. If this was not the case, whole system would have to wait until this function is over which takes time.
void* motorUp(void *);


// Brings motor down. Called automatically by when ramp needs to go down.
// Takes one argument which is ID of the motor. For function to know which ramp needs to go down.
void motorDown(int);


// Brings motor down. Called manually by the GUI application.
// Function is designed to be called on seperate thread. If this was not the case, whole system would have to wait until this function is over which takes time.
void* _motorDown(void *);


// Loads data from file database to the linked list.
void loadDatabaseFromFile();


// Checks if user ID is valid. Meaning: it checks if user ID is in database (is it allowed to use parking),
// and if previous is ture it checks if user is already on the parking.
// If user ID is valid it lets user in/out.
void idCheck(list*);


// Adds node to the linked list. Used to add new users.
// Takes four arguments: poiter to the linked list which will be updated, user ID, user name and user surname.
void addNode(list*, int, char*, char*);


// Adds new user data to the file database.
// Takes three arguments: user ID, user name and user surname.
void addToFile(int, char*, char*);


// Prints out data from linked list to the console.
void printList();


// Removes user form database file and from linked list.
// Takes one argument which is ID of the user that will be deleted.
void removeUser(int);


// Deletes single node from linked list. This function is automatically called by 'removeUser' function.
// Takes one argument which is ID of the user that will be deleted.
void deleteNode(int);


// Updates file database with new data from linked list
void updateFile(list*);


#endif