/// This code is for personal use not commercial,  developed by Abdelhakim Dh

#include <RCSwitch.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  // CE,   CSN
const byte address[6] = "00001";
RCSwitch mySwitch = RCSwitch();
SoftwareSerial MyBlue(6, 5);  // RX | TX


long interval = 14900;  // interval for the gate to open completly

boolean button_state = 0;
long previousMillis = 0;
int open = 0;   // is the door open
int run = 0;    // is the door mouving
int timer = 0;   
const int relayPin = 3;   // for opening 
const int relayPin2 = 4;  // for closing
int flag = 0;     // BLUETOOTH recived msg

void setup() {
  Serial.begin(9600);
  MyBlue.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);  //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MAX);      //You can set this as minimum or maximum depending on the distance between   the transmitter and receiver.
  radio.startListening();             //This sets   the module as receiver
  mySwitch.enableReceive(0);          // RF433 Receiver input on interrupt 0 (D2)
  mySwitch.enableTransmit(5);         // RF433 Transmit output on interrupt 5 (D8)
  pinMode(relayPin, OUTPUT);   // relayPin as output- Optional
  pinMode(relayPin2, OUTPUT);  // relayPin2 as output- Optional
  digitalWrite(relayPin, HIGH);  // My relays are inversed high mean OFF
  digitalWrite(relayPin2, HIGH);  // My re----
}

void loop() {
  unsigned long currentMillis = millis();
  if (mySwitch.available()) {
    Serial.print("ID:");             
    Serial.println(mySwitch.getReceivedValue());          //To print remote ID
    int long value = mySwitch.getReceivedValue();              //To store recived ID
    delay(400);            // to prevent doble click
    ////////////////////////////////////////////////REMOTE///////////////////////////////////////////////////////////
    /// change value with your recived remote code
    if (open == 0 && (value == 7747348 || value == 10888932 || value == 6917476)) {
      Serial.println("remote open");
      run = 1;
      open = 1;
      digitalWrite(relayPin, LOW);
      digitalWrite(relayPin2, HIGH);
      mySwitch.send(5393, 24);       //send a feedback over NRF 
      previousMillis = currentMillis;
      // secend remote click while door is mouving will stop it
    } else if (run == 1 && (value == 7747348 || value == 10888932 || value == 6917476)) {
      digitalWrite(relayPin, HIGH);
      run = 0;
      Serial.println("stop");
      mySwitch.send(4393, 24);   //send a feedback over NRF 
    }
    mySwitch.resetAvailable();  //clear recived code
    //----------------------- close ----
    if (open == 1 && (value == 7747346 || value == 10888930 || value == 6917480)) {
      Serial.println("remote close");
      digitalWrite(relayPin2, LOW);
      digitalWrite(relayPin, HIGH);

      run = 1;
      open = 0;
      mySwitch.send(5393, 24);
      previousMillis = currentMillis;
      // secend remote click while door is mouving will stop it
    } else if (run == 1 && (value == 7747346 || value == 10888930 || value == 6917480)) {
      digitalWrite(relayPin2, HIGH);
      run = 0;
      mySwitch.send(4393, 24); //send a feedback over NRF
      mySwitch.resetAvailable();  //clear recived code
    }
    mySwitch.resetAvailable(); //clear unwanted remote codes
  }
  ///////////////////////////////////////BLUETOOTH///////////////////////////////////////////////////////////
  if (MyBlue.available() > 0) {
    flag = MyBlue.read();
    delay(500);
    {
      //Serial.print(flag);
      if (flag == 3 && open == 1) {
        Serial.println("open blut");
        digitalWrite(relayPin2, LOW);
        digitalWrite(relayPin, HIGH);
        run = 1;
        open = 0;
        flag = 0;
        previousMillis = currentMillis;
      } else if (run == 1 && flag == 3) {
        Serial.println("stop blut");
        digitalWrite(relayPin2, HIGH);
        run = 0;
        flag = 0;
      }
      //-------------------
      if (flag == 2 && open == 0) {
        Serial.println("close blut");
        digitalWrite(relayPin, LOW);
        digitalWrite(relayPin2, HIGH);
        run = 1;
        open = 1;
        flag = 0;
        previousMillis = currentMillis;
      } else if (run == 1 && flag == 2) {
        Serial.println("stop blut");
        digitalWrite(relayPin, HIGH);
        run = 0;
        flag = 0;
      }
    }
  }
  ////////////////////////////////////////////RADIO RF///////////////////////////////////////////////////////////////////
  if (radio.available())  //Looking   for the data.
  {
    radio.read(&button_state,   sizeof(button_state));
      Serial.println(button_state);                
      if (open ==0 && button_state == 1) {
        Serial.println("open radio");
        digitalWrite(relayPin, LOW);
        digitalWrite(relayPin2, HIGH);
        run = 1;
        open = 1;
        previousMillis = currentMillis;
    }
    else if (run == 1 && button_state == 1) {
        Serial.println("stop radio");
        digitalWrite(relayPin, HIGH);
        run = 0;
      }
  if (open ==1 &&button_state == 0) {
      Serial.println("close radio");
        digitalWrite(relayPin2, LOW);
        digitalWrite(relayPin, HIGH);
        run = 1;
        open = 0;
        previousMillis = currentMillis;
    }
    else if (run == 1 && button_state == 0) {
        Serial.println("stop radio");
        digitalWrite(relayPin2, HIGH);
        run = 0;
    }
    }
  /////////////////////////////////////////////////TIMER///////////////////////////////////////////////////////////////////
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(relayPin, HIGH);
    digitalWrite(relayPin2, HIGH);
    flag = 0;
    run = 0;
    // mySwitch.send(4393, 24);
    Serial.print("run state:");
    Serial.print(run);
    Serial.print("  open state:");
    Serial.println(open);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}