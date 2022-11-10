/* ----------------------------------------------------------------------
 *  Ejemplo sending_example.ino 
 *    Este ejemplo muestra como utilizar el puerto serie uart (Serial1) 
 *    para comunicarse con otro dispositivo.
 *    
 *  Asignatura (GII-IC)
 * ---------------------------------------------------------------------- 
 */

//------------------------IMPORTACIONES SENSOR-------------------
#include <Wire.h> // Arduino's I2C library
 
#define SRF02_I2C_ADDRESS byte((0xE0)>>1)
#define SRF02_I2C_INIT_DELAY 100 // in milliseconds
#define SRF02_RANGING_DELAY 70 // milliseconds

#define SRF04_I2C_ADDRESS byte((0xF0)>>1)
#define SRF04_I2C_INIT_DELAY 200 // in milliseconds
#define SRF04_RANGING_DELAY 140 // milliseconds

// LCD05's command related definitions
#define COMMAND_REGISTER byte(0x00)
#define SOFTWARE_REVISION byte(0x00)
#define RANGE_HIGH_BYTE byte(2)
#define RANGE_LOW_BYTE byte(3)
#define AUTOTUNE_MINIMUM_HIGH_BYTE byte(4)
#define AUTOTUNE_MINIMUM_LOW_BYTE byte(5)

// SRF02's command codes
#define REAL_RANGING_MODE_INCHES    byte(80)
#define REAL_RANGING_MODE_CMS       byte(81)
#define REAL_RANGING_MODE_USECS     byte(82)
#define FAKE_RANGING_MODE_INCHES    byte(86)
#define FAKE_RANGING_MODE_CMS       byte(87)
#define FAKE_RANGING_MODE_USECS     byte(88)
#define TRANSMIT_8CYCLE_40KHZ_BURST byte(92)
#define FORCE_AUTOTUNE_RESTART      byte(96)
#define ADDRESS_CHANGE_1ST_SEQUENCE byte(160)
#define ADDRESS_CHANGE_3RD_SEQUENCE byte(165)
#define ADDRESS_CHANGE_2ND_SEQUENCE byte(170)
//----------------------------FIN IMPORTACIONES SENSOR-----------------------


constexpr const uint32_t serial_monitor_bauds=115200;
constexpr const uint32_t serial1_bauds=9600;

constexpr const uint32_t pseudo_period_ms=1000;

uint8_t counter=0;
uint8_t led_state=LOW;


//---------------------- FUNCIONES SENSOR------------------------
inline void write_command(byte address,byte command)
{ 
  Wire.beginTransmission(address);
  Wire.write(COMMAND_REGISTER); 
  Wire.write(command); 
  Wire.endTransmission();
}

byte read_register(byte address,byte the_register)
{
  Wire.beginTransmission(address);
  Wire.write(the_register);
  Wire.endTransmission();
  
  // getting sure the SRF02 is not busy
  Wire.requestFrom(address,byte(1));
  while(!Wire.available()) { /* do nothing */ }
  return Wire.read();
} 

 //-----------------------------FIN FUNCIONES SENSOR=--------------------------------

 
void setup()
{

  //------------SET UP SENDERRECEIVER------------------------------
  // Configuración del LED incluido en placa
  // Inicialmente apagado
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,led_state); led_state=(led_state+1)&0x01;
  
  // Inicialización del puerto para el serial monitor 
  Serial.begin(serial_monitor_bauds);
  while (!Serial);

  // Inicialización del puerto de comunicaciones con el otro dispositivo MKR 
  Serial1.begin(serial1_bauds);

  //-----FIN DE SETUP DE SENDERRECIVER--------------------




  //------------------SET UP DEL SENSOR----------------------
  Serial.begin(9600);
  
  Serial.println("initializing Wire interface ...");
  Wire.begin();
  delay(SRF02_I2C_INIT_DELAY);  
   
  byte software_revision=read_register(SRF02_I2C_ADDRESS,SOFTWARE_REVISION);
  Serial.print("SFR02 ultrasonic range finder in address 0x");
  Serial.print(SRF02_I2C_ADDRESS,HEX); Serial.print("(0x");
  Serial.print(software_revision,HEX); Serial.println(")");

  //----------------FIN DE SET UP DEL SENSOR---------------

  //------------------SET UP DEL SENSOR 2---------------------
  delay(SRF04_I2C_INIT_DELAY);  
   
  byte software_revision_2=read_register(SRF04_I2C_ADDRESS,SOFTWARE_REVISION);
  Serial.print("SFR04 ultrasonic range finder in address 0x");
  Serial.print(SRF04_I2C_ADDRESS,HEX); Serial.print("(0x");
  Serial.print(software_revision_2,HEX); Serial.println(")");
  //----------------FIN DE SET UP DEL SENSOR---------------
}

