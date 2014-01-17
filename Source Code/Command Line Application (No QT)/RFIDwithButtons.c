//RFID Reader Project
// Spring 2013
//Ian Bablewski, Calvin Flegal, Bryant Moquist

/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <pthread.h>
#include <signal.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>

/* Structs */
typedef struct {
    char* name;
    unsigned int serial;
    } tag;

/* Static Variables */
unsigned int charge = 2;
int gDebug = 0;
int gPortFd = -1;
char calvin[] = "Calvin";
char bryant[] = "Bryant";
char ian[] = "Ian";
int mode = 0;
int chargeSetting = 0;
int newValueSetting = 0;
int newAmount=100;
int autonomous = 0;

/* Idenfication */
tag calvin_tag = {.name = calvin, .serial = 3471480065};
tag bryant_tag = {.name = bryant, .serial = 4108817665};
tag ian_tag = {.name = ian, .serial = 2313852161};

/* Enumerations */
typedef enum {ReadSerial, ReadBalance, Payment, WriteNewBalance, TransactionComplete} state;

/* Option Variables */
struct option gLongOption[] =
{
    // option       A  Flag   V  (has_arg, flag, val)
    // -----------  -  ----  ---
    { "pay",      1, NULL, 'p' },
    { "write",      1, NULL, 'w' },
    { "debug",      0, NULL, 'd' },
    { "help",       0, NULL, 'h' },
	{ "read",       0, NULL, 'r' },
	{ "autonomous", 0, NULL, 'a' },
    { 0 },
    
};

/* Function Prototypes */
void  Usage( void );
void WriteBalance(unsigned int balance); 
unsigned int Pay(unsigned int starting_bal, unsigned int amount);
unsigned int ReadBal();
unsigned int ReadSerialNumber();
int getStat();

/* Functions */
void WriteBalance(unsigned int balance) {
    
    char status;
    char WriteBalCommand[9] = {'!','R','W','',''};
    WriteBalCommand[5] = balance & 0xff;
    WriteBalCommand[6] = (balance >> 8) & 0xff;
    WriteBalCommand[7] = (balance >> 16) & 0xff;
    WriteBalCommand[8] = (balance >> 24) & 0xff;
    
    if (gDebug) printf("WriteBalance function: Writing Balance \n");
    
    //Send the write command
    if( write( gPortFd, WriteBalCommand, 9 ) != 9 )  {
        printf("Unable to send write balance command \n");
        return;
    }
    
    //Check for errors
    if (read( gPortFd, &status, 1 ) != 1) {
        printf("Unable to read device status byte\n");
        return;
    }
    if (status != 1) {
        printf("Received error status byte. The status byte is: %d\n",status);
        return;
    }
    
    //Success
    if(gDebug) printf("Successful write.\n");
    return;
}

unsigned int Pay(unsigned int starting_bal, unsigned int amount) {
    unsigned int new_balance = starting_bal-amount;
    if(gDebug) printf("Pay function: The new balance is %u", new_balance);
    WriteBalance(new_balance);
    return new_balance;
}

unsigned int ReadBal() {
    
    int status;
    char ReadBalanceCommand[5] = {'!','R','W','',''};
    char ReadBuff[5]; //Include space for null terminator
    
    if (gDebug) printf("ReadBal function: Reading Balance \n");
    
    //Send the read command
    if( write( gPortFd, ReadBalanceCommand, 5 ) != 5 )  {
        printf("Unable to send read balance command.\n");
        return -1;
    }
    
    //Check for errors
    if (read( gPortFd, &ReadBuff, 5 ) != 5) {
        printf("Unable to read balance from device.\n");
        return -1;
    }
    
    status = (int)ReadBuff[0];
    if (status != 1) {
        printf("Received status byte error.  The status byte is: %d\n",status);
        return -1;
    }
    
    //Success
    char balance[5];
    int i =0;
    for (i=0; i<4;i++) balance[i] = ReadBuff[i+1];
    balance[4] = '\0';
    
    //Convert to an integer
    unsigned int *pInt = (unsigned int*)balance;
    unsigned int balance_int = *pInt;
    
    return balance_int;
}


