
#include <ESP8266WiFi.h>

// Add wifi access point credentials
const char* ssid = "finEsp8266";
const char* password = "123456li";

WiFiServer server(80);// Set port to 80


String header; // This stores the HTTP request

// Declare the pins to which the LEDs are connected 
int inD1 = D1;
int inD2 = D2; 

String D1state = "关";// state of blue LED
String D2state = "关";// state of lamp


void setup() {
  Serial.begin(9600);
 // Set the pinmode of the pins to which the LEDs are connected and turn them low to prevent fluctuations
  pinMode(inD1, OUTPUT);
  pinMode(inD2, OUTPUT);
  digitalWrite(inD1, LOW);
  digitalWrite(inD2, LOW);
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
  Serial.println(WiFi.localIP());// this will display the Ip address of the Pi which should be entered into your browser
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients，client对象也用来发送服务器（这里的esp8266）的响应信息
  
  //ss
  char a[20] = ""; //定义字符数组，接受来自上位机的数据
  int i = 0;
  char s[] = "q"; //每隔3s发送一次字符数组s
  //ss
  
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there are bytes reading from the client,
//        Serial.println("client.available "+ client.available());
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
            if (header.indexOf("GET /D1/on") >= 0) {
              Serial.println("D1 开");
              D1state = "开";
              digitalWrite(inD1, HIGH);
            } else if (header.indexOf("GET /D1/off") >= 0) {
              Serial.println("D1 关");
              D1state = "关";
              digitalWrite(inD1, LOW);
            } else if (header.indexOf("GET /D2/on") >= 0) {
              Serial.println("D2 开");
              D2state = "开";
              digitalWrite(inD2, HIGH);
            } else if (header.indexOf("GET /D2/off") >= 0) {
              Serial.println("D2 关");
              D2state = "关";
              digitalWrite(inD2, LOW);
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
            client.println("<body><h1>nodemcu 网页测试 BY：Aeolus</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>D1 - 信息 " + D1state + "</p>");
            // If the green LED is off, it displays the ON button       
            if (D1state == "关") {
              client.println("<p><a href=\"/D1/on\"><button class=\"button\">开</button></a></p>");
            } else {
              client.println("<p><a href=\"/D1/off\"><button class=\"button button2\">关</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>D2 - 信息 " + D2state + "</p>");
            // If the red LED is off, it displays the ON button       
            if (D2state == "关") {
              client.println("<p><a href=\"/D2/on\"><button class=\"button\">开</button></a></p>");
            } else {
              client.println("<p><a href=\"/D2/off\"><button class=\"button button2\">关</button></a></p>");
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

