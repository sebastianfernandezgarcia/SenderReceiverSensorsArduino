/* ----------------------------------------------------------------------
 *  Ejemplo echo.ino 
 *    Este ejemplo muestra como utilizar el puerto serie uart (Serial1) 
 *    para comunicarse con otro dispositivo.
 *    
 *  Asignatura (GII-IC)
 * ---------------------------------------------------------------------- 
 */

 #define akc 1

constexpr const uint32_t serial_monitor_bauds=115200;
constexpr const uint32_t serial1_bauds=9600;

constexpr const uint32_t pseudo_period_ms=1000;

uint8_t led_state=LOW;

char helpString[5] = "help";

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
    if (input == "help") {  // Si se lee help mostramos el menú de ayuda
      help();
    } else {  // Si no mostramos que se ha leido
      SerialUSB.print("Se ha leido: ");
      SerialUSB.println(input);
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
      Serial1.write(akc);
      break;
    }
  }

  
  digitalWrite(LED_BUILTIN,led_state); led_state=(led_state+1)&0x01;
}


void help(){
  SerialUSB.println("- help: se muestra información acerca de los comandos aceptados y la operativa correcta del sistema.");
  SerialUSB.println("- us <srf02> {one-shot | on <period_ms> | off}: se comanda un único disparo del sensor de ultrasonidos (one-shot), o bien se establece que se dispare con un periodo específico de manera continuada (on <period_ms>), o que se cese de disparar el sensor de manera periódica si lo estuviera (off). En el comando debe identificarse qué sensor SRF02 quiere dispararse (<srf02>).");
  SerialUSB.println("- us <srf02> unit {inc | cm | ms}: este comando permite modificar la unidad de medida devuelta por un sensor SRF02 específico (<srf02>).");
  SerialUSB.println("- us <srf02> delay <ms>: este comando establece el tiempo de espera o retardo mínimo que debe haber entre dos disparos consecutivos del sensor (<srf02>).");
  SerialUSB.println("- us <srf02> status: este comando debe proporcionar información de configuración del sensor, en concreto, su dirección I2C, retardo mínimo entre disparos, su configuración de unidades de medida, y su estado de disparo periódico, en el caso de que éste esté activado o no.");
  SerialUSB.println("- us: este comando debe proporcionar la relación de sensores de ultrasonidos disponibles en el dispositivo sensor.");
}
