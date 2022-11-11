/*
 * Implementacion de interfaz para control de sensores
 */

// Banderas de control para la configuración de las unidades de medición de los sensores
volatile boolean inc_flag_2=false;
volatile boolean ms_flag_2=false;
volatile boolean inc_flag_4=false;
volatile boolean ms_flag_4=false;
// Banderas de control para la configuración del estado de los sensores
volatile boolean off_2=false;
volatile boolean off_4=false;
volatile boolean one_shot_2=false;
volatile boolean one_shot_4=false;

//------------------------IMPORTACIONES SENSOR-------------------
#include <Wire.h> // Arduino's I2C library

int data_len=3; // Logintud del buffer de escritura
 
#define SRF02_I2C_ADDRESS byte((0xE0)>>1)
#define SRF02_I2C_INIT_DELAY 100 // in milliseconds
int SRF02_RANGING_DELAY=70; // milliseconds

#define SRF04_I2C_ADDRESS byte((0xF0)>>1)
#define SRF04_I2C_INIT_DELAY 200 // in milliseconds
int SRF04_RANGING_DELAY=140; // milliseconds

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

constexpr const uint32_t pseudo_period_ms=1100;

uint8_t counter=0;
uint8_t led_state=LOW;


//---------------------- FUNCIONES SENSOR------------------------
inline void write_command(byte address,byte command){ 
  Wire.beginTransmission(address);
  Wire.write(COMMAND_REGISTER); 
  Wire.write(command); 
  Wire.endTransmission();
}

byte read_register(byte address,byte the_register){
  Wire.beginTransmission(address);
  Wire.write(the_register);
  Wire.endTransmission();
  
  // getting sure the SRF02 is not busy
  Wire.requestFrom(address,byte(1));
  while(!Wire.available()) { /* do nothing */ }
  return Wire.read();
} 

 //-----------------------------FIN FUNCIONES SENSOR=--------------------------------

 