unsigned int ReadSerialNumber() {
    
    int status;
    char ReadSerialCommand[5] = {'!','R','W','',' '};
    char ReadBuff[5]; //Include space for null terminator
    
    if (gDebug) printf("ReadSerialNumber function: Reading Serial\n");
    
    //Send the read command
    if( write( gPortFd, ReadSerialCommand, 5 ) != 5 )  {
        printf("Unable to send read serial command.\n");
        return 0;
    }
    
    //Check for errors
    if (read( gPortFd, &ReadBuff, 5 ) != 5) {
        printf("Unable to read serial number from device.\n");
        return 0;
    }
    
    status = (int)ReadBuff[0];
    if (status != 1) {
       // printf("Received status byte error.  The status byte is: %d\n",status);
        return 0;
    }
    
    //Success
    char serialnum[5];
    int i =0;
    for (i=0; i<4;i++) serialnum[i] = ReadBuff[i+1];
    serialnum[4] = '\0';
    
    //Convert to an integer
    unsigned int *pInt = (unsigned int*)serialnum;
    unsigned int serial = *pInt;
    
    return serial;
}

void DisplayName(unsigned int serial) {
    if (serial == calvin_tag.serial) {
        printf("Hello %s \n",calvin_tag.name);
        return;
        
    }
    if (serial == bryant_tag.serial) {
        printf("Hello %s \n",bryant_tag.name);
        return;
    }
    if (serial == ian_tag.serial) {
        printf("Hello %s \n",ian_tag.name);
        return;
    }
}

/* Status Function */
int getStat() {

	FILE * pFile;
	char * buff;
	pFile = fopen("/dev/kRFID","r+");

	
	if(pFile == NULL)
		printf("File not found");
	
	buff = (char*) malloc (128);

	int count;	
	count = fread (buff, 1, 128, pFile);
	
	fclose(pFile);
	
	sscanf(buff,"%d %d %d ",&mode, &chargeSetting, &newValueSetting);
	
	free(buff);
	
	charge = chargeSetting*5;
	newAmount = newValueSetting*20;
	
	return 0;
	
}

/*  Modes:
0: Read
1: Pay
2: New Value
*/

