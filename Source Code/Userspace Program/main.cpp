#include <QtGui/QApplication>
#include "Rfid.h"

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
struct {
    char* name;
    unsigned int serial;
} calvin_tag,bryant_tag,ian_tag;

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

typedef enum {ReadSerial, ReadBalance, Payment, WriteNewBalance, TransactionComplete} state;

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

void  Usage( void );
void WriteBalance(unsigned int balance); 
unsigned int Pay(unsigned int starting_bal, unsigned int amount);
unsigned int ReadBal();
unsigned int ReadSerialNumber();
int getStat();


/// Here lies the normal qt main not...

int main(int argc, char *argv[])
{

    char        devName[] = "/dev/ttyS2";
    speed_t     baudRate=B9600;
    struct termios attr;

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

    /* Qt main */
    QApplication app(argc, argv);
    Rfid widget(0,gPortFd);
    widget.show();

    return app.exec();
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

