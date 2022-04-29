
#include <SoftwareSerial.h>
#include <dht.h>
//for ultrasonic sensor
#include <NewPing.h> 
#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


//SoftwareSerial stateSerial(4, 5); //RX=4,TX=5，用于传送relay/led state的软串口
SoftwareSerial ctlSerial(2, 3);  //RX=2,TX=3,用于控制灯泡开关的软串口
//继电器
const int relay = 8;  
String lastRelayState, relayState;

// pir sensor
const int pirSensor = 13;
// 温湿度传感器
dht DHT;
const int DHTSensor = 7;
// 光敏传感器
const int photocellPin = A0;

void setup() {
  Serial.begin(9600);
//  stateSerial.begin(9600);
  ctlSerial.begin(9600);  

  pinMode(relay, OUTPUT);
  pinMode(pirSensor, INPUT);
  digitalWrite(relay, HIGH);  // 继电器的 HIGH 是断开（磁片接到NC端）
  relayState = "0";
}

void motorRun()
{
  ctlSerial.listen();
  while(true){
    if(ctlSerial.available()>0){
      String command = ctlSerial.readStringUntil('\r');
      Serial.println(command);
      if(command == "2"){  // 关灯
        digitalWrite(relay, HIGH);
        relayState = "0";
      }else if(command == "3"){  // 开灯
        digitalWrite(relay, LOW);
        relayState = "1";
      }else if(command == "0"){  // 关闭远程控制模式
        break;
      }
    }
   Serial.println(relayState);
   ctlSerial.print(relayState);  //每隔2秒发送一次
   //...
   delay(1000);
  }
}

void loop() {
  
  DHT.read11(DHTSensor);
  String temperature = String(DHT.temperature);
  String humidity = String(DHT.humidity);
  int photocellValue = analogRead(photocellPin);
  
  ctlSerial.listen();   //开启软串口监听状态
  if (ctlSerial.available() > 0 ) 
  {
     Serial.println("in ctlSerial.available");
     String command = ctlSerial.readStringUntil('\r'); // read that data and store it in command variable
     Serial.println(command);
     if(command == "1"){  // 远程管理模式已打开
        motorRun();
     }
  }else{
   delay(100);   // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
   unsigned int roundTripTime = sonar.ping(); // Send ping, get ping time in microseconds (uS).
   int distance = roundTripTime / US_ROUNDTRIP_CM;  // Convert ping time to distance in cm and print result (0 = outside set distance range)
   Serial.println("Ping: "+String(distance)+"cm");
   int pirValue = digitalRead(pirSensor);
   if((distance > 0 && distance <= 80) || pirValue == 1) 
   {
    digitalWrite(relay, LOW); //turn on the led 
    relayState = "1";
   }else
   {
    digitalWrite(relay, HIGH); //turn off the led
    relayState = "0";
   } 
  }
   String sensorState = "{\"ledState\": " + relayState + ",\"temperature\": " + temperature + ",\"humidity\": " + humidity + ",\"light\": " + photocellValue + "}";
   Serial.println(sensorState);
   ctlSerial.print(sensorState);  //每隔3秒发送一次
   //...
   delay(3000);
}

