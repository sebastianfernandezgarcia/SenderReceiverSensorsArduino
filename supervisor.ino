/*
 * Interfaz de supervisor de un conjunto de sensores
 */
 
#include <Regexp.h>

#define ack 6   // Confirmador de comunicación correcta

#define data_len 50 // Logintud del buffer de lectura

constexpr const uint32_t serial_monitor_bauds=115200;
constexpr const uint32_t serial1_bauds=9600;

constexpr const uint32_t pseudo_period_ms=1000;

uint8_t led_state=LOW;

void setup(){
  // Configuración del LED incluido en placa
  // Inicialmente apagado
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,led_state); led_state=(led_state+1)&0x01;
  
  // Inicialización del puerto para el serial monitor 
  Serial.begin(serial_monitor_bauds);
  while (!Serial);

  // Inicialización del puerto de comunicaciones con el otro dispositivo MKR 
  Serial1.begin(serial1_bauds);
}

void loop(){ 

  if(Serial.available()>0) { // Si se encuentra algo que leer desde el monitor serie
    String input = Serial.readStringUntil('\n'); //Leemos hasta que se encuentra un salto de línea

    char Buf[50];
    input.toCharArray(Buf, 50);

    MatchState ms;
    ms.Target(Buf);

    char result1 = ms.Match("^us [a-z0-9]+ [(oneshot)+|(on)? 0-9|off]+$"); //este no he podido hacerlo con one-shot por lo que lo he puesto como oneshot
    char result2 = ms.Match("^us [a-z0-9]+ unit [(inc|cm|ms)]+$"); 
    char result3 = ms.Match("^us [a-z0-9]+ delay [0-9]+$"); 
    char result4 = ms.Match("^us [a-z0-9]+ status$"); 

    // En todas las ordenes pasamos a los metodos parametros de tipo byte []
    // Siendo en todos los casos a posición 0 del array el codigo de orden
    // la posición 1 el sensor con el que se trabajará
    // y las ordenes que lo necesitan la posición 2 es la opcion elejida de la orden
    // la posición 3 solo se usa en la orden oneshot para los ms del retardo de la opcion 'on'
    
    char data_c[data_len];  // Bufer de lectura por I2C para un array de char
    byte data_b[data_len];  // Bufer de lectura por I2C para un array de byte
    if (input == "help") {  // Si se lee help mostramos el menú de ayuda
      help();
    } else if (result1 == REGEXP_MATCHED) { // Orden de un disparo
      int posicionPrimerEspacio = input.indexOf(" ");
      String stringDesdePrimerEspacio = input.substring(posicionPrimerEspacio+1);
      int posicionSegundoEspacio = stringDesdePrimerEspacio.indexOf(" ");
      String nombreSensor = stringDesdePrimerEspacio.substring(0, posicionSegundoEspacio);   
      String stringDesdeSegundoEspacio = stringDesdePrimerEspacio.substring(posicionSegundoEspacio+1);
      int posicionTercerEspacio = stringDesdeSegundoEspacio.indexOf(" ");
      String nombreOpcion = stringDesdeSegundoEspacio.substring(0, posicionTercerEspacio);
      String tiempo = "";
      if (nombreOpcion.equals("on")){   // Si se selecciona la opción on guardamos el tiempo de retardo
        tiempo = stringDesdeSegundoEspacio.substring(posicionTercerEspacio+1);
      }
      byte v[4];
      v[0] = 1;                       // Primera posición de v es el codigo de orden
      if (nombreSensor.equals("srf04")) {   // Segunda posición es el identificador del sensor con el que se va a trabajar
        v[1] = 2;
      } else {
        v[1] = 1;
      }
      if (nombreOpcion.equals("off")) {    // Tercera posición es el tipo de opción que usa la orden
        v[2] = 3;
        v[3] = 0;
      } else if (nombreOpcion.equals("on")) {
        v[2] = 2;
        v[3] = (byte)tiempo.toInt();      // En caso de opción 'on' la cuarta posición es el tiempo en ms entre dos pulsos (al ser tipo byte el número máximo es 255) 
      } else {
        v[2] = 1;
        v[3] = 0;
      }
      oneshot(v, data_c);
    } else if (result2 == REGEXP_MATCHED) { // Orden para cambiar unidades
      byte v[3];
      v[0] = 2;
      int posicionPrimerEspacio = input.indexOf(" ");
      String stringDesdePrimerEspacio = input.substring(posicionPrimerEspacio+1);
      int posicionSegundoEspacio = stringDesdePrimerEspacio.indexOf(" ");
      String nombreSensor = stringDesdePrimerEspacio.substring(0, posicionSegundoEspacio);
      String stringDesdeSegundoEspacio = stringDesdePrimerEspacio.substring(posicionSegundoEspacio+1);
      int posicionTercerEspacio = stringDesdeSegundoEspacio.indexOf(" ");
      String unidadMedida = stringDesdeSegundoEspacio.substring(posicionSegundoEspacio);
      if (nombreSensor.equals("srf04")) {
        v[1] = 2;
      } else {
        v[1] = 1;
      }
      if (unidadMedida.equals("ms")) {
        v[2] = 3;
      } else if (unidadMedida.equals("inc")) {
        v[2] = 2;
      } else {
        v[2] = 1;
      }
      changeUnit(v, data_c);
    } else if (result3 == REGEXP_MATCHED) { // Orden para cambiar retardo entre disparos
      byte v[3];
      v[0] = 3;
      int posicionPrimerEspacio = input.indexOf(" ");
      String stringDesdePrimerEspacio = input.substring(posicionPrimerEspacio+1);
      int posicionSegundoEspacio = stringDesdePrimerEspacio.indexOf(" ");
      String nombreSensor = stringDesdePrimerEspacio.substring(0, posicionSegundoEspacio);
      String stringDesdeSegundoEspacio = stringDesdePrimerEspacio.substring(posicionSegundoEspacio+1);
      int posicionTercerEspacio = stringDesdeSegundoEspacio.indexOf(" ");
      String tiempo = stringDesdeSegundoEspacio.substring(posicionSegundoEspacio);
      if (nombreSensor.equals("srf04")) {
        v[1] = 2;
      } else {
        v[1] = 1;
      }
      v[2] = (byte)tiempo.toInt();
      delayed(v, data_c);
    } else if (result4 == REGEXP_MATCHED) { // Orden para obtener configuración del sensor
      byte v[2];
      v[0] = 4;
      int posicionPrimerEspacio = input.indexOf(" ");
      String stringDesdePrimerEspacio = input.substring(posicionPrimerEspacio+1);
      int posicionSegundoEspacio = stringDesdePrimerEspacio.indexOf(" ");
      String nombreSensor = stringDesdePrimerEspacio.substring(0, posicionSegundoEspacio);
      if (nombreSensor.equals("srf04")) {
        v[1] = 2;
      } else {
        v[1] = 1;
      }
      state(v, data_b);
    } else if (input == "us") { // Orden para mostrar lista de sensores
      byte v[1];
      v[0] = 5;
      us(v, data_c);
    } else {  // Si la orden introducida no es valida mostramos un mensaje por el monitor
      SerialUSB.print("La orden ");
      SerialUSB.print(input);
      SerialUSB.println(" no es una orden valida. Para obtener la lista de ordenes ecriba 'help'");
    }
  }


  /*uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) 
  { 
    if(Serial1.available()>0) {  
      uint8_t data = Serial1.read();
      SerialUSB.print("La medida obtenida es: ");
      SerialUSB.print(data);
      SerialUSB.println("cms");
      Serial1.write(ack);     // Constante definida con valor 6 para controlar que la comunicación con el sensor es correcta
      break;
    }
  }*/
  digitalWrite(LED_BUILTIN,led_state);
  led_state=(led_state+1)&0x01;
}


