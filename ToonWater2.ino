
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

// Set web server port number to 80
WiFiServer server(80);
String header;

//default custom static IP
char static_ip[16] = "192.168.10.137";
char static_gw[16] = "192.168.10.1";
char static_sn[16] = "255.255.255.0";
String localIP;
String gatewayIP;
String subnetMask ;

//domoticz
int domoticzWaterFlowIdx = 0;
int domoticzWaterCountIdx = 0;
char dompass[40] = "";
char domname[40] = "";
char domoticzIP[16] = "192.168.10.185";
char domoticzPort[20] = "8080";
char domoticzFlowIDX[20] = "161";
char domoticzQuantityIDX[20] = "162";
bool domEnable = false;
int  numberofpulses = 0;


int totalwaterquantity = 1031251;
const char* filename = "/lastvalue.txt";
int minuteflowString = 0;


//watermeter
int Pulse2 = D2;            // D1

int waterflow =1;
bool newpulse = true;
int inputValue2 = 0;
int pulsecount; //Variable to count number of pulses 

bool reset1 = false;
bool reset2 = false;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long startMillis2;  //some global variables available anywhere in the program
unsigned long startMillis3;  //some global variables available anywhere in the program
unsigned long currentMillis;
unsigned long currentMillis2;
unsigned long period = 30*1000;  //time between pulses, initial 30s
unsigned long period2 = 10*60000;  //time between saving of the total quantity to SPIFF, , initial 10 min
unsigned long timebetweenmillis; 

//flag for saving data
bool shouldSaveConfig = false;

void setup() {
  startMillis = millis();
  startMillis2 = millis();
  startMillis3 = millis();
  Serial.begin(9600);
  Serial.println("Booting");
  int i;
  String oldval = "";

  if(SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
  }

  //Format File System remove line after first time use
  //if(SPIFFS.format()){ Serial.println("File System Formated");} else { Serial.println("File System Formatting Error");}

  //Read File data
  File f = SPIFFS.open(filename, "r");
  if (!f) {
    Serial.println("file open failed");
  }
  else
  {
    Serial.println("Reading Data from File:    ");
    //Data from file
    for(i=0;i<f.size();i++) //Read upto complete file size
    {
      char c1 = (char)f.read();
      oldval += c1;      
    }
    Serial.println("total : " + oldval);
    totalwaterquantity = oldval.toInt();
    Serial.println(totalwaterquantity); 
    f.close();  //Close file
    Serial.println("        File Closed");
  }

  //write file data
  writeTotal();
    
  //wifi credentials
  if (SPIFFS.exists("/config.json")) {
    Serial.println("reading config file");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      if (json.success()) {
        Serial.println("\nparsed json");
        strcpy(domname, json["domoticzUserName"]);
        strcpy(dompass, json["domoticzPassWord"]);
        strcpy(domoticzIP, json["domoticzIP"]);
        strcpy(domoticzPort, json["domoticzPort"]);
        strcpy(domoticzFlowIDX, json["domoticzFlowIDX"]);
        strcpy(domoticzQuantityIDX, json["domoticzQuantityIDX"]);
        
        if(json["domEnable"]) {
            Serial.println("Domoticz Selected");
            domEnable=true;
          } else{
            Serial.println("Domoticz not Selected");
          }        
        if(json["ip"]) {
          Serial.println("setting custom ip from config");
          strcpy(static_ip, json["ip"]);
          strcpy(static_gw, json["gateway"]);
          strcpy(static_sn, json["subnet"]);
          Serial.println(static_ip);
        } else {
          Serial.println("no custom ip in config");
        }
      } else {
        Serial.println("failed to load json config");
      }
    }
  }

  //end wifi
  Serial.println(static_ip);
  WiFiManagerParameter custom_text1("<p>Select Checkbox for Domoticz");
  WiFiManagerParameter custom_domEnable("Domoticz", "Domoticz enable", "T", 2, "type=\"checkbox\" ");
  WiFiManagerParameter custom_text2("Domoticz parameters:");
  WiFiManagerParameter custom_domoticzUserName("Name", "Dom. User (empty for none)", domname, 40);
  WiFiManagerParameter custom_domoticzPassWord("Pass", "Dom. Pass (empty for none)", dompass, 40);
  WiFiManagerParameter custom_domoticzadress("domIP", "Domoticz IP Adress", domoticzIP, 40);
  WiFiManagerParameter custom_domoticzPort("domPort", "Domoticz  Port (8080)", domoticzPort, 40);
  WiFiManagerParameter custom_domoticzFlowIDX("domFlow", "Domoticz Flow IDX", domoticzFlowIDX, 20);
  WiFiManagerParameter custom_domoticzQuantityIDX("domQuantity", "Domoticz Quantity IDX", domoticzQuantityIDX, 20);
  WiFiManagerParameter custom_emptyLine(" <br>");
  WiFiManagerParameter custom_text3("<p>Select Static IP adress:");

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  wifiManager.setMinimumSignalQuality();

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  wifiManager.addParameter(&custom_emptyLine);
  wifiManager.addParameter(&custom_text1);
  wifiManager.addParameter(&custom_domEnable);
  wifiManager.addParameter(&custom_text2);
  wifiManager.addParameter(&custom_domoticzadress);
  wifiManager.addParameter(&custom_domoticzPort);
  wifiManager.addParameter(&custom_domoticzUserName);
  wifiManager.addParameter(&custom_domoticzPassWord);
  wifiManager.addParameter(&custom_domoticzFlowIDX);
  wifiManager.addParameter(&custom_domoticzQuantityIDX);
  wifiManager.addParameter(&custom_emptyLine);
  wifiManager.addParameter(&custom_text3);

  if (!wifiManager.autoConnect("AutoConnectAP", "")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println("connected...yeey :)");
  strcpy(domoticzIP, custom_domoticzadress.getValue());
  strcpy(domoticzPort, custom_domoticzPort.getValue());
  strcpy(domname, custom_domoticzUserName.getValue());
  strcpy(dompass, custom_domoticzPassWord.getValue());
  strcpy(domoticzFlowIDX, custom_domoticzFlowIDX.getValue());
  strcpy(domoticzQuantityIDX, custom_domoticzQuantityIDX.getValue());
  domEnable = (strncmp(custom_domEnable.getValue(), "T", 1) == 0);

  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["domEnable"] = domEnable;
    json["domoticzUserName"] = domname;
    json["domoticzPassWord"] = dompass;
    json["domoticzIP"] = domoticzIP;
    json["domoticzPort"] = domoticzPort;
    json["domoticzFlowIDX"] = domoticzFlowIDX;
    json["domoticzQuantityIDX"] = domoticzQuantityIDX;
    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  localIP= String() + WiFi.localIP()[0] + "." + WiFi.localIP()[1] + "." + WiFi.localIP()[2] + "." + WiFi.localIP()[3];
  gatewayIP= String() + WiFi.gatewayIP()[0] + "." + WiFi.gatewayIP()[1] + "." + WiFi.gatewayIP()[2] + "." + WiFi.gatewayIP()[3];
  subnetMask=String() + WiFi.subnetMask()[0] + "." + WiFi.subnetMask()[1] + "." + WiFi.subnetMask()[2] + "." + WiFi.subnetMask()[3];
  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Pull-up to prevent "floating" input for watersensor
  pinMode(Pulse2, INPUT_PULLUP);
  pinMode(BUILTIN_LED, OUTPUT);

  server.begin();
  
}

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


bool SendCommandToDomo(String domcommand){
  HTTPClient http;
  bool retVal = false;
  String url = "http://" + String(domoticzIP) + ":" + String(domoticzPort) + "/" + String(domcommand);;
  if (String(dompass) == ""){//without password
    url = "http://" + String(domoticzIP) + ":" + String(domoticzPort) + "/" + String(domcommand);
  }else{
    
    url = "http://" + String(domname) + ":" + String(dompass)+ "@" + String(domoticzIP) + ":" + String(domoticzPort) + "/" + String(domcommand);
  }
  Serial.print(url);
  http.begin(url); //HTTP
  int httpCode = http.GET();
  if (httpCode > 0)
  { // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      retVal = true;
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return retVal;
}



void writeTotal(){
  //write file data
  File f = SPIFFS.open(filename, "w");
  if (!f) {
    Serial.println("file open failed");
  }
  else
  {
      //Write data to file
      Serial.println("Writing Data to File");
      Serial.println( totalwaterquantity);
      f.print(totalwaterquantity);
      f.close();  //Close file
  }
}

void watermeter() {
  //inputValue2 = digitalRead(Pulse1) + digitalRead(Pulse2) + digitalRead(Pulse3) + digitalRead(Pulse4);
  inputValue2 = digitalRead(Pulse2);
  if (inputValue2 >= 1) {
    newpulse = true;
  }
  if ((inputValue2 == 0) && (newpulse == true)) {
    Serial.println("Pulse received ...");
    totalwaterquantity = totalwaterquantity + 1;
    timebetweenmillis = millis() - startMillis2;  //initial start time
    waterflow = 60000/timebetweenmillis;
    if(domEnable){
      String newcommand = "json.htm?type=command&param=udevice&idx=" + String(domoticzFlowIDX) + "&nvalue=0&svalue=" + String(waterflow);
      SendCommandToDomo(newcommand);
    }    
    minuteflowString = waterflow;
    startMillis2 = millis();
    String flowtext = "actual Flow: ";
    flowtext += waterflow;
    Serial.println(flowtext);
    if(domEnable){
      if (numberofpulses >=10){
          Serial.println("Pulses has reached 10 so sending total");
          //String newcommand = "json.htm?type=command&param=udevice&idx=" + String(domoticzQuantityIDX) + "&nvalue=0&svalue=1";
          //SendCommandToDomo(newcommand);
          String newcommand = "json.htm?type=command&param=udevice&idx=" + String(domoticzQuantityIDX) + "&nvalue=0&svalue=" + String(totalwaterquantity);
          SendCommandToDomo(newcommand);
          numberofpulses=1;
      }
      else{
        numberofpulses++;
      }
    }
    pulsecount++; //Increment the variable on every pulse
    newpulse = false;
  }
}

void webserver(){
    // Set web server port number to 80
    WiFiClient client = server.available();   // Listen for incoming clients

    if (client) {                             // If a new client connects,
      Serial.println("New Client.");          // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          header += c;
          if (c == '\n') {                    // if the byte is a newline character
            if (currentLine.length() == 0) {
            String newjson = "{\"waterflow\":\"";
            newjson += minuteflowString;
            newjson += "\",\"waterquantity\":\"";
            newjson += totalwaterquantity;
            newjson += "\"}";
             
            if (currentLine.length() == 0) { 
              //client.println("HTTP/1.1 200 OK");
              //client.println("Content-type:text/html");
              //client.println("Connection: close");
              //client.println();
              
              // Display the HTML web page
              if ((header.indexOf("GET /") >= 0) && (header.indexOf("GET /setnew") <0) && (header.indexOf("GET /json.html") <0) && (header.indexOf("GET /water.html") <0)) {
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println("");
                client.println("<!DOCTYPE html><html>");
                client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                client.println("<link rel=\"icon\" href=\"data:,\">");
                client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
                client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                client.println(".button2 {background-color: #77878A;}</style></head>");
                
                client.println("<body><h1>Toon WaterMeter</h1>");
                client.println("<p></p>");
              }

              if ((header.indexOf("GET /") >= 0) && (header.indexOf("GET /reset/") <0) && (header.indexOf("GET /setnew") <0) && (header.indexOf("GET /json.html") <0) && (header.indexOf("GET /water.html") <0)) {
                client.println("<p>" +newjson + "</p>");
                client.println("<p>gebruik /water.html of /json.html fom resultaten te lezen</p>");
                client.println("<p>gebruik /setnew?12344 om een nieuwe tellerstand in te geven (12344 id de nieuwe stand)</p>");
                client.println("<p></p>");
                client.println("<p><a href=\"/reset/req\"><button class=\"button\">Reset</button></a></p>");
                client.println("<p>Als reset wordt gedrukt dan wordt de unit gereset en wordt AutoConnect gestart</p>");
                client.println("<p>Maak verbinding met  192.168.4.1</p>");
                client.println("</body></html>");
                client.println();
              }
            }

            if (header.indexOf("GET /reset/req") >= 0) {
              reset1 = true;
              reset2 = false;
              Serial.print("Reset1 :");
              Serial.print(reset1);
              Serial.print(", Reset2 :");
              Serial.println(reset2);
              client.println("<p>Weet u zeker ?</p>");
              client.println("<p> </p>");
              client.println("<p><a href=\"/reset/ok\"><button class=\"button\">Ja</button></a><a href=\"/../..\"><button class=\"button\">Nee</button></a></p>");
              client.println("</body></html>");
            }

            if (header.indexOf("GET /reset/ok") >= 0) {
              reset2 = true;
              Serial.print("Reset1 :");
              Serial.print(reset1);
              Serial.print(", Reset2 :");
              Serial.println(reset2);
              client.println("<p>Reset carried out</p>");
              client.println("<p>Maak verbinding met wifinetwerk AutoConnectAP en gan naar 192.168.4.1 voor configuratie</p>");
              client.println("</body></html>");
            }

            if (header.indexOf("GET /water.html") >=0) { 
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println("");
                client.println(newjson);
            }

           if (header.indexOf("GET /setnew") >= 0) {
               Serial.println("Client request");
               int pointer = header.indexOf("?"); //get the command from the adressline
               int pointer2 = header.indexOf(" ", pointer);  //get the first space
               String webcommand= header.substring(pointer+1, pointer2);
               client.println("HTTP/1.1 200 OK");
               client.println("Content-type:text/html");
               client.println("Connection: close");
               client.println("");
               client.println("New quantity recived");
               client.println(webcommand);
               totalwaterquantity = webcommand.toInt();
               writeTotal();
            }

           if (header.indexOf("GET /json.html") >= 0) {
               Serial.println("Client request");
               String jsoncontent;
               StaticJsonBuffer<200> jsonBuffer;
                JsonObject& json = jsonBuffer.createObject();
                json["waterflow"] = minuteflowString;
                json["waterquantity"] = totalwaterquantity;
                json.prettyPrintTo(Serial);
                json.printTo(jsoncontent);
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println("");
                client.println(jsoncontent);
            }

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
    header= "";
    // Close the connection
    client.flush();
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}  


void loop() {
  webserver();
  watermeter();
  delay(50);

  if ((reset1) && (reset2)) { //all reset pages have been acknowledged and reset parameters have been set from the webpages
    delay(2000);
    Serial.println("Reset requested");
    Serial.println("deleting configuration file");
    SPIFFS.remove("/config.json");
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(500);
    ESP.reset();
  }

  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period) { //test whether the pulsetime has eleapsed
      String minuteflowtext = "actual Flow this last 30s: ";
      minuteflowString = pulsecount;
      if(domEnable){
        String newcommand = "json.htm?type=command&param=udevice&idx=" + String(domoticzFlowIDX) + "&nvalue=0&svalue=" + String(minuteflowString);
        SendCommandToDomo(newcommand);
      } 
      pulsecount = 0;  //Start counting from 0 again
      startMillis = currentMillis; 
  }
  

  // save total to SPIFF
  currentMillis2 = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis2 - startMillis3 > period2)
  {
    //save the totalwaterquantity to file
    writeTotal();
    startMillis3 = currentMillis2; 
  }
}
