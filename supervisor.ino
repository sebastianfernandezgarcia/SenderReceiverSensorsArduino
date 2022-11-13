/* ----------------------------------------------------------------------
 *  Ejemplo echo.ino 
 *    Este ejemplo muestra como utilizar el puerto serie uart (Serial1) 
 *    para comunicarse con otro dispositivo.
 *    
 *  Asignatura (GII-IC)
 * ---------------------------------------------------------------------- 
 */

 #define akc 6

constexpr const uint32_t serial_monitor_bauds=115200;
constexpr const uint32_t serial1_bauds=9600;

constexpr const uint32_t pseudo_period_ms=1000;

uint8_t led_state=LOW;

int option = 0;

void setup()
{
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

void loop()
{
  //Serial.println("******************** echo example *********************"); 

  if(Serial.available()>0) { // Si se encuentra algo que leer 
    String input = Serial.readStringUntil('\n'); //Leemos hasta que se encuentra un sato de línea

    // En todas las ordenes pasamos a los metodos parametros de tipo char []
    // Ya que el metodo Serial1.write() no permite mandar tipo String
    
    if (input == "help") {  // Si se lee help mostramos el menú de ayuda
      help();
    } else if (input == "") { // Orden de un disparo
      String sensor = "srf02";
      char sens [5];
      String option = "on";
      char opt [8];
      String miliseconds = "500";
      char tmp [5];
      sensor.toCharArray(sens, 5);
      option.toCharArray(opt, 8);
      miliseconds.toCharArray(tmp, 5);
      oneshot(sens, opt, tmp);
    } else if (input == "") { // Orden para cambiar unidades
      String sensor = "srf02";
      char sens [5];
      String unidad = "cm";
      char unit [3];
      sensor.toCharArray(sens, 5);
      unidad.toCharArray(unit, 3);
      changeUnit(sens, unit);
    } else if (input == "") { // Orden para cambiar retardo entre disparos
      String sensor = "srf02";
      char sens [5];
      String miliseconds = "500";
      char tmp [5];
      sensor.toCharArray(sens, 5);
      miliseconds.toCharArray(tmp, 5);
      delayed(sens, tmp);
    } else if (input == "") { // Orden para obtener configuración del sensor
      String sensor = "srf02";
      char sens [5];
      sensor.toCharArray(sens, 5);
      state(sens);
    } else if (input == "us") { // Orden para mostrar lista de sensores
      us();
    } else {  // Si la orden introducida no es valida mostramos un mensaje por el monitor
      SerialUSB.print("La orden ");
      SerialUSB.print(input);
      SerialUSB.println(" no es una orden valida. Para obtener la lista de ordenes ecriba 'help'");
    }
  }


  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) 
  { 
    if(Serial1.available()>0) {  
      uint8_t data = Serial1.read();
      SerialUSB.print("La medida obtenida es: ");
      SerialUSB.print(data);
      SerialUSB.println("cms");
      Serial1.write(akc);     // Constante definida con valor 6 para controlar que la comunicación con el sensor es correcta
      break;
    }
  }
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
int oneshot(char sensor [5], char accion [8], char tiempo [5]) {
  option = 1;
  Serial1.write(option);
  Serial1.write(sensor);
  Serial1.write(accion);
  if (tiempo != "" || tiempo != 0) {
    Serial1.write(tiempo);
  }
}

// Orden para cambiar la unidad de medida del sensor
int changeUnit(char sensor [5], char unidad [3]) {
  option = 2;
  Serial1.write(option);
  Serial1.write(sensor);
  Serial1.write(unidad);
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) {
    uint8_t data = Serial1.read();
    return data;
  }
}

// Orden para modificar el retardo entre dos disparos del sensor
int delayed(char sensor [5], char miliseconds [5]) {
  option = 3;
  Serial1.write(option);
  Serial1.write(sensor);
  Serial1.write(miliseconds);
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) {
    uint8_t data = Serial1.read();
    return data;
  }
}

// Orden para obtener la informacion de configuración del sensor
int state(char sensor [5]) {
  option = 4;
  Serial1.write(option);
  Serial1.write(sensor);
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {  
      uint8_t data = Serial1.read();
      SerialUSB.print("La la configuracion del sensor ");
      SerialUSB.print(sensor);
      SerialUSB.print(" es: ");
      SerialUSB.println(data);
      break;
    }
  }
  last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) {
    uint8_t data = Serial1.read();
    return data;
  }
}

// Orden para informar de todos los sensores disponibles
int us() {
  SerialUSB.println("Los sensores disponibles son: ");
  uint32_t last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) { 
    if(Serial1.available()>0) {  
      uint8_t data = Serial1.read();
      SerialUSB.println(data);
      break;
    }
  }
  last_ms=millis();
  while(millis()-last_ms<pseudo_period_ms) {
    uint8_t data = Serial1.read();
    return data;
  }
}
