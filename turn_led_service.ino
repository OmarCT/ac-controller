
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <WiFi.h>
#include <WebServer.h>

#include <DHT.h>

#define DHTPIN 18
#define DHTTYPE DHT11


const char* SSID = "Tu Cola";
const char* PSSWD = "papasfritasviejas";

bool ON = HIGH;
bool OFF = LOW;

unsigned short IR_LED = 32;
unsigned short GREEN_LED = 12;

unsigned short IR_RECEPTOR = 33; 

bool ESTADO_IR_LED = OFF;
bool ESTADO_GREEN_LED = OFF;

unsigned short LED_LIST[] = {IR_LED, GREEN_LED};
bool LED_STATUS_LIST[] = {ESTADO_IR_LED, ESTADO_GREEN_LED};

IRrecv irrecv(IR_RECEPTOR);
DHT dht(DHTPIN, DHTTYPE);
WebServer server(8001);

decode_results codigo;

String do_post(unsigned char led_id, bool led_status) {

  char message[255];
  unsigned short previous_status = LED_STATUS_LIST[led_id];
  unsigned short current_status = led_status;

  LED_STATUS_LIST[led_id] = current_status;

  digitalWrite(LED_LIST[led_id], led_status);


  if (snprintf(message, sizeof(message), "{\"leda_id\": %d,\"led_previous_status\": %d, \"led_current_statu s\": %d}", led_id, previous_status, current_status)){
    return message;
  } else {
    return ("Error con el servidor");
  }
}

String do_get(unsigned char led_id) {

  char message[255];
  unsigned short previous_status = LED_STATUS_LIST[led_id];
  unsigned short current_status = LED_STATUS_LIST[led_id];

  if (snprintf(message, sizeof(message), "{\"leda_id\": %d,\"led_previous_status\": %d, \"led_current_statu s\": %d}", led_id, previous_status, current_status)){
    return message;
  } else {
    return ("Error con el servidor");
  }

}

void handle_OnConnect() {

  String message = "";
  char temp_message[255];
  for(int i = 0; i < 2; i++){
    message += do_get(i);
    message += ",";
  }

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    message += " {\"error\": 1,\"message\": \"Fallo lectura de temperatura\"}";
  }
  else{
    snprintf(temp_message, sizeof(temp_message), " {\"temperatura\": %f,\"humedad\": %f}", temperature, humidity);
    message += temp_message;
  }

  Serial.println("GPIO4 Estado: OFF | GPIO5 Estado: OFF");
  server.send(200, "application/json", message);
}

void handle_led1on() {
  Serial.println("GPIO4 Estado: ON");
  server.send(200, "application/json", do_post(0, ON));
}

void handle_led1off() {
  Serial.println("GPIO4 Estado: OFF");
  server.send(200, "application/json", do_post(0, OFF));
}

void handle_led2on() {
  Serial.println("GPIO5 Estado: ON");
  server.send(200, "application/json", do_post(1, ON));
}

void handle_led2off() {
  Serial.println("GPIO5 Estado: OFF");
  server.send(200, "application/json", do_post(1, OFF));
}

void handle_NotFound() {
  server.send(404, "text/plain", "La pagina no existe");
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  //pinMode(DHTPIN, INPUT);

  Serial.println("Inicializacion sensor de temperatura/humedad");
  dht.begin();
  Serial.println("Sensor inicializado");

  irrecv.enableIRIn();

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSSWD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(SSID);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  server.on("/", handle_OnConnect);
  server.on("/on/0", handle_led1on);
  server.on("/on/1", handle_led2on);
  server.on("/off/0", handle_led1off);
  server.on("/off/1", handle_led2off);
  server.onNotFound(handle_NotFound); 

  server.begin();
  Serial.println("Servidor HTTP iniciado");

}
/*
 * Para gestionar las la peticiones HTTP es necesario llamar al metodo "handleClient"
 * de la libreria WebServer que se encarga de recibir las peticiones y lanzar las fuciones
 * de callback asociadas tipo "handle_led1on()" "handle_led2on()" etc
 * Tambien ejecutan el cambio de estado de los pines y por lo tanto hacen que los 
 * LEDs se enciendan o apaguen
 */
void loop() {
  //server.handleClient();
  if (irrecv.decode(&codigo)){
    Serial.print("Codigo recibido: ");
    serialPrintUnit64(codigo.value, HEX);
    irrecv.resume();
  }
    delay(100);
}