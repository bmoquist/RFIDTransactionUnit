#include "Rfid.h"

Rfid::Rfid(QWidget *parent, int gPort):QWidget(parent)
{
    // local variable to this instance 
    std::string calvin("Calvin");
    std::string bryant("Bryant");
    std::string ian("Ian");


    // set all variables used throughout the class
    gPortFd = gPort;
    operateDevice = 1;
    serialNum = 0;
    lastSerial = 0;
    balance = 0;
    autonomous = 1;
    chargeSetting = 0;
    newValueSetting = 0;
    gDebug = 1; 
    charge = 2;
    currentState = ReadSerial;
    newAmount=100;
    calvin_tag.name = calvin;
    calvin_tag.serial = 3471480065;
    bryant_tag.name = bryant;
    bryant_tag.serial = 4108817665;
    ian_tag.name = ian;
    ian_tag.serial = 2313852161;  
    mode = 0;

    // set window title and size
    setWindowTitle("RFID Transaction Module");
    resize(480,250);


    // declare font type
    QFont f("Arial", 15, QFont::Bold);

    // construct welcome banner
    welcome = new QLabel();
    welcome->setStyleSheet("qproperty-alignment: AlignCenter");
    welcome->show();

    // constuct a qstr as an intermediate value
    qstr = new QString();    
    
    // construct the proper QLabel's and QLineEdit's
    name = new QLineEdit();
    qbalance = new QLineEdit();
    cost = new QLineEdit();
    newqbalance = new QLineEdit();
    amountAdd = new QLineEdit();
    nameText = new QLabel();
    qbalanceText = new QLabel();
    costText = new QLabel();
    newqbalanceText = new QLabel();
    amountAddText = new QLabel();
    
    // Set the text alignments
    name->setStyleSheet("qproperty-alignment: AlignCenter");
    qbalance->setStyleSheet("qproperty-alignment: AlignCenter");
    cost->setStyleSheet("qproperty-alignment: AlignCenter");
    newqbalance->setStyleSheet("qproperty-alignment: AlignCenter");
    nameText->setStyleSheet("qproperty-alignment: AlignCenter");
    qbalanceText->setStyleSheet("qproperty-alignment: AlignCenter");
    costText->setStyleSheet("qproperty-alignment: AlignCenter");
    newqbalanceText->setStyleSheet("qproperty-alignment: AlignCenter");
    amountAdd->setStyleSheet("qproperty-alignment: AlignCenter");
    amountAddText->setStyleSheet("qproperty-alignment: AlignCenter");

    // set the fonts
    welcome->setFont(f);
    name->setFont(f);
    qbalance->setFont(f);
    cost->setFont(f);
    newqbalance->setFont(f);
    nameText->setFont(f);
    qbalanceText->setFont(f);
    costText->setFont(f);
    newqbalanceText->setFont(f);
    amountAdd->setFont(f);
    amountAddText->setFont(f);

    // hide everyone but the welcome banner
    name->hide();
    qbalance->hide();
    cost->hide();
    newqbalance->hide();
    nameText->hide();
    qbalanceText->hide();
    costText->hide();
    newqbalanceText->hide();
    amountAdd->hide();
    amountAddText->hide();

    // set up widget layout
    QFormLayout *qlayout = new QFormLayout;
    qlayout->addRow(welcome);
    qlayout->addRow(nameText,name);    
    qlayout->addRow(qbalanceText,qbalance);    
    qlayout->addRow(costText,cost);    
    qlayout->addRow(amountAddText,amountAdd);    
    qlayout->addRow(newqbalanceText,newqbalance);
    qlayout->setFormAlignment(Qt::AlignCenter);    
    this->setLayout(qlayout); 

    // timer to continuously call UART to simulate a while loop
    uartTimer = new QTimer(this);
    uartTimer->setInterval(5);
    connect(uartTimer, SIGNAL(timeout()),this, SLOT(UART()));
    uartTimer->start();
}