void loop()
{

  //------
  Serial.print("ranging 1...");
  write_command(SRF02_I2C_ADDRESS,REAL_RANGING_MODE_CMS);
  delay(SRF02_RANGING_DELAY);
  
  byte high_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_HIGH_BYTE);
  byte low_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_LOW_BYTE);
  byte high_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_HIGH_BYTE);
  byte low_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_LOW_BYTE);
  
  Serial.print(int((high_byte_range<<8) | low_byte_range)); Serial.print(" cms. (min=");
  Serial.print(int((high_min<<8) | low_min)); Serial.println(" cms.)");
  

  Serial.print("ranging 2...");
  write_command(SRF04_I2C_ADDRESS,REAL_RANGING_MODE_CMS);
  delay(SRF04_RANGING_DELAY);
  
  byte high_byte_range_2=read_register(SRF04_I2C_ADDRESS,RANGE_HIGH_BYTE);
  byte low_byte_range_2=read_register(SRF04_I2C_ADDRESS,RANGE_LOW_BYTE);
  byte high_min_2=read_register(SRF04_I2C_ADDRESS,AUTOTUNE_MINIMUM_HIGH_BYTE);
  byte low_min_2=read_register(SRF04_I2C_ADDRESS,AUTOTUNE_MINIMUM_LOW_BYTE);
  
  Serial.print(int((high_byte_range_2<<8) | low_byte_range_2)); Serial.print(" cms. (min=");
  Serial.print(int((high_min_2<<8) | low_min_2)); Serial.println(" cms.)");

  //-------
  //Serial.println("******************* sending example *******************"); 

  //Serial.print("Enviando 1--> : "); Serial.println(int((high_byte_range<<8) | low_byte_range));
  //Serial1.write(int((high_byte_range<<8) | low_byte_range));

  //Serial.print("Enviando 2--> : "); Serial.println(int((high_byte_range_2<<8) | low_byte_range_2));
  //Serial1.write(int((high_byte_range_2<<8) | low_byte_range_2));
  
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) 
    {
      int data_len = 50;
      byte data [data_len];
      int rlen = Serial1.readBytes(data, data_len);
      SerialUSB.println("Leyendo...");
      if((int)data[0]==1){      // Orden de un disparo
        Serial.print("Ejecutando orden oneshot");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04");
        } else {
          Serial.print(" con el sensor srf02");
        }
        if ((int)data[2]==2) {
          Serial.print(" opcion on");
          SerialUSB.print(" con un delay de ");
          SerialUSB.println((int)data[3]);
        } else if ((int)data[2]==3) {
          Serial.println(" opcion off");
        } else {
          Serial.println(" opcion oneshot");
        }
        char res []= {'o', 'k'};
        Serial1.write(res, 2);
      } else if (data[0]==2) {    // Orden para cambiar unidades
        Serial.print("Ejecutando orden chaneUnits");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04");
        } else {
          Serial.print(" con el sensor srf02");
        }
        if ((int)data[2]==2) {
          Serial.println(" cambiando a inc");
        } else if ((int)data[2]==3) {
          Serial.println(" cambiando a ms");
        } else {
          Serial.println(" cambiando a cm");
        }
        char res []= {'o', 'k'};
        Serial1.write(res, 2);
      } else if(data[0]==3) {     // Orden para cambiar retardo entre disparos
        Serial.print("Ejecutando orden delay");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04");
        } else {
          Serial.print(" con el sensor srf02");
        }
        Serial.print(" con un valor de ");
        SerialUSB.println((int)data[3]);
        char res []= {'o', 'k'};
        Serial1.write(res, 2);
      } else if(data[0]==4) {     // Orden para obtener configuración del sensor
        Serial.print("Ejecutando orden status");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04");
        } else {
          Serial.print(" con el sensor srf02");
        }
        char res []= {'o', 'k'};
        Serial1.write(res, 2);
      } else if (data[0]==5) {    // Orden para mostrar lista de sensores
        Serial.print("Ejecutando orden us");
        char res []= {'o', 'k'};
        Serial1.write(res, 2);
      } 
      break;
    }
  }

  digitalWrite(LED_BUILTIN,led_state); led_state=(led_state+1)&0x01;
}
