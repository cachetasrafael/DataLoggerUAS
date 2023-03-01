#include "main.h"

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org", TIME_OFFSET);

AsyncWebServer server(80);
WebSocketsServer webSocket(81);

//RTC object
RTC_DS3231 RTC;

//Buffer to store the time and date
char time_buffer[20];

//Array for PWM signal output string
char pwmNAME[2][50]={"PWM SIGNAL (ON) ", "PWM SIGNAL (OFF) "};

String timeString, combinedString;

int flagNTPTime, ledState = 0;
volatile int flag_writeSD = 0;

//time variables for SD card LED when an append happens
unsigned long previousMillis = 0;
unsigned long interval = 1000;



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // this function is called when a websocket message is received
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Message received: %s\n", num, payload);
      break;
  }
}

void setup() {
  //Start UART
  Serial.begin(9600);

  //Setup PWM pin as an input pullup resistor (less susceptible to noise)
  pinMode(PWM_pin, INPUT_PULLUP);
  
  //Initialize digital pin PWM_LED, WIFI_LED, SD_LED as an output
  pinMode(PWM_LED, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(SD_LED, OUTPUT);

  //Setup SD Module
  SD.begin(SD_CS); 
  if(!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return; // init failed
  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "TESTING CAMPAIGN");
    digitalWrite(SD_LED, HIGH);
    delay(3000);
    digitalWrite(SD_LED, LOW);
    delay(1000);
  }
  else {
    Serial.println("File already exists");
    digitalWrite(SD_LED, HIGH);
    delay(3000);
    digitalWrite(SD_LED, LOW);
    delay(1000);  
  }
  file.close();

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(F("Connecting to WiFi..."));
  }
  Serial.println("WiFi connected.");
  // Print local IP address and start web server
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(WIFI_LED, HIGH);   // turn the LED on (HIGH is the voltage level)   

  // Initialize NTP client
  timeClient.begin();

  // Create HTML page with the respective sockets being sent  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200,"text/html", 
    "<html>\
      <head>\
        <title>Data Logger Air Unit Terminal</title>\
      </head>\
      <body>\
        <h1>Data Logger Air Unit</h1>\
        <pre id='output'></pre>\
        <script>\
          const socket = new WebSocket('ws://' + window.location.hostname + ':81');\
          socket.onmessage = function (event) {\
            console.log(event.data);\
            document.getElementById('output').innerHTML += event.data + '\\n';\
          };\
          socket.onclose = function (event) {\
            console.error('WebSocket closed', event);\
          };\
        </script>\
      </body>\
    </html>"
   );
   if(outputSent){
    request->send(200, "text/plain", combinedString);
   }
  });

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.begin();
  Serial.println("HTTP server started");



}

/*********************************FUNCTION TO GET THE CURRENT TIME FROM THE NTP SERVER*****************************************/
void getTIME(){
  timeClient.update();
    
  unsigned long currentTime = timeClient.getEpochTime(); //retrieves the current time from the NTP server in epoch time (number of seconds elapsed since January 1, 1970)

  //converts the number of seconds to a date structure
  time_t now = (time_t)currentTime;
  struct tm *timeinfo = localtime(&now); 
  
  //converts the data structure to char for printing on the SD card
  strftime (time_buffer,20,"%Y-%m-%d %H:%M:%S",timeinfo);

  Serial.print(time_buffer);
  Serial.print(" ");
  appendFile(SD, "/data.txt", time_buffer);
  appendFile(SD, "/data.txt", " ");

  //convert char to string for sending the data to the webserver
  timeString = String(time_buffer);

  //if this flag is 1 then this means the Wi-Fi is still up and will be continuously updating the RTC
  flagNTPTime = 1;
}


void lostWiFi(){
  DateTime nowRTC;
  char bufferRTC[20];

  if(flagNTPTime){
    timeClient.update();
    RTC.adjust(DateTime(timeClient.getEpochTime()));
    nowRTC=RTC.now();
    
    flagNTPTime = 0;
  }

  //if the flag is 0 then from now on the RTC time will be printed with the last saved NTP server time
  else{
    sprintf(bufferRTC, "%04d/%02d/%02d %02d:%02d:02d", nowRTC.year(), nowRTC.month(), nowRTC.day(), nowRTC.hour(), nowRTC. minute(), nowRTC.second());
    Serial.print(bufferRTC);
    Serial.print(" ");
    appendFile(SD, "/data.txt", bufferRTC);
    appendFile(SD, "/data.txt", " ");
  }
}

void loop() {  
  readpwm();
  if(outputSent){
      outputSent = false;
      combinedString = timeString + " " + pwmStr;
      webSocket.broadcastTXT(combinedString.c_str(), combinedString.length());
      pwmStr.remove(0, pwmStr.length());
  }
  
  webSocket.loop();

  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(WIFI_LED, HIGH);   // turn the LED on (HIGH is the voltage level)    
  }
  else{
    digitalWrite(WIFI_LED, LOW);   // turn the LED on (HIGH is the voltage level)    
  }
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    return;
  }
  if(file.print(message)) {
  } else {
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {

  unsigned long currentMillis = millis();

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    return;
  }
  if(file.print(message)) {
    if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(SD_LED, ledState);
  }
  } else {
  }
  file.close();
}