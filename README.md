RFIDTransactionUnit
===================

Radio Frequency Identification Transaction Unit using Parallax RFID module, gumstix, and LCD screen

Authors: Bryant Moquist, Ian Bablewski, Calvin Flegal

#####Included:
* Final Project Report
* Project Photos
* Key Resources
* Source Code
 
#####Final Project Report: 
Detailed description of the project with diagrams and photographs. 

#####Key Resources: 
This folder contains two pieces of sample code that were useful during the project.  The links are listed in the reference sections of the report.

#####Source Code:
*Kernel module*:  Compile the kRFID.ko kernel module using the Makefile.  Transfer kRFID to the gumstix with Hyperterminal.  

To insert the module from the command line, type:
```
mknod /dev/kRFID c 61 0
insmod(kRFID)
```

To remove the module:
```
rmmod(kRFID).
```

*User space program*:  Compile using the Makefile.  Then, transfer to the Gumstix using Hyperterminal.  Run the program with the following command:
```
./newqt -qws
```

To run the program:
```
1. Transfer the appropriate compiled files to the Gumstix with Hyperterminal.
2. Make a device driver node.
3. Insert the kernel module.
4. Run the userspace program.
```
*Command Line Application (No QT)*:  Also find included a c program that runs only the RFID module and the buttons interface.  This was an intermediate product of our project that we used to interface with the RFID module before we integrated with QT.  

Example compile on Linux:
$EC535/gumstix/oe/cross/bin/arm-linux-gcc RFIDwithButtons.c -o RFIDwithButtons

To run the program:
```
1. Transfer the compiled kernel module and the compiled RFIDwithButtons file to the Gumstix.
2. Make a device driver node.
3. Insert the kernel module.
4. Run the RFIDwithButtons with the appropriate flag.  To see a list of flag options for the modes and additional input options, type: "./RFIDwithButtons -h"  The default setting is to read the serial number in an infinite loop. 
```

