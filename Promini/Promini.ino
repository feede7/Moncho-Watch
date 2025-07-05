/*  Comandos AT SIM800L
 * AT – Verifica que la comunicacion es correcta. contesta con un OK.
 * ATI - Muestra la versión del modulo.
 * AT+CSQ – Calidad de la señal, 0-31.
 * AT+CBC - Regresa el status de la batería, % y voltaje.
 * AT+CMGF=1 - Activa el modo sms.
 * Rafael Lozano Rolon
 * soporte@taloselectronics.com
*/

#include <SoftwareSerial.h>   // Incluimos la librería  SoftwareSerial
#define Gsm_tx 8  // Al TXD del SIM800L
#define Gsm_rx 9  // Al RXD del SIM800L

SoftwareSerial MOD_SIM800L(Gsm_tx, Gsm_rx);
String Numero_cliente = "91165369244";
// SoftwareSerial SIM800L(2, 3);   // Definimos los pines RX y TX del Arduino conectados al Bluetooth
int loop_n = 0;
int DELAY_LOOP = 10;

// AM2302
#include <AM2302-Sensor.h>
constexpr unsigned int SENSOR_PIN {4};
AM2302::AM2302_Sensor am2302{SENSOR_PIN};
int am2302_tick = 0;

// HC-SR501
int ledPin = 12;                // choose the pin for the LED
int inputPin = 7;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

// TEMT6000
int pinSensor = 4;
int light_valor = 0;

// Moncho
// String activated_yes = "yes";
String activated_yes = "no";
String activated = activated_yes;

// Strings
String temperature_status = "";
String ACTIVATE = "morcilla";
String DEACTIVATE = "chinchu";
String TEMPERATURE = "tapolar";
String STATUS = "moncho";
String COMMANDS = "##";

void select_answer(String number, String message) {
  String answer = "";

  if(message.indexOf(ACTIVATE) >= 0){
    activated = "yes";
    answer = "Fuerte y claro";
  }
  if(message.indexOf(DEACTIVATE) >= 0){
    activated = "no";
    answer = "Te mando un besito!";
  }
  if(message.indexOf(TEMPERATURE) >= 0){
    answer = temperature_status;
    answer = "Acordate que no tengo sensores!";
  }
  if(message.indexOf(STATUS) >= 0){
    answer = "Status:\r\n";
    answer += "Activated: " + activated + "\r\n";
    // answer += temperature_status + "\r\n";
  }
  if(message.indexOf(COMMANDS) >= 0){
    answer = "Activate: " + ACTIVATE + "\r\n";
    answer += "Deactivate: " + DEACTIVATE + "\r\n";
    // answer += "Temperature: " + TEMPERATURE + "\r\n";
    answer += "Status: " + STATUS + "\r\n";
  }
  if(answer != "")
    Enviar_msj(Numero_cliente, answer);
  
  String a_time = "";
  a_time += "es CMT!\r\n";
  a_time += number + "\r\n";
  a_time += message;
  Enviar_msj(Numero_cliente, a_time);
}

void Enviar_msj(String numero, String msj)
{
  //Se establece el formato de SMS en ASCII
  String config_numero = "AT+CMGS=\"+54" + numero + "\"\r\n";
  Serial.println(config_numero);

  //configurar modulo como modo SMS
  MOD_SIM800L.write("AT+CMGF=1\r\n");
  delay(1000);
  
  //Enviar comando para un nuevos SMS al numero establecido
  MOD_SIM800L.print(config_numero);
  delay(1000);

  //Enviar contenido del SMS
  MOD_SIM800L.print(msj);
  delay(1000);

  //Enviar Ctrl+Z
  MOD_SIM800L.write((char)26);
  delay(1000);
  Serial.println("Mensaje enviado");
}

void check_sr501(){
  val = digitalRead(inputPin);  // read input value
  
  if (val == HIGH)	// check if the input is HIGH
  {            
    digitalWrite(ledPin, HIGH);  // turn LED ON
	
    if (pirState == LOW) 
	{
      Serial.println("Motion detected!");	// print on output change
      if (activated == "yes")
        Enviar_msj(Numero_cliente, "raaari");
      pirState = HIGH;
    }
  } 
  else 
  {
    digitalWrite(ledPin, LOW); // turn LED OFF
	
    if (pirState == HIGH)
	{
      Serial.println("Motion ended!");	// print on output change
      pirState = LOW;
    }
  }
}

void updateSerial()
{
  delay(500);
  // while (Serial.available()){
  //   MOD_SIM800L.write(Serial.read());//Forward what Serial received to Software Serial Port
  // }
  String whole_msg = "";
  while(MOD_SIM800L.available()){
    char msj = MOD_SIM800L.read();
    Serial.write(msj);//Forward what Software Serial received to Serial Port
    whole_msg += msj;
  }
  if(whole_msg != ""){
    if(whole_msg.indexOf("ERR") > 0){
      Serial.print("ERROR");//Forward what Software Serial received to Serial Port
      Serial.print(whole_msg);//Forward what Software Serial received to Serial Port
    }
    analyse_msj(whole_msg);
  }
}

