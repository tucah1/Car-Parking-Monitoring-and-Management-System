#include "Main_RaspberryPi_Code.h"

struct sockaddr_in cli;
struct sockaddr_in srv;
char readBuffer[READ_BUFFER];
char writeBuffer[WRITE_BUFFER];
int cli_len, n;
int fd, newfd;
void *p;

int header;
int data;
bool isEntranceOpened;		
bool isExitOpened;
int id_from_qt;
char name_from_qt[16];
char surname_from_qt[21];

list database;
motorUpParam parameter;
pthread_mutex_t lock;

int main (void)
{
	n = 0;
	isEntranceOpened = false;
	isExitOpened = false;
	pthread_t server_thread;
	createSocket();
	populateSrv();
	bindAddress();
	listenOnSocket();
	database._counter = 0;
	if (gpioInitialise() < 0) 
		return -1;
	
	loadDatabaseFromFile();

	if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex init failed\n");
        return 1;
    }

	if (pthread_create(&server_thread, NULL, &server, NULL) != 0) {
		perror("pthread_create");
		return 1;
	}

	while(1)
	{
		idCheck(&database);
	}

	pthread_join(server_thread, NULL);
	pthread_mutex_destroy(&lock);

	return 0;
}


void createSocket() {
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

}

void populateSrv() {
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET; 
    srv.sin_port = htons(8090); 
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
}