void help(){
  SerialUSB.println("- help: se muestra información acerca de los comandos aceptados y la operativa correcta del sistema.");
  SerialUSB.println("- us <srf02> {one-shot | on <period_ms> | off}: se comanda un único disparo del sensor de ultrasonidos (one-shot), o bien se establece que se dispare con un periodo específico de manera continuada (on <period_ms>), o que se cese de disparar el sensor de manera periódica si lo estuviera (off). En el comando debe identificarse qué sensor SRF02 quiere dispararse (<srf02>).");
  SerialUSB.println("- us <srf02> unit {inc | cm | ms}: este comando permite modificar la unidad de medida devuelta por un sensor SRF02 específico (<srf02>).");
  SerialUSB.println("- us <srf02> delay <ms>: este comando establece el tiempo de espera o retardo mínimo que debe haber entre dos disparos consecutivos del sensor (<srf02>).");
  SerialUSB.println("- us <srf02> status: este comando debe proporcionar información de configuración del sensor, en concreto, su dirección I2C, retardo mínimo entre disparos, su configuración de unidades de medida, y su estado de disparo periódico, en el caso de que éste esté activado o no.");
  SerialUSB.println("- us: este comando debe proporcionar la relación de sensores de ultrasonidos disponibles en el dispositivo sensor.");
}