void Rfid::updateMode()
{
    // calls the appropriate page setup according to the mode
    if(mode==0)
    {
	setReadPage();
    }
    else if(mode==1)
    {
	setUpdatePage();
    }
    else if(mode==2)
    {
	setWritePage();
    }
    else
    {
	setWelcomePage();
    }
}

void Rfid::clearScreen()
{
    // sets visible states of widgets according to state
    if(mode==0)
    {
	name->setText("");
	name->show();
	qbalance->setText("");
	qbalance->show();
    }
    else if(mode==1)
    {
	name->setText("");
	name->show();
	qbalance->setText("");
	qbalance->show();
	newqbalance->setText("");
	newqbalance->show();
    }
    else if(mode==2)
    {
	name->setText("");
	name->show();
	qbalance->setText("");
	qbalance->show();
	newqbalance->setText("");
	newqbalance->show();
    }
    else
    {
	setWelcomePage();
    }
}

void Rfid::setWelcomePage()
{
    // sets the welcome page layout
    welcome->setText("Welcome!\n Courtesy of Embedded Bologna");
    name->hide();
    nameText->hide();
    cost->hide();
    costText->hide();
    qbalance->hide();
    qbalanceText->hide();
    newqbalance->hide();	
    newqbalanceText->hide();	
    amountAdd->hide();
    amountAddText->hide();
}

void Rfid::setReadPage()
{
    // sets the read page layout
    setWindowTitle("RFID Transaction Module");
    welcome->setText("Tap tag to check balance");

    QString nm = DisplayName(serialNum);
    name->setText(nm);
    name->show();

    QString bal = QString::number(balance);
    qbalance->setText(bal);
    qbalance->show();

    cost->hide();
    newqbalance->hide();

    nameText->setText("Name: ");
    nameText->show();

    qbalanceText->setText("Account balance: ");
    qbalanceText->show();

    newqbalanceText->hide();
    costText->hide();

    amountAdd->hide();
    amountAddText->hide();
}

void Rfid::setWritePage()
{
    // sets the write page layout
    setWindowTitle("RFID Transaction Module");
    welcome->setText("Tap to load new balance");

    QString nm = DisplayName(serialNum);
    name->setText(nm);
    name->show();

    QString bal = QString::number(balance);
    qbalance->setText(bal);
    qbalance->show();

    cost->hide();

    QString nbal = QString::number(nbalance);
    newqbalance->setText(nbal);
    newqbalance->show();

    nameText->setText("Name: ");
    nameText->show();

    qbalanceText->setText("Account balance: ");
    qbalanceText->show();

    costText->hide();

    newqbalanceText->setText("New account balance: ");
    newqbalanceText->show();

    QString add = QString::number(newAmount);
    amountAdd->setText(add);
    amountAdd->show();

    amountAddText->setText("Amount to add: ");
    amountAddText->show();
}

void Rfid::setUpdatePage()
{
    // sets the payment page layout
    setWindowTitle("RFID Transaction Module");
    welcome->setText("Tap tag to make payment");

    QString nm = DisplayName(serialNum);
    name->setText(nm);
    name->show();

    QString bal = QString::number(balance);
    qbalance->setText(bal);
    qbalance->show();

    QString nbal = QString::number(nbalance);
    newqbalance->setText(nbal);
    newqbalance->show();

    QString chrg = QString::number(charge);    
    cost->setText(chrg);
    cost->show();

    nameText->setText("Name: ");
    nameText->show();

    qbalanceText->setText("Account balance: ");
    qbalanceText->show();

    costText->setText("Charge: ");
    costText->show();

    newqbalanceText->setText("New account balance: ");
    newqbalanceText->show();

    amountAdd->hide();
    amountAddText->hide();
}