void analyse_msj(String msj){
  int length = msj.length();
  Serial.print("LEN: "); Serial.println(length);
  if(length > 40){
    int field = 0;
    /*from: https://descubrearduino.com/sim800l-gsm/
    Su respuesta comienza con +CMT: Todos los campos de la respuesta están separados 
    por comas y el primer campo es el número de teléfono. El segundo campo es el nombre 
    de la persona que envía el SMS. El tercer campo es una marca de tiempo mientras que 
    el cuarto campo es el mensaje real.*/
    String header = "";
    String number = "";
    String name = "";
    String date = "";
    String time = "";
    String message = "";
    String separators = ", \r\n";
    char last = 'X';
    bool reading_msg = false;

    for(int i=0; i < length; i++){
      if(separators.indexOf(msj[i]) >= 0 & !reading_msg & header != ""){
        if(!(separators.indexOf(last) >= 0))
          field++;
      }
      else{
        if(field == 0){
          header += msj[i];
        } else
        if(field == 1){
          number += msj[i];
        } else
        if(field == 2){
          name += msj[i];
        } else
        if(field == 3){
          date += msj[i];
        } else
        if(field == 4){
          time += msj[i];
        } else
        if(field == 5){
          reading_msg = true;
          message += msj[i];
        } 
      }
      last = msj[i];
    }
    Serial.print("header: "); Serial.println(header);
    Serial.print("number: "); Serial.println(number);
    Serial.print("name: "); Serial.println(name);
    Serial.print("date: "); Serial.println(date);
    Serial.print("time: "); Serial.println(time);
    Serial.print("message: "); Serial.println(message);
    if(header.indexOf("CMT" >= 0)){
      if(number != "OK"){
        Serial.println("es CMT!");
        select_answer(number, message);
      }
    }
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // D13
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)

  for(int i=0; i < 20; i++){
    delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // turn the LED on (HIGH is the voltage level)
  }

  // SIM800L.begin(115200);       // Inicializamos el puerto serie BT (Para Modo AT 2)
  Serial.begin(9600);   // Inicializamos  el puerto serie
  Serial.print("Sketch:   ");   Serial.println(__FILE__);

  // When power is supplied to sensor, don't send any instruction to the sensor within one second to pass unstable status
  delay(1000);

  // AutoBauding -> 1200, 2400, 4800, 9600, 19200, 38400, 57600
  // MOD_SIM800L.begin(115200);
  MOD_SIM800L.begin(57600);
  // MOD_SIM800L.begin(38400);
  // MOD_SIM800L.begin(9600);
  Serial.println("Initializing...");
  delay(1000);

  MOD_SIM800L.println("AT"); //Once the handshake test is successful, it will back to OK
  Serial.println("AT");
  updateSerial();
  MOD_SIM800L.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  Serial.println("AT+CSQ");
  updateSerial();
  MOD_SIM800L.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  Serial.println("AT+CCID");
  updateSerial();
  MOD_SIM800L.println("AT+CREG?"); //Check whether it has registered in the network
  Serial.println("AT+CREG");
  updateSerial();
  MOD_SIM800L.println("AT+CMGF=1"); // Configuring TEXT mode
  Serial.println("AT+CMGF=1");
  updateSerial();
  MOD_SIM800L.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  Serial.println("AT+CNMI");
  updateSerial();
  Enviar_msj(Numero_cliente, "tamo ready");
  delay(1000);
}

// void get_am2302(){
//   if (am2302_tick == int(1000 / DELAY_LOOP)){
//     // This enters each 5s
//     am2302_tick = 0;
//     // auto status = am2302.read();

//     // temperature_status = "Temperature: ";
//     // temperature_status += am2302.get_Temperature();
//     // temperature_status += "\r\n";
//     // temperature_status += "Humidity: ";
//     // temperature_status += am2302.get_Humidity();
//     // temperature_status += "\r\n";

//     // // Light sensor
//     // light_valor = analogRead(pinSensor); 
//     // temperature_status += "Light: " + (String) light_valor;
//   }
//   else {
//     am2302_tick++;
//   }
// }

void loop() {
  // put your main code here, to run repeatedly:
  
  if (loop_n % int(1000 / DELAY_LOOP) == 0){
    // This enters each 1s
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // turn the LED on (HIGH is the voltage level)
  }
  loop_n++;

  // get_am2302();
  check_sr501();

  updateSerial();

  delay(DELAY_LOOP);
}
