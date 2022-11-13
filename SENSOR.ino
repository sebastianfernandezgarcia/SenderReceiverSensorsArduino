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
}

void loop()
{

  //------
   // Serial.print("ranging ...");
  write_command(SRF02_I2C_ADDRESS,REAL_RANGING_MODE_CMS);
  delay(SRF02_RANGING_DELAY);
  
  byte high_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_HIGH_BYTE);
  byte low_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_LOW_BYTE);
  byte high_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_HIGH_BYTE);
  byte low_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_LOW_BYTE);
  
  //Serial.print(int((high_byte_range<<8) | low_byte_range)); Serial.print(" cms. (min=");
  //Serial.print(int((high_min<<8) | low_min)); Serial.println(" cms.)");
  
  delay(1000);



  //-------
  //Serial.println("******************* sending example *******************"); 

  Serial.print("Enviando --> : "); Serial.print(int((high_byte_range<<8) | low_byte_range));
  Serial1.write(int((high_byte_range<<8) | low_byte_range));

  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) 
  { 
    if(Serial1.available()>0) 
    {
      uint8_t data=Serial1.read();
      
      if(data==1){
        Serial.println(". Se envió el dato y recibió la confirmación correctamente");
      }
      else Serial.println(". Se envió el dato pero no hay confirmación de respuesta"); 

      //Serial.print("<-- received: "); Serial.println(static_cast<int>(data)); 
      break;
    }
  }

  if(millis()-last_ms<pseudo_period_ms) delay(pseudo_period_ms-(millis()-last_ms));
  else Serial.println("<-- received: TIMEOUT!!"); 

  Serial.println("*******************************************************"); 

  digitalWrite(LED_BUILTIN,led_state); led_state=(led_state+1)&0x01;
}