// Orden de hacer oneshot, disparar continuado con 'tiempo' periodico o apagar el sensor
int oneshot(byte v [4], char data[data_len]) {
  Serial1.write(v, sizeof(v));
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {  
      int rlen = Serial1.readBytes(data, data_len);
      SerialUSB.print("Estado de la orden: ");
      for (int i = 0; i < rlen; i++) {
        SerialUSB.print(data[i]);
      }
      SerialUSB.println();
      break;
    }
  }
}

// Orden para cambiar la unidad de medida del sensor
int changeUnit(byte v [3], char data[data_len]) {
  Serial1.write(v, sizeof(v));
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) {
      if(Serial1.available()>0) {  
      int rlen = Serial1.readBytes(data, data_len);
      SerialUSB.print("Estado de la orden: ");
      for (int i = 0; i < rlen; i++) {
        SerialUSB.print(data[i]);
      }
      SerialUSB.println();
      break;
    }
  }
}

// Orden para modificar el retardo entre dos disparos del sensor
int delayed(byte v [3], char data[data_len]) {
  Serial1.write(v, sizeof(v));
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) {
    if(Serial1.available()>0) {  
      int rlen = Serial1.readBytes(data, data_len);
      SerialUSB.print("Estado de la orden: ");
      for (int i = 0; i < rlen; i++) {
        SerialUSB.print(data[i]);
      }
      SerialUSB.println();
      break;
    }
  }
}

// Orden para obtener la informacion de configuración del sensor
int state(byte v [2], byte data[data_len]) {
  Serial1.write(v, sizeof(v));
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {  
      int rlen = Serial1.readBytes(data, data_len);
      SerialUSB.print("El sensor ");
      if ((int)data[0] == 2) {      // Si la primera posición es un 2 trabajamos con el sensor srf04
        SerialUSB.print("srf04 esta midiendo en ");
        if ((int)data[1] == 3) {    // Si la segunda posición es un 3 significa que lee en inc
          SerialUSB.print("inc ");
        } else if ((int)data[1] == 2) { // Si la segunda posición es un 2 significa que lee en ms
          SerialUSB.print("ms ");
        } else {    // Por defecto lee en cm
          SerialUSB.print("cm ");
        }
      } else {    // Por defecto trabajamos con el sensor srf02
        SerialUSB.print("srf02 esta midiendo en ");
        if ((int)data[1] == 3) {
          SerialUSB.print("inc ");
        } else if ((int)data[1] == 2) {
          SerialUSB.print("ms ");
        } else {
          SerialUSB.print("cm ");
        }
      }
      SerialUSB.print("cada ");
      SerialUSB.print((int)data[2]);    // La última posicion son los ms que tiene el sensor de retardo entre disparos
      SerialUSB.println("ms");
      break;
    }
  }
}

// Orden para informar de todos los sensores disponibles
int us(byte v[1], char data[data_len]) {
  Serial1.write(v, sizeof(v));
  SerialUSB.print("Los sensores disponibles son: ");
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {  
      int rlen = Serial1.readBytes(data, data_len);
      for (int i = 0; i < rlen; i++) {
        SerialUSB.print(data[i]);     // Imprime la lista de sensores caracter a caracter
      }
      SerialUSB.println();
      break;
    }
  }
}