void bindAddress() {
    if(bind(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
        perror("bind");
        exit(1);
    }
}

void listenOnSocket() {
    if(listen(fd, 10) < 0) {
        perror("listen");
        exit(1);
    }
}

void readMessage() {
    if ((n = read(newfd, readBuffer, READ_BUFFER)) < 0) {
         perror("read");
     }
}

void writeMessage() {
    p = writeBuffer;
	n = 0;
    int size = 0, total = 0;
    for (; writeBuffer[size] != 0; size++);
    do {
		
        if ((n = write(newfd, p, size)) < 0) {
            perror("write");
            break;
        }
        p += n;
		total += n;
    } while(total < size);
    p = 0;
    n = 0;
}

int getWordFromBuffer(char arr[], char result[], int index) {
    int i = 0;
    int c = index;
	printf("Getting word form buffer\n");
    while(1) {
        if ((arr[c] == ' ') || (arr[c] == '\n') || (arr[c] == 0) || (arr[c] == '>'))
			break;
		result[i] = arr[c];
        i++;
		c++;
    }
    return c;
}

void* server() {
    
    cli_len = sizeof(cli);
	char temp[32];
    while(1) {

		if ((newfd = accept(fd, (struct sockaddr*) &cli, &cli_len)) == - 1) {
			perror("accept");
			continue;
		}
		inet_ntop(AF_INET, &(cli.sin_addr), temp, INET_ADDRSTRLEN);
        readMessage();
        if ((cli.sin_addr.s_addr == inet_addr(MICROCONTROLLER1)) || (cli.sin_addr.s_addr == inet_addr(MICROCONTROLLER2))) {
            handleMicrocontrollers();
        }
        else {
            handleQtConnection();
        }
        
        memset(readBuffer, 0, READ_BUFFER);
        memset(writeBuffer, 0, WRITE_BUFFER);
		close(newfd);
	}
}

void handleMicrocontrollers() {
	char _data[8];
	char _header[1];
	
	pthread_mutex_lock(&lock);

	_header[0] = readBuffer[0];
	header = atoi(_header);
    getWordFromBuffer(readBuffer, _data, 2);
    data = atoi(_data);

	pthread_mutex_unlock(&lock);

	memset(_data, 0, 8);

}

void handleQtConnection() {
	char _header[1];
	char _id[8];
    int flag, c, counter = 0, m = 0;
	
    _header[0] = readBuffer[0];
	
	if ( _header[0] == 'k') {
		pthread_mutex_lock(&lock);
		flag = getWordFromBuffer(readBuffer, _id, 2);
		id_from_qt = atoi(_id);	
		printf("%d\n\n", id_from_qt);
		removeUser(id_from_qt);
		id_from_qt = 0;
		pthread_mutex_unlock(&lock);
		return;
	}
	else if( _header[0] == 'j') {
		pthread_mutex_lock(&lock);
		flag = getWordFromBuffer(readBuffer, _id, 2);
		flag = getWordFromBuffer(readBuffer, name_from_qt, flag + 1);
		flag = getWordFromBuffer(readBuffer, surname_from_qt, flag + 1);
		id_from_qt = atoi(_id);
		printf("%d\n%s\n%s\n\n", id_from_qt, name_from_qt, surname_from_qt);
		addToFile(id_from_qt, name_from_qt, surname_from_qt);
		id_from_qt = 0;
		memset(name_from_qt, 0, 16);
		memset(surname_from_qt, 0, 21);
        flag = 0;
		pthread_mutex_unlock(&lock);
		return;
	}
    else if (_header[0] == 'a') {
		pthread_mutex_lock(&lock);
        FILE* fp;
		if ((fp = fopen("all_users.DAT", "r")) == NULL)
			printf("Could not open all_users file\n");

		while((fscanf(fp, "%c", &c)) != EOF)
		{
			writeBuffer[counter++] = c;
		}
		counter = 0;
		printf("%s\n", writeBuffer);
        writeMessage();
		fclose(fp);
		pthread_mutex_unlock(&lock);
        return;
    }
	else {
		pthread_t temp;
		pthread_t temp2;
		switch(_header[0]) {
			case 'b':
				sendActiveUsers(&database);
				break;
			case 'i':
				m = MOTOR_RIGHT;
				pthread_create(&temp, NULL, &motorUpManual, (void *)&m);
				pthread_detach(temp);
				break;
			case 'h':
				m = MOTOR_LEFT;
				pthread_create(&temp, NULL, &motorUpManual, (void *)&m);
				pthread_detach(temp);
				break;
			case 'g':
				m = MOTOR_RIGHT;
				pthread_create(&temp, NULL, &motorUpAndStay, (void *)&m);
				pthread_detach(temp);
				break;
			case 'f':
				m = MOTOR_LEFT;
				pthread_create(&temp, NULL, &motorUpAndStay, (void *)&m);
				pthread_detach(temp);
				break;
			case 'e':
				m = MOTOR_RIGHT;
				pthread_create(&temp, NULL, &_motorDown, (void *)&m);
				pthread_detach(temp);
				break;
			case 'd':
				m = MOTOR_LEFT;
				pthread_create(&temp, NULL, &_motorDown, (void *)&m);
				pthread_detach(temp);
				break;
			case 'c':
				m = MOTOR_RIGHT;
				pthread_create(&temp, NULL, &motorUpAndStay, (void *)&m);
				pthread_detach(temp);
				time_sleep(0.05);
				m = MOTOR_LEFT;
				pthread_create(&temp2, NULL, &motorUpAndStay, (void *)&m);
				pthread_detach(temp2);
				break;			
		}
		return;
	}
}

void sendActiveUsers(list* _database) {
	char temp[64];
	bool check = false;

	pthread_mutex_lock(&lock);

	printf("IN SENDACTIVEUSERS!\n");
    if (_database->_counter == 0) {
        writeBuffer[0] = '/';
        writeMessage();
		pthread_mutex_unlock(&lock);
		return;
    }

	node * current = _database->_first_node;
	while(current) {
		if (current->_status == 1 || current->_status == 2) {
            sprintf(temp, "%d %s %s \n", current->_id, current->_name, current->_surname);
            strcat(writeBuffer, temp);
			check = true;
		}
		current = current->_next;
        memset(temp, 0, 64);
	}
	if (check)
    	writeMessage();
	else {
		writeBuffer[0] = '/';
		writeMessage();
		pthread_mutex_unlock(&lock);
		return;
	}
	pthread_mutex_unlock(&lock);
}

void irControlOfTheMotorManual(int ir_pin, int motor)
{
	int i;
   	int check = 0;

    for (i = 0; i <= 10; i++)
    {
	if (gpioRead(ir_pin) == check)
	{
		break;
	}
	time_sleep(1);

	if (i == 8)
     {
          motorDown(motor);
          return;
     }
    }

    while(gpioRead(ir_pin) == check)
    {
        time_sleep(0.3);
    }
    time_sleep(1);
	motorDown(motor);

}

void irControlOfTheMotor(int ir_pin, int motor, node* _node)
{
    int i;
    int check = 0;
    bool didCarPass = false;

    for (i = 0; i <= 10; i++)
    {
	if (gpioRead(ir_pin) == check)
	{
		didCarPass = true;
		break;
	}
	time_sleep(1);
	if (i == 8)
     {
          motorDown(motor);
          return;
     }
    }

    while(gpioRead(ir_pin) == check)
    {
        time_sleep(0.3);
    }
	if (didCarPass)
	{
		pthread_mutex_lock(&lock);
		if (_node->_status == 1)
			_node->_status = 0;
		else if(_node->_status == 0)
			_node->_status = 1;
		else
			deleteNode(_node->_id);
		pthread_mutex_unlock(&lock);
	}
    time_sleep(1);
	motorDown(motor);
}

void* motorUpAndStay(void * motor)
{
	int * _motor = (int *) motor;
	int m = *_motor;
	if (m == MOTOR_RIGHT)
	{
		pthread_mutex_lock(&lock);
		if (isEntranceOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		else
			isEntranceOpened = true;
		pthread_mutex_unlock(&lock);
	}
	else
	{
		pthread_mutex_lock(&lock);
		if (isExitOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		else
			isExitOpened = true;
		pthread_mutex_unlock(&lock);
	}

	int width = MAX_WIDTH;
	while (width > MIN_WIDTH)
	{
		gpioServo(m, width);
		width -= STEP;
		time_sleep(0.1);
	}

	return NULL;
}

void* motorUpManual(void * motor)
{
	int * _motor = (int *) motor;
	int m = *_motor;
	if (m == MOTOR_RIGHT)
	{
		pthread_mutex_lock(&lock);
		if (isEntranceOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		else
			isEntranceOpened = true;
		pthread_mutex_unlock(&lock);
	}
	else
	{
		pthread_mutex_lock(&lock);
		if (isExitOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		else
			isExitOpened = true;
		pthread_mutex_unlock(&lock);
	}

	int width = MAX_WIDTH;
	while (width > MIN_WIDTH)
	{
		gpioServo(m, width);
		width -= STEP;
		time_sleep(0.1);
	}

	if (m == MOTOR_LEFT)
		irControlOfTheMotorManual(SENSOR_LEFT, m);
	else
		irControlOfTheMotorManual(SENSOR_RIGHT, m);
	
	return NULL;
}

void* motorUp(void* str)
{
	motorUpParam* __parameter;
	int motor;
	node* _node;
	__parameter = (motorUpParam *) str;
	motor = __parameter->_motor;
	_node = __parameter->__node;

	if (motor == MOTOR_RIGHT)
	{
		pthread_mutex_lock(&lock);
		if (isEntranceOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		else
			isEntranceOpened = true;
		pthread_mutex_unlock(&lock);
	}
	else
	{
		pthread_mutex_lock(&lock);
		if (isExitOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		else
			isExitOpened = true;
		pthread_mutex_unlock(&lock);
	}
	int width = MAX_WIDTH;
	while (width > MIN_WIDTH)
	{
	gpioServo(motor, width);
	width -= STEP;
	time_sleep(0.1);
	}

	if (motor == MOTOR_LEFT)
		irControlOfTheMotor(SENSOR_LEFT, motor, _node);
	else
		irControlOfTheMotor(SENSOR_RIGHT, motor, _node);

	return NULL;
}
 
void motorDown(int motor)
{
	pthread_mutex_lock(&lock);
	if (motor == MOTOR_RIGHT)
	{
		if (!isEntranceOpened) {
			pthread_mutex_unlock(&lock);
			return;
		}
	}
	else
	{
		if (!isExitOpened) {
			pthread_mutex_unlock(&lock);
			return;
		}
	}
	pthread_mutex_unlock(&lock);

	int width = MIN_WIDTH;
	for (; width <= MAX_WIDTH; width += STEP)
	{
	gpioServo(motor, width);
	time_sleep(0.1);
	}

	pthread_mutex_lock(&lock);

	if (motor == MOTOR_RIGHT)
		isEntranceOpened = false;
	else
		isExitOpened = false;
	pthread_mutex_unlock(&lock);
}

void* _motorDown(void * motor)
{
	int * _motor = (int *) motor;
	int m = *_motor;
	pthread_mutex_lock(&lock);
	if (m == MOTOR_RIGHT)
	{
		if (!isEntranceOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
	}
	else
	{
		if (!isExitOpened) {
			pthread_mutex_unlock(&lock);
			return NULL;
		}
	}
	pthread_mutex_unlock(&lock);

	int width = MIN_WIDTH;
	for (; width <= MAX_WIDTH; width += STEP)
	{
	gpioServo(m, width);
	time_sleep(0.1);
	}

	pthread_mutex_lock(&lock);

	if (m == MOTOR_RIGHT)
		isEntranceOpened = false;
	else
		isExitOpened = false;
	pthread_mutex_unlock(&lock);

	return NULL;

}

void loadDatabaseFromFile()
{
	int __id;
	char __name[20];
	char __surname[25];
	FILE* fp;

	if ((fp = fopen("all_users.DAT", "r")) == NULL)
	{
		printf("Could not open file all_users.DAT");
		return;
	}
	

	while (!feof(fp))
	{
		fscanf(fp, "%d", &__id);
		fscanf(fp, "%s", __name);
		fscanf(fp, "%s", __surname);
		addNode(&database, __id, __name, __surname);
	}
	fclose(fp);
}

void idCheck(list* _database)
{
	node* current = _database->_first_node;	

	while(current)
	{
		if (current->_id == data)
		{
			pthread_t temp;

			if(current->_status == 0 && header == 1)
			{
				printf("Status: %d and Header: %d!", current->_status, header);
				parameter._motor = MOTOR_RIGHT;
				parameter.__node = current;
				pthread_create(&temp, NULL, &motorUp, (void *)&parameter);
				pthread_detach(temp);
			}

			else if(current->_status == 1 && header == 2)
			{
				printf("Status: %d and Header: %d!!", current->_status, header);
				parameter._motor = MOTOR_LEFT;
				parameter.__node = current;
				pthread_create(&temp, NULL, &motorUp, (void *)&parameter);
				pthread_detach(temp);
			}
			else if(current->_status == 2 && header == 2)
			{
				printf("Status: %d and Header: %d!!!", current->_status, header);
				parameter._motor = MOTOR_LEFT;
				parameter.__node = current;
				pthread_create(&temp, NULL, &motorUp, (void *)&parameter);
				pthread_detach(temp);
			}
		}

		current = current->_next;
	}
	data = 0;
	header = 0;
}

void addNode(list* _database, int id, char* name, char* surname)
{
	
	if (database._counter == 0)
	{
		node* newNode = (node*) malloc(sizeof(node));
		newNode->_id = id;
		strcpy(newNode->_name, name);
		strcpy(newNode->_surname, surname);
		newNode->_status = 0;
		newNode->_next = 0;
		_database->_first_node = newNode;
		printf("\n");
		_database->_counter += 1;
		return;
	}
	else 
	{
		node* current = _database->_first_node;

		while(current)
		{
			if (current->_id == id)
				return;
			
			if (!current->_next)
			{
				
				node* newNode = (node*) malloc(sizeof(node));
				newNode->_id = id;
				strcpy(newNode->_name, name);
				strcpy(newNode->_surname, surname);
				newNode->_status = 0;
				newNode->_next = 0;
				current->_next = newNode;
				printf("\n");
				_database->_counter += 1;
				return;
			}
			else
			{
				current = current->_next;
			}
		}
	}

}

void addToFile(int id, char* name, char* surname)
{
	node* current = database._first_node;
	while(current)
	{
		if(current->_id == id)
		return; 
		current = current->_next;
	}

	FILE* fp;
	if ((fp = fopen("all_users.DAT", "a")) == NULL)
	{
		printf("Could not open file all_users.DAT");
		return;
	}

	fprintf(fp, "%d ", id);
	fprintf(fp, "%s ", name);
	fprintf(fp, "%s \n", surname);
	fclose(fp);
	addNode(&database,id,name,surname);
}

void printList()
{
	node* current = database._first_node;
	while (current)
	{
		printf("%d %s %s %d\n", current->_id, current->_name, current->_surname, (int)current->_status);
		current = current->_next;

	}
}

void removeUser(int id)
{
	bool check = false;
	node* current = database._first_node;
	while(current)
	{
		if(current->_id == id)
		{
			check = true;
			if (current->_status == 1)
				current->_status = 2;
			else if(current->_status == 0)
				deleteNode(id);
		}
		current = current->_next;
	}

	if (!check)
		return;
}

void deleteNode(int id)
{
	if (database._first_node->_id == id)
	{
		node* temp = database._first_node;
		database._first_node = database._first_node->_next;
		free(temp);
		updateFile(&database);
		return;
	}

	node* current = database._first_node->_next;
	node* previous = database._first_node;

	while(current)
	{
		if (current->_id == id)
		{
			previous->_next = current->_next; 
			free(current);
			updateFile(&database);
			return;
		}

		previous = current;
		current = current->_next;
	}
}

void updateFile(list* _database)
{
	system("rm all_users.DAT");

	FILE* fp;
	
	if((fp = fopen("all_users.DAT", "w")) == NULL)
	{
		printf("Could not create all_users.DAT/n");
		return;
	}

	node* current = _database->_first_node;

	while(current)
	{
		fprintf(fp, "%d ", current->_id);
		fprintf(fp, "%s ", current->_name);
		fprintf(fp, "%s \n", current->_surname);
		
		current = current->_next;
	}
	fclose(fp);
}
