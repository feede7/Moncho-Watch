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
SoftwareSerial SIM800L(2, 3);   // Definimos los pines RX y TX del Arduino conectados al Bluetooth
int loop_n = 0;

#include <AM2302-Sensor.h>
constexpr unsigned int SENSOR_PIN {4};

AM2302::AM2302_Sensor am2302{SENSOR_PIN};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // D13

  SIM800L.begin(115200);       // Inicializamos el puerto serie BT (Para Modo AT 2)
  Serial.begin(9600);   // Inicializamos  el puerto serie
  Serial.print("Sketch:   ");   Serial.println(__FILE__);

  // When power is supplied to sensor, don't send any instruction to the sensor within one second to pass unstable status
  delay(1000);

  // set pin and check for sensor
  if (am2302.begin()) {
    // this delay is needed to receive valid data,
    // when the loop directly read again
    delay(3000);
  }
  else {
    while (true) {
    Serial.println("Error: sensor check. => Please check sensor connection!");
    delay(10000);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.print("Loop " + String(loop_n) + "\n");
  loop_n ++;

  auto status = am2302.read();
  Serial.print("\n\nstatus of sensor read(): ");
  Serial.println(status);

  Serial.print("Temperature: ");
  Serial.println(am2302.get_Temperature());

  Serial.print("Humidity:    ");
  Serial.println(am2302.get_Humidity());
  delay(5000);

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // turn the LED on (HIGH is the voltage level)
  // delay(1000);                      // wait for a second
}
