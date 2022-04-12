
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h> // defining the software serial library

//SoftwareSerial stateSerial(D6,D5); // RX=D6,TX=D5，用于传送relay/led state的软串口
SoftwareSerial ctlSerial(D8,D7); //RX=D8,TX=D7,用于控制灯泡开关的软串口

// Add wifi access point credentials
const char* ssid = "finEsp8266";
const char* password = "123456li";

WiFiServer server(80);// Set port to 80

String header, ledState; // This stores the HTTP request
int govern;  //是否开启远程管理模式，govern=1: 是，govern=0: 否

void setup() {
  Serial.begin(9600);
  //stateSerial.begin(9600);
  ctlSerial.begin(9600);
  //connect to access point
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
  Serial.println(WiFi.localIP()); // this will display the Ip address of the Pi which should be entered into your browser
  server.begin();
  ledState = "0";
  govern = 0;
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients，client对象也用来发送服务器（这里的esp8266）的响应信息
  if (client) {                             // If a new client connects,
    Serial.println("+++");
    getLedState();                          //update led state
    //if(ledState == "") ledState="0";
    //if(ledState.indexOf('\n') >= 0) ledState = ledState.substring(1); 
    Serial.println("Led state: "+ledState+" ?");
    Serial.println("+++");
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there are bytes reading from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /govern/on/LED/on") >= 0) {
              Serial.println("led 开");
              ledState = "1";
              ctlSerial.print("open");
            } else if (header.indexOf("GET /govern/on/LED/off") >= 0) {
              Serial.println("led 关");
              ledState = "0";
              ctlSerial.print("close");
            } else if(header.indexOf("GET /govern/on") >= 0){
              govern = 1;
              Serial.println("governOn");
              ctlSerial.print("governOn");
            } else if(header.indexOf("GET /govern/off") >= 0){
              govern = 0;
              Serial.println("governOff");
              ctlSerial.print("governOff");
            }
       
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" charset=\"utf-8\">");
          
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>智能灯控系统</h1>");
            String governStr = govern == 1 ? "开" : "关";
            client.println("<p>远程管理模式：" + governStr + "</p>");     
            if (govern == 0) {
              client.println("<p><a href=\"/govern/on\"><button class=\"button\">开</button></a></p>");
            } else {
              client.println("<p><a href=\"/govern/off\"><button class=\"button button2\">关</button></a></p>");
            } 

            if(govern == 1){   //如果打开了远程管理模式
              String ledStateStr = ledState == "1" ? "开" : "关";
              client.println("<p>LED - 当前状态：" + ledStateStr + "</p>");     
              if (ledState == "0") {
                client.println("<p><a href=\"/govern/on/LED/on\"><button class=\"button\">开</button></a></p>");
              } else {
                client.println("<p><a href=\"/govern/on/LED/off\"><button class=\"button button2\">关</button></a></p>");
              }  
             }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  //  Serial.println("Client disconnected.");
    //Serial.println("");
  }
}

void getLedState(){
    ctlSerial.listen();
    if(ctlSerial.available() > 0){
      ledState = ctlSerial.readStringUntil('\r');
      Serial.println("available!");
    }
}

