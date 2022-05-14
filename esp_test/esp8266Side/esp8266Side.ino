
#include <ArduinoOTA.h>
#include <SoftwareSerial.h> // defining the software serial library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//SoftwareSerial stateSerial(D6,D5); // RX=D6,TX=D5，用于传送relay/led state的软串口
SoftwareSerial ctlSerial(D8,D7); // RX=D8,TX=D7,用于控制灯泡开关的软串口

const char* ssid = "finEsp8266";
const char* password = "123456li";

// MQTT Broker
const char *mqtt_broker = "124.222.94.149";
const int mqtt_port = 1883;
const char *topic = "lamp";  //发布主题lamp
const char *topic2 = "ctl";  //订阅主题ctl
const char *mqtt_username = "admin";
const char *mqtt_password = "123312";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
String sensorState;
String client_id;  // mqtt client id
long lastMsg = 0;  // 记录上一次发送信息的时长

void setup() {
  Serial.begin(9600);
//  stateSerial.begin(9600);
  ctlSerial.begin(9600);
  
  //connecting to a WiFi network---------------------------
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // this will display the Ip address of the nodemcu which should be entered into your browser
  //WiFi----------------------------------------------------

  //set mqtt client id
  client_id = "esp8266-client-";
  client_id += String(WiFi.macAddress());
  //set mqtt broker
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setKeepAlive(10); // 设置心跳间隔时间
  mqttClient.setCallback(callback);  // 设定回调方法，当ESP8266收到订阅消息时会调用此方法
  sensorState = "null";

  // OTA设置并启动
  ArduinoOTA.setHostname("ESP8266OTA");
  ArduinoOTA.setPassword("123");
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]); // 打印主题内容
    }
    if ((char)payload[0] == '0') {  // 关闭远程控制模式
        ctlSerial.print('0');
    }else if ((char)payload[0] == '1') {  // 打开远程控制模式
        ctlSerial.print('1'); 
    }else if ((char)payload[0] == '2') {  // 关灯
        ctlSerial.print('2'); 
    }else if ((char)payload[0] == '3') {  // 开灯
        ctlSerial.print('3'); 
    }
    Serial.println();
    Serial.println("-----------------------");
}

void connect() {
  while (!mqttClient.connected()) {
    Serial.printf("The client %s (re)connects to the public mqtt broker\n", client_id.c_str());
    // Attempt to connect
    if (mqttClient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Emqx mqtt broker connected");
      mqttClient.subscribe(topic2);
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
   }
  }
}

void loop(){
  ArduinoOTA.handle();

  Serial.println("111");
  while(!ctlSerial.available());
  if(ctlSerial.available()){
    Serial.println("222");
    sensorState = ctlSerial.readStringUntil('\r');
//    ledState = ledState.substring(ledState.length()-1);  // 仅需要最后一个字符
//    Serial.println("ledState: "+ledState);
    Serial.println(sensorState);
  }
  delay(100);  //2000
  
  if (!mqttClient.connected()) {
    connect();
  }
  
  mqttClient.loop();  //执行到这里才能调用回调函数
  
  long now = millis();
  if (now - lastMsg > 100) {  //1000+2000=5.5s   500+1500=4.5s  
    lastMsg = now;
//    String msg = "sensor state: "+ledState;
    mqttClient.publish(topic, sensorState.c_str());
  }
  ESP.wdtFeed();  // 喂狗
}

