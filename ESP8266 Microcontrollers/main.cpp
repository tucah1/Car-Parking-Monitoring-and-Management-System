#include <Arduino.h>
#include <Wiegand.h>
#include <ESP8266WiFi.h>
#include <string.h>
#include <string>


// Struct that will represent single node in linked list.
typedef struct __node{
  int _value;
  struct __node* _next;
}node;

// Struct that will represent linked list. This linked list is used to store digits entered by user through keypad.
// Linked list is used to easily create and manipulate queue of digits.
typedef struct __linked_list{
  short int counter;
  node* _first_node;
}list;

// Port and address of the server that is running on Raspberry Pi. Server is integrated into Main_RaspberryPi_Code.c code.
const uint16_t port = 8090;
const char * host = "192.168.1.100";

// Array that will hold seven digits from linked list when they need to be sent to main server on Raspberry Pi.
char* array = (char*) malloc(7 * sizeof(char));
// Creating actual linked list variable that will be used to hold digits from keypad.
list keypad;

WiFiClient wifiClient;
WIEGAND wg;


/*
*   Sends data to server on Raspberry Pi.
*   Takes one argument which is the data -> (code entered using button pad). Code is passed to the function in String format.
*/
void sendDataToServer(String);

/*
*   Adds newly entered digit from button pad to the list 'keypad'.
*   List can hold maximum 7 digits (elements). So if list already have 7 digits this function will remove first digit in the list and add the new one on the back of the list.
*/
void addNode(list*, int);

/*
*   Prints out list to the Serial monitor.
*/
void printList();

/*
*   Converts seven digits from the linked list to the seven digit integer.
*   Returns integer representation of the digits from linked list.
*/
int convertToInt();



void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  wg.begin(5,4);

  delay(10);

  Serial.println();
    Serial.println();
    Serial.print("Connecting to wifi");

    // Function below connects ESP microcontroller to the wifi. Wifi SSID is '24/7' and password is 'bosanskigazija99'.
    WiFi.begin("24/7", "bosanskigazija99");

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    keypad.counter = 0;
}

void loop() {

  if(wg.available())
    {
      unsigned long input = wg.getCode();
      Serial.print(input);
      Serial.println();

      // If RFID tag/card is read data(code) will be sent to the server straight away. RFID tag/card code is allways number higher than 100.
      // If button pad is used digit pressed is stored in linked list. Values between 0 and 9 (both 0 and 9 included).
      // If submit button is pressed on button pad code entered via button pad is sent to the server. Submit button has value of 13.
      if (input>99)
      {
        String cardid = String(input, DEC);
        sendDataToServer(cardid);
      }
      else
      {
        if (input < 10)
        {
          addNode(&keypad, (int)input);
        }
        else if(input == 13)
        {
          sendDataToServer((String)convertToInt());
		  for (int i = 0; i < 7; i++)
			  addNode(&keypad, 0);
        }
      }
    }
    }



void addNode(list* myList, int value)
{
  if (myList->counter == 0)
  {
    node* some_node = new node;
    some_node->_value = value;
    Serial.println(some_node->_value);
    Serial.println("Added as a head of the linked list!");
    Serial.println(myList->counter);
    some_node->_next = 0;
    myList->_first_node = some_node;
    Serial.println(myList->_first_node->_value);
    myList->counter = 1;
  }
  else if (myList->counter < 7)
  {
    node* some_node = new node;
    some_node->_value = value;
    Serial.println(some_node->_value);
    Serial.println(myList->counter);
    some_node->_next = 0;

    node* current = myList->_first_node;
    while(current)
    {
      if (!current->_next)
      {
        current->_next = some_node;
        Serial.println("Added as below 7");
        Serial.println(current->_next->_value);
        myList->counter += 1;
        Serial.println(myList->counter);
        break;
      }
      else
      {
        current = current->_next;
      }
    }
  }
  else
  {
      node* some_node = new node;
      some_node->_value = value;
      Serial.println(some_node->_value);
      Serial.println("Added as above 7");
      some_node->_next = 0;

      node* current = myList->_first_node;
      while(current)
      {
        if (!current->_next)
        {
          current->_next = some_node;
          break;
        }
        else
        {
          current = current->_next;
        }
      }

      node* _node = myList->_first_node;
      myList->_first_node = myList->_first_node->_next;
      delete _node;
    }

}

int convertToInt()
{

  int result = 0;
  node* current = keypad._first_node;
  while(current)
  {
      result = (result * 10) + current->_value;

      current = current->_next;
  }
  Serial.println(result);
  return result;
}

void sendDataToServer(String data)
{

    WiFiClient client;

    if (!client.connect(host, port)) {

        Serial.println("Connection to host failed");

        delay(1000);
        return;
    }

    Serial.println("Connected to server successful!");

    // Header '2' that is put in front of data is for server on Raspberry Pi to know which ESP controller is message coming. Entrance ramp is header '1' and exit ramp is header '2'.
    // This is only difference in the code between entrance and exit RFID reader and button pad microcontrollers.
    String _data = "2\n" + data;

    client.print(_data);
    Serial.println(_data);
    Serial.println("Disconnecting...");
    client.stop();

    delay(3000);
}