void setup(){

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

void loop() {

  byte data_b[data_len];  // Bufer de escritura por I2C para un array de byte
  uint8_t akc_data; // Recibidor de ack para confirmación de comunicación correcta

  if (off_2 && off_4) { // Si las banderas de estado apagado de ambos sensores estan a true informamos de ello
    Serial.println("Los dos sensores se encuentran apagados, esperando a que se encienda alguno...");
  }
  
  //------
  if (!off_2) {  // Si el sensor srf02 no esta en estado apagado
    Serial.print("ranging 1...");
    data_b[0]=1;
    if (inc_flag_2 && !ms_flag_2) {   // En este caso configuramos el sensor srf02 para que mida en inc
      write_command(SRF02_I2C_ADDRESS,REAL_RANGING_MODE_INCHES);
      data_b[2]=3;
    } else if (!inc_flag_2 && ms_flag_2) {    // En este caso configuramos el sensor srf02 para que mida en ms
      write_command(SRF02_I2C_ADDRESS,REAL_RANGING_MODE_USECS);
      data_b[2]=2;
    } else {    // Por defecto configuramos el sensor srf02 para que mida en cm
      write_command(SRF02_I2C_ADDRESS,REAL_RANGING_MODE_CMS);
      data_b[2]=1;
    }
    delay(SRF02_RANGING_DELAY);
    
    byte high_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_HIGH_BYTE);
    byte low_byte_range=read_register(SRF02_I2C_ADDRESS,RANGE_LOW_BYTE);
    byte high_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_HIGH_BYTE);
    byte low_min=read_register(SRF02_I2C_ADDRESS,AUTOTUNE_MINIMUM_LOW_BYTE);
    
    Serial.print(int((high_byte_range<<8) | low_byte_range));
    data_b[1]=((high_byte_range<<8) | low_byte_range); 
    if (inc_flag_2 && !ms_flag_2) {   // En este caso significa que el sensor srf02 está midiendo en inc
      Serial.print(" inc. (min=");
    } else if (!inc_flag_2 && ms_flag_2) {  // En este caso significa que el sensor srf02 está midiendo en ms
      Serial.print(" ms. (min=");
    } else {    // Por defecto el sensor srf02 está midiendo en cm
      Serial.print(" cms. (min="); 
    }
    Serial.print(int((high_min<<8) | low_min));
    if (inc_flag_2 && !ms_flag_2) {   // En este caso significa que el sensor srf02 está midiendo en inc
      Serial.println(" inc.)");
    } else if (!inc_flag_2 && ms_flag_2) {  // En este caso significa que el sensor srf02 está midiendo en ms
      Serial.println(" ms.)");
    } else {    // Por defecto el sensor srf02 está midiendo en cm
      Serial.println(" cms.)"); 
    }
    Serial.print("Enviando 1--> : "); 
    Serial.print(data_b[0]);
    Serial.print(", ");
    Serial.print(data_b[1]);
    Serial.print(", ");
    Serial.print(data_b[2]);
    Serial1.write(data_b, data_len);
    confirmation(akc_data); //  Confirmamos que se envio la medicion al supervisor correctamente
    if (one_shot_2) { // Si el sensor srf02 se le solicita un solo disparo
      one_shot_2 = false; // Devolvemos la bandera de un disparo a false
      off_2 = true; // Volvemos a apagar el sensor 
    }
  }

  if (!off_4) { // Si el sensor srf04 no esta en estado apagado
    Serial.print("ranging 2...");
    data_b[0]=2;
    if (inc_flag_4 && !ms_flag_4) {   // En este caso configuramos el sensor srf04 para que mida en inc
      write_command(SRF04_I2C_ADDRESS,REAL_RANGING_MODE_INCHES);
      data_b[2]=3;
    } else if (!inc_flag_4 && ms_flag_4) {    // En este caso configuramos el sensor srf04 para que mida en ms
      write_command(SRF04_I2C_ADDRESS,REAL_RANGING_MODE_USECS);
      data_b[2]=2;
    } else {    // Por defecto configuramos el sensor srf04 para que mida en cm
      write_command(SRF04_I2C_ADDRESS,REAL_RANGING_MODE_CMS);
      data_b[2]=1;
    }
    delay(SRF04_RANGING_DELAY);
    
    byte high_byte_range_2=read_register(SRF04_I2C_ADDRESS,RANGE_HIGH_BYTE);
    byte low_byte_range_2=read_register(SRF04_I2C_ADDRESS,RANGE_LOW_BYTE);
    byte high_min_2=read_register(SRF04_I2C_ADDRESS,AUTOTUNE_MINIMUM_HIGH_BYTE);
    byte low_min_2=read_register(SRF04_I2C_ADDRESS,AUTOTUNE_MINIMUM_LOW_BYTE);
    
    Serial.print(int((high_byte_range_2<<8) | low_byte_range_2));
    data_b[1]=((high_byte_range_2<<8) | low_byte_range_2);
    if (inc_flag_4 && !ms_flag_4) {   // En este caso significa que el sensor srf04 está midiendo en inc
      Serial.print(" inc. (min=");
    } else if (!inc_flag_4 && ms_flag_4) {  // En este caso significa que el sensor srf04 está midiendo en ms
      Serial.print(" ms. (min=");
    } else {    // Por defecto el sensor srf04 está midiendo en cm
      Serial.print(" cms. (min="); 
    }
    Serial.print(int((high_min_2<<8) | low_min_2));
    if (inc_flag_4 && !ms_flag_4) {   // En este caso significa que el sensor srf04 está midiendo en inc
      Serial.println(" inc.)");
    } else if (!inc_flag_4 && ms_flag_4) {  // En este caso significa que el sensor srf04 está midiendo en ms
      Serial.println(" ms.)");
    } else {    // Por defecto el sensor srf04 está midiendo en cm
      Serial.println(" cms.)"); 
    }
    Serial.print("Enviando 2--> : ");
    Serial.print(data_b[0]);
    Serial.print(", ");
    Serial.print(data_b[1]);
    Serial.print(", ");
    Serial.print(data_b[2]);
    Serial1.write(data_b, data_len);
    confirmation(akc_data); //  Confirmamos que se envio la medicion al supervisor correctamente
    if (one_shot_4) { // Si el sensor srf04 se le solicita un solo disparo
      one_shot_4 = false; // Devolvemos la bandera de un disparo a false
      off_4 = true;   // Volvemos a apagar el sensor 
    }
  }

  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {
      int data_len = 50;
      byte data [data_len];
      int rlen = Serial1.readBytes(data, data_len);
      if((int)data[0]==1){      // Orden de un disparo
        Serial.print("Ejecutando orden oneshot");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04");
          if ((int)data[2]==2) {
            Serial.print(" opcion on");
            SerialUSB.print(" con un delay de ");
            if ((int)data[3] > 0) {
              SRF04_RANGING_DELAY = (int)data[3]; 
            }
            SerialUSB.println(SRF04_RANGING_DELAY);
            off_4 = false;
          } else if ((int)data[2]==3) {
            Serial.println(" opcion off");
            off_4 = true;
          } else {
            Serial.println(" opcion oneshot");
            off_4 = false;
            one_shot_4 = true;
          }
        } else {
          Serial.print(" con el sensor srf02");
          if ((int)data[2]==2) {
            Serial.print(" opcion on");
            SerialUSB.print(" con un delay de ");
            if ((int)data[3] > 0) {
              SRF02_RANGING_DELAY = (int)data[3]; 
            }
            SerialUSB.println(SRF02_RANGING_DELAY);
            off_2 = false;
          } else if ((int)data[2]==3) {
            Serial.println(" opcion off");
            off_2 = true;
          } else {
            Serial.println(" opcion oneshot");
            off_2 = false;
            one_shot_2 = true;
          }
        }
        char res []= {'o', 'k'};
        Serial1.write(res, 2); // Enviamos un char array con mensaje de confirmación del cambio
      } else if (data[0]==2) {    // Orden para cambiar unidades
        Serial.print("Ejecutando orden changeUnits");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04");
          if ((int)data[2]==2) {
            Serial.println(" cambiando a inc");
            inc_flag_4 = true;
            ms_flag_4 = false;
          } else if ((int)data[2]==3) {
            Serial.println(" cambiando a ms");
            inc_flag_4 = false;
            ms_flag_4 = true;
          } else {
            Serial.println(" cambiando a cm");
            inc_flag_4 = false;
            ms_flag_4 = false;
          }
        } else {
          Serial.print(" con el sensor srf02");
          if ((int)data[2]==2) {
            Serial.println(" cambiando a inc");
            inc_flag_2 = true;
            ms_flag_2 = false;
          } else if ((int)data[2]==3) {
            Serial.println(" cambiando a ms");
            inc_flag_2 = false;
            ms_flag_2 = true;
          } else {
            Serial.println(" cambiando a cm");
            inc_flag_2 = false;
            ms_flag_2 = false;
          }
        }
        char res []= {'o', 'k'};
        Serial1.write(res, 2); // Enviamos un char array con mensaje de confirmación del cambio
      } else if(data[0]==3) {     // Orden para cambiar retardo entre disparos
        Serial.print("Ejecutando orden delay");
        if ((int)data[1]==2) {
          Serial.print(" con el sensor srf04 con un valor de ");
          if ((int)data[3] > 0) {
            SRF04_RANGING_DELAY = (int)data[3]; 
          }
          SerialUSB.println(SRF04_RANGING_DELAY);
        } else {
          Serial.print(" con el sensor srf02 con un valor de ");
          if ((int)data[3] > 0) {
            SRF02_RANGING_DELAY = (int)data[3]; 
          }
          SerialUSB.println(SRF02_RANGING_DELAY);
        }
        char res []= {'o', 'k'};
        Serial1.write(res, 2);
      } else if(data[0]==4) {     // Orden para obtener configuración del sensor
        Serial.print("Ejecutando orden status");
        byte res [3]; // Array de byte para almacenar la información de configuración a enviar al supervisor
        // En la posición 0 almacenamos 2 si es el sensor srf04 y 1 si es el sensor srf02
        // En la posición 1 almacenamos un 1 si el sensor mide en cm, un 2 si mide en ms y un 3 si mide en inc
        // En la posición 2 almacenamos el número de ms que tiene el sensor configurado como delay
        if ((int)data[1]==2) {
          Serial.println(" con el sensor srf04");
          res[0] = 2;
          if (inc_flag_4 && !ms_flag_4) {
            res[1] = 3;
          } else if (!inc_flag_4 && ms_flag_4) {
            res[1] = 2;
          } else {
            res[1] = 1;
          }
          res[2] = SRF04_RANGING_DELAY;
        } else {
          Serial.println(" con el sensor srf02"); 
          res[0] = 1;
          if (inc_flag_2 && !ms_flag_2) {
            res[1] = 3;
          } else if (!inc_flag_2 && ms_flag_2) {
            res[1] = 2;
          } else {
            res[1] = 1;
          }
          res[2] = SRF02_RANGING_DELAY;
        }
        Serial1.write(res, 3);
      } else if (data[0]==5) {    // Orden para mostrar lista de sensores
        Serial.println("Ejecutando orden us");
        char res []= {'s', 'r', 'f', '0', '2', ',', ' ', 's', 'r', 'f', '0', '4'}; // Enviamos un array de char con los sensores disponibles
        Serial1.write(res, 12);
      } 
      break;
    }
  }

  digitalWrite(LED_BUILTIN,led_state); led_state=(led_state+1)&0x01;
}

void confirmation(uint8_t data) { // Método para la comprobación de conexión con el supervisor
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {
      data=Serial1.read();
      if (static_cast<int>(data) == 6) {    // Si se recibe un valor 6 (valor declarado de akc) la conexión es correcta
        Serial.println("; <-- akc received!!");
      } else {
        Serial.println("; <-- Hubo un problema de comunicación");
      }
      break;
    }
  }
  if(millis()-last_ms<pseudo_period_ms) {
    delay(pseudo_period_ms-(millis()-last_ms));
  } else {      // Si pasa el tiempo de respuesta maximo avisamos del timeout
    Serial.println("; <-- akc received: TIMEOUT!!");
  }
}
