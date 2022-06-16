#include <FlexCAN_T4.h>
FlexCAN_T4<CAN0, RX_SIZE_256, TX_SIZE_16> Can0;

class Btn {
  public:
  int colour;
  int state;
  //int Brightness  Only avaslile in programming mode
};

enum BtnName {P, R, N, D, Ign, Spd, Aux, Tra};
enum Colour {None, Rd, Gr, Bl, Ye, Cy, Mg, WhBl, AmOr, YeGr};
enum State {off, On, Flash};

Btn btns[8];

int prePres[8] = {0,1,2,3,4,5,6,7};

int Ispress = 0;



void setup(void) {
  Serial.begin(115200); delay(400);
  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16);
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();
  // Setup PRND Pins as output
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  // Setup Colours
  btns[P].colour   = Ye;
  btns[R].colour   = AmOr;
  btns[N].colour   = Rd;
  btns[D].colour   = Bl;
  btns[Ign].colour = Rd;
  btns[Spd].colour = Bl;
  btns[Aux].colour = Bl;
  btns[Tra].colour = Bl;

  // Remove any previos colours or states
  for (int i = 0; i<8; i++){
    CAN_message_t Setup;
    Setup.id = 0x18EF2100;
    Setup.buf[0] = 0x04;
    Setup.buf[1] = 0x1B;
    Setup.buf[2] = 0x01;
    Setup.buf[3] = (i+1);
    Setup.buf[4] = btns[i].colour;
    Setup.buf[5] = 0x00;
    Setup.buf[6] = 0xFF;
    Setup.buf[7] = 0xFF;
    Setup.flags.extended = 1;
    Can0.write(Setup);
  }
  
  // Test if 
//  btns[P].colour = Bl;
//  btns[P].state = Flash;
}

void canSniff(const CAN_message_t &msg) {
  for ( int i = 0; i < msg.len; i++ ) {
    // Loop through all of the bytes recived
    if (i == 4){
      Serial.print("Buf: ");
      Serial.print(msg.buf[i]);
      // Set Ispress to show a button has been pressed
      if (msg.buf[i] != 0){
        Ispress = 1;
      }
      // Wait unitl the button has stopped being pressed and then decide weather to go into the switch statment below
      // logic is based on stopping accidentally changning into park when mulitple buttons are being pressed / switched between rapidly
      if (Ispress == 1 && msg.buf[i] == 0) { // only allow in if a button has been pressed and it has now stop being pressed
        if( prePres[0] > 1){ // if the previous press was > 1 i.e not a quick press and not park or muilti press it must be a single button so go 
          Ispress = 2;
        }
        else if (prePres[0] == 1 && prePres[1] == 1){ // if the button has stopped being pressed anf the 
          Ispress = 2;
        }
        else{
          Ispress = 0;
        }
      }
      else if (Ispress == 1 && msg.buf[i] == 0 && prePres[1] == 0){
        Ispress = 0;
      }
      
      if (Ispress == 2){
          Ispress = 0 ;
          switch(prePres[0]){
            case 1: // P
              for (int j = 0; j <4; j++) btns[j].state = off;
              digitalWrite(1, LOW);
              digitalWrite(5, LOW);
              digitalWrite(6, LOW);
              // Set new values
              btns[P].state = On;
              digitalWrite(0, HIGH);
              
              break;
              
            case 2: // R
              for (int j = 0; j <4; j++) btns[j].state = off;
              digitalWrite(0, LOW);
              digitalWrite(5, LOW);
              digitalWrite(6, LOW);
              btns[R].state = On;
              digitalWrite(1, HIGH);
              break;
              
            case 4: // N
              for (int j = 0; j <4; j++) btns[j].state = off;
              digitalWrite(0, LOW);
              digitalWrite(1, LOW);
              digitalWrite(6, LOW);
              btns[N].state = On;
              digitalWrite(5, HIGH);
              break;
              
            case 8: // D
              for (int j = 0; j <4; j++) btns[j].state = off;
              digitalWrite(0, LOW);
              digitalWrite(1, LOW);
              digitalWrite(5, LOW);
              btns[D].state = On;
              digitalWrite(6, HIGH);
              break;
              
            case 16: // Ign
              if (btns[Ign].state == On){
                 btns[Ign].state = off;
              }
             else{
                btns[Ign].state = On;
             }
             
              break;
              
            case 32: // Spd
              break;
              
            case 64: // Tra
              break;
              
            default:
            // Do nothing as more than one button pressed
            // Button presses do add thorugh, so P+N = 5
            // Can Detect any number of buttons being pressed at the same time
              break;  
         
        }
      }
     Serial.println(); 
    }
   //Serial.print(msg.buf[i], HEX); Serial.print(" ");
    
  } 
  //Serial.println();
  for(int k = 7; k > 0; k--){
    prePres[k] = prePres[k-1];
  }
  prePres[0] = msg.buf[4];
  Serial.print(" Array: ");
  for(int k = 0; k < 8; k++) Serial.print(prePres[k]);
  
  Serial.println();
  
  
}

void loop() {
  Can0.events();

  static uint32_t timeout = millis();
  if ( millis() - timeout > 200 ) {
    for (int i = 0; i < 8; i++){
      CAN_message_t msg;
      msg.id = 0x18EF2100;
      msg.buf[0] = 0x04;
      msg.buf[1] = 0x1B;
      msg.buf[2] = 0x01;
      msg.buf[3] = (i+1);
      msg.buf[4] = btns[i].colour;
      msg.buf[5] = btns[i].state;
      msg.buf[6] = 0xFF;
      msg.buf[7] = 0xFF;
      msg.flags.extended = 1;
      Can0.write(msg);
    }
    
    //Serial.print("Message Sent \n\r");
    timeout = millis();
  }

}
