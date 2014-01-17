#ifndef RFID_H
#define RFID_H

#include <QtGui/QApplication>
#include <QWidget>
#include <QProcess>
#include <QLineEdit>
#include <QFormLayout>
#include <QLabel>
#include <QFont>
#include <QTimer>
#include <QStringList>
#include <QMainWindow> 


class Rfid : public QWidget 
{
    Q_OBJECT

    public:
	explicit Rfid(QWidget *parent = 0, int gPort = -1); 

	private slots:  

	    void updateMode();
	void setWelcomePage();
	void setReadPage();
	void setWritePage();
	void setUpdatePage();
	void clearScreen();
	void UART();
	void WriteBalance(unsigned int balance); 
	unsigned int Pay(unsigned int starting_bal, unsigned int amount);
	unsigned int ReadBal();
	unsigned int ReadSerialNumber();
	int getStat();                   
	QString DisplayName(unsigned int serial);

    private:

	int mode,operateDevice,autonomous,chargeSetting;
	int newValueSetting,gDebug,gPortFd;
	unsigned int serialNum,balance,lastSerial;
	unsigned int charge,nbalance;
	int newAmount;

	typedef enum {ReadSerial, ReadBalance, Payment, WriteNewBalance, TransactionComplete} state;
	state currentState;
	struct tag{
	    std::string name;
	    unsigned int serial;
	} calvin_tag,bryant_tag,ian_tag;

	QTimer *uartTimer;


	QLabel *welcome;
	QLineEdit *name;
	QLineEdit *qbalance;
	QLineEdit *cost;
	QLineEdit *newqbalance;
	QLineEdit *amountAdd;

	QLabel *nameText;
	QLabel *qbalanceText;
	QLabel *costText;
	QLabel *newqbalanceText;
	QLabel *amountAddText;
	QString *qstr;
};

#endif
