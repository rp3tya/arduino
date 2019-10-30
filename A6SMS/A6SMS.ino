// https://lastminuteengineers.com/a6-gsm-gprs-module-arduino-tutorial/
#include <SoftwareSerial.h>

//Create software serial object to communicate with A6
SoftwareSerial mySerial(D1, D2); //A6 Tx & Rx is connected to Arduino #3 & #2
//                      RX, TX

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and A6
  mySerial.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();

//  readSMS();
//  sendSMS();
}

void loop()
{
  updateSerial();
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void sendSMS() {
  // AT+CMGF=1 – Selects SMS message format as text. Default format is Protocol Data Unit (PDU)
  // AT+CMGS=+ZZxxxxxxxxxx – Sends SMS to the phone number specified. The text message entered 
  //             followed by a ‘Ctrl+z’ character is treated as SMS. ‘Ctrl+z’ is actually a 26th 
  //             non-printing character described as ‘substitute’ in ASCII table. So, we need 
  //             to send 26DEC (1AHEX) once we send a message.
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+ZZxxxxxxxxx\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  mySerial.print("test"); //text content
  updateSerial();
  mySerial.write(26); // Ctrl+Z
}

void readSMS() {
  // AT+CMGF=1 – Selects SMS message format as text. Default format is Protocol Data Unit (PDU)
  // AT+CNMI=1,2,0,0,0 – specifies how newly arrived SMS messages should be handled. This way 
  //             you can tell the A6 module either to forward newly arrived SMS messages directly
  //             to the PC, or to save them in message storage and then notify the PC about their 
  //             locations in message storage.
  // Its response starts with +CMT: All the fields in the response are comma-separated with first 
  // field being phone number. The second field is the name of person sending SMS. Third field is 
  // a timestamp while forth field is the actual message.
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CSCS=\"GSM\""); //
  updateSerial();
//  mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
//  updateSerial();
  // If your SoftwareSerial receive buffer is getting filled up and discarding characters, you are 
  // not reading fast enough from the buffer.
  // Go to ... -> Arduino -> libraries -> SoftwareSerial -> src, Open SoftwareSerial.h and change the line:
  // #define _SS_MAX_RX_BUFF 64 => 512
}

void makeCall(){
  // To place a call following AT commands are used:
  //   ATD+ZZxxxxxxxxxx; – Dials a specified number.
  //   ATH – Hangs up the call
  mySerial.println("ATD+ZZxxxxxxxxxx"); //  change ZZ with country code and xxxxxxxxxxx with phone number to dial
  updateSerial();
  delay(20000); // wait for 20 seconds...
  mySerial.println("ATH"); //hang up
  updateSerial();
}

void receiveCall() {
  // Incoming call is usually indicated by ‘RING’ on serial monitor followed by phone number and caller ID. 
  // To accept/hang a call following AT commands are used:
  //   ATA – Accepts incoming call.
  //   ATH – Hangs up the call. On hanging up the call it sends NO CARRIER on the serial monitor indicating call couldn’t connect.
}