//State Machine
void Rfid::UART()
{
    switch (currentState) {
	case ReadSerial:
	    {

		if(serialNum==0)
		{

		    if(autonomous==1) 
		    {
			updateMode();
			clearScreen();
			int test = getStat();
		    }

		    if(gDebug) printf("ReadSerial: Looking for tag.\n");
		    serialNum = ReadSerialNumber();
		    currentState = ReadSerial;
		}

		else
		{
		    if(gDebug) printf("ReadSerial: The serial number is %u.\n",serialNum);
		    currentState = ReadBalance;
		}
		break;
	    }

	case ReadBalance:
	    {
		if(gDebug) printf("ReadBalance: Reading the balance.\n");
		balance = ReadBal();
		updateMode();
		if(mode==1)
		    currentState = Payment;
		else if(mode==2)
		    currentState = WriteNewBalance;
		else
		{
		    currentState = TransactionComplete;
		    serialNum = 0;
		}
		break;
	    }

	case Payment:
	    {
		if(gDebug) printf("WriteNewBalance: Writing a new value");
		nbalance = Pay(balance,charge);
		if(gDebug) printf("The balance written by gumstix to device: %u\n", nbalance);
		updateMode();
		currentState = TransactionComplete;
		break;
	    }

	case WriteNewBalance:
	    {
		if(gDebug) printf("WriteNewBalance: Writing %u to tag.\n", newAmount);
		int writeBal = newAmount + balance;
		if(writeBal > 1000) writeBal = 1000;
		//WriteBalance(writeBal);
		nbalance = Pay(writeBal,0);
		updateMode();
		if(autonomous==0)
		{
		    operateDevice = 0;	
		}
		currentState = TransactionComplete;
		break;
	    }                                  

	case TransactionComplete:
	    {
		if(mode!=0)
		{
		    nbalance = ReadBal();
		    updateMode();
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

void Rfid::WriteBalance(unsigned int balance)
{
    char status;
    char WriteBalCommand[9] = {'!','R','W','',''};
    WriteBalCommand[5] = balance & 0xff;
    WriteBalCommand[6] = (balance >> 8) & 0xff;
    WriteBalCommand[7] = (balance >> 16) & 0xff;
    WriteBalCommand[8] = (balance >> 24) & 0xff;

    //Write the balance
    if (gDebug) printf("WriteBalance function: Writing Balance \n");
    if( write( gPortFd, WriteBalCommand, 9 ) != 9 )  {
	printf("Unable to send write balance command \n");
	return;
    }

    if (read( gPortFd, &status, 1 ) != 1) {
	printf("Unable to read device status byte\n");
	return;
    }
    if (status != 1) {
	printf("Received error status byte. The status byte is: %d\n",status);
	return;
    }

    if(gDebug) printf("Successful write.\n");
    nbalance = balance;
} 

unsigned int Rfid::Pay(unsigned int starting_bal, unsigned int amount)
{
    //Calculate the new balance
    unsigned int new_balance = starting_bal-amount;
    if(new_balance > 4000000) new_balance = 0;
    if(gDebug) printf("Pay function: The new balance is %u", new_balance);
    
    //Write the new balance to the tag
    WriteBalance(new_balance);
    return new_balance;
}

unsigned int Rfid::ReadBal()
{
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

unsigned int Rfid::ReadSerialNumber()
{
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
	printf("Received status byte error.  The status byte is: %d\n",status);
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

//Check the status of the buttons by reading the kernel module
int Rfid::getStat()
{
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

    //Multiply the settings to set the values the user can choose
    charge = chargeSetting*5;
    newAmount = newValueSetting*20;

    return 0;
}

//To display the user names to Qt
QString Rfid::DisplayName(unsigned int serial) {
    if (serial == calvin_tag.serial) {
	*qstr = QString::fromStdString(calvin_tag.name);
	return  *qstr;

    }
    if (serial == bryant_tag.serial) {
	*qstr = QString::fromStdString(bryant_tag.name);
	return  *qstr;
    }
    if (serial == ian_tag.serial) {
	*qstr = QString::fromStdString(ian_tag.name);
	return  *qstr;
    }
    qstr->fromStdString("uh oh");
    return *qstr;
}