/* Main Function */
int main (int argc, char **argv )
{
    state currentState = ReadSerial;
    int         opt;
    char        devName[] = "/dev/ttyS2";
    speed_t     baudRate=B9600;
    struct termios attr;
    const char* amountEntered;
    const char* chargeEntered;
    int operateDevice = 1;
    unsigned int serialNum = 0;
    unsigned int balance;
    
    // Parse the command line options
    while (( opt = getopt_long( argc, argv, "p:w:dhra", gLongOption, NULL )) > 0 )
    {
        switch ( opt )
        {   
            case 'p':
            {
                chargeEntered = optarg;
                charge = atoi(chargeEntered);
				mode = 1;
				break;
            }
            
            case 'w':
            {
                amountEntered = optarg;
                mode = 2;
                newAmount = atoi(amountEntered);
				break;
            }
            case 'd':
            {
                gDebug = 1;
                break;
            }
			case 'r':
            {
                mode = 0;
                break;
            }
			case 'a':
			{
				autonomous = 1;
				break;
			}
            case '?':
            case 'h':
            {
                Usage();
                return 1;
            }
        }
    }
    
    // Open the serial port initially using O_NONBLOCK so that we won't block waiting for
    // carrier detect.
    
    if (( gPortFd = open( devName, O_RDWR | O_EXCL | O_NONBLOCK )) < 0 )
    {
        fprintf( stderr, "Unable to open serial port '%s': %s\n", devName, strerror( errno ));
        exit( 2 );
    }
    
    // Now that the serial port is open, we can turn off the non-blocking behaviour (for us we want
    // the reads to have blocking semantics).
    
    fcntl( gPortFd, F_SETFL, fcntl( gPortFd, F_GETFL ) & ~O_NONBLOCK );
    
    if ( tcgetattr( gPortFd, &attr ) < 0 )
    {
        fprintf( stderr, "Call to tcgetattr failed: %s\n", strerror( errno ));
        exit( 3 );
    }
    
    cfmakeraw( &attr );
    
    // CLOCAL - Disable modem control lines
    // CREAD  - Enable Receiver
    
    attr.c_cflag |= ( CLOCAL | CREAD );
    
    cfsetispeed( &attr, baudRate );
    cfsetospeed( &attr, baudRate );
    
    if ( tcsetattr( gPortFd, TCSAFLUSH, &attr ) < 0 )
    {
        fprintf( stderr, "Call to tcsetattr failed: %s\n", strerror( errno ));
        exit( 4 );
    }
	
	
    //Begin device operation
    while(operateDevice){
        switch (currentState) {
            case ReadSerial:
            {

                if(serialNum==0)
                {

					if(autonomous==1) 
					{
			
					int test = getStat();
					printf("The mode is: %d \n The chargeSetting is %d\n The newValueSetting is %d\n",mode,chargeSetting,newValueSetting);
					}
                    if(gDebug) printf("ReadSerial: Looking for tag.\n");
                    serialNum = ReadSerialNumber();
					sleep(3);
                    currentState = ReadSerial;
                }
                else if(serialNum!=0 && mode==2)
                {
                    if(gDebug) printf("ReadSerial: Begin write transaction.  The serial number is %u.\n", serialNum);
                    DisplayName(serialNum);
                    currentState = WriteNewBalance;
                }
                else
                {
                    if(gDebug) printf("ReadSerial: The serial number is %u.\n",serialNum);
                    DisplayName(serialNum);
                    currentState = ReadBalance;
                }
                break;
            }
            case ReadBalance:
            {
                if(gDebug) printf("ReadBalance: Reading the balance.\n");
                balance = ReadBal();
                printf("Current balance: %u\n", balance);
				if(mode==1)
					currentState = Payment;
				else
					{
					currentState = TransactionComplete;
					serialNum = 0;
					}
                break;
            }
            case Payment:
            {
                printf("Amount deducted: %u\n", charge);
                if(gDebug) printf("WriteNewBalance: Writing a new value");
                balance = Pay(balance,charge);
                if(gDebug) printf("The balance written by gumstix to device: %u\n", balance);
                currentState = TransactionComplete;
                break;
            }
            case WriteNewBalance:
            {
                if(gDebug) printf("WriteNewBalance: Writing %u to tag.\n", newAmount);
                WriteBalance(newAmount);
                if(autonomous==0)
				{
				operateDevice = 0;	
				}
                currentState = TransactionComplete;
            }
            case TransactionComplete:
            {
				if(mode!=0)
				{
                printf("Transaction Complete\n");
                balance = ReadBal();
                printf("New balance: %u\n", balance);
				}
				serialNum = 0;
                currentState = ReadSerial;
				sleep(3);
                break;
            }
            default:
                break;
        }
        
    }
    
    fprintf( stderr, "Exiting...\n" );

    close( gPortFd );
    
    if ( gDebug )
    {
        fprintf( stderr, "Done\n" );
    }
    
    exit( 0 );
    
    return 0;
}

//Usage
void Usage()
{
    fprintf( stderr, "Usage: rfid [option(s)]\n" );
    fprintf( stderr, "Communication via UART" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "  -r, --read        Read the current balance\n" );
	fprintf( stderr, "  -p, --charge=newCharge   Set the amount of the charge.  The default charge is %u\n", charge);
    fprintf( stderr, "  -w, --Amount=newAmount   Write a new amount to a tag.  The amount must be greater than or equal to zero.\n" );
	fprintf( stderr, "  -a, --autonomous        Autonomous Mode with button input.\n" );   
    fprintf( stderr, "  -d, --debug       Turn on debug output\n" );
    fprintf( stderr, "  -h, --help        Display this message\n" );
}