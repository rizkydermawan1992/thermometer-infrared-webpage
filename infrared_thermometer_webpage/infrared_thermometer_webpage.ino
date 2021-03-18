
// Import required libraries
#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Replace with your network credentials
const char* ssid = "Jailangkung";
const char* password = "kenapatanyatanya";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const int buzzer = 14;

String obj() {
  float t_obj = mlx.readObjectTempC();
  if (isnan(t_obj)) {    
    Serial.println("Failed to read from MLX90614 sensor!");
    return "--";
  }
  else {
    Serial.print("Object = ");
    Serial.println(t_obj);
    return String(t_obj);
  }
}
String amb() {
  float t_amb = mlx.readAmbientTempC();
  if (isnan(t_amb)) {    
    Serial.println("Failed to read from MLX90614 sensor!");
    return "--";
  }
  else {
    Serial.print("Ambient = ");
    Serial.println(t_amb);
    return String(t_amb);
  }
}

String stat() {
  String Status;
  if(mlx.readObjectTempC() >= 37.5){
   Status = "Suspect";
   digitalWrite(buzzer, HIGH);
  }
  else if(mlx.readObjectTempC() < 37.5){
    Status = "Normal";
    digitalWrite(buzzer, LOW);
  }
  else{
    Status = "--";
  }
  Serial.print("Status = ");
  Serial.println(Status);
  return Status;
}
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .temp-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    img {
      width: 350px;
      height: 45px;
      padding-top: 7px;
      padding-bottom: 7px;
    }
    footer {
      padding-top: 10px;
      font-size: 1.3rem;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <img src="http://absenrfid.com/img/png2.png" alt="Responsive image">
  <h1>MLX90614 BODY SCAN</h1>
  <p>
    <i id = "icon-obj"></i> 
    <span class="temp-labels">Object: </span> 
    <span id="obj">%OBJECT%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#FF1493;"></i> 
    <span class="temp-labels">Ambient: </span> 
    <span id="amb">%AMBIENT%</span>
    <sup class="units">&deg;C</sup>
  </p>
   <p>
    <i id = "icon-stat" style="color:#6495ED;"></i> 
    <span class="temp-labels">Status: </span> 
    <span id="stat">%STATUS%</span>
  </p>

   <footer>Copyright &copy;2021 Rizky Project</footer>

</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("obj").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/obj", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("amb").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/amb", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("stat").innerHTML = this.responseText;
      if(this.responseText == "Suspect"){
         document.getElementById("stat").style.color = "#DC143C";
         document.getElementById("obj").style.color = "#DC143C";
         document.getElementById("icon-stat").setAttribute("class", "fas fa-user-times");
         document.getElementById("icon-stat").style.color="#DC143C";
         document.getElementById("icon-obj").setAttribute("class", "fas fa-temperature-high");
         document.getElementById("icon-obj").style.color="#DC143C";
      }
      else{
         document.getElementById("stat").style.color = "black";
         document.getElementById("obj").style.color = "black";
         document.getElementById("icon-stat").setAttribute("class", "fas fa-user-check");
         document.getElementById("icon-stat").style.color="#008B8B";
         document.getElementById("icon-obj").setAttribute("class", "fas fa-thermometer-half");
         document.getElementById("icon-obj").style.color="#228B22";
      }
    }
  };
  xhttp.open("GET", "/stat", true);
  xhttp.send();
}, 1000 ) ;
console.log(document.getElementById("stat"));
</script>
</html>)rawliteral";

// Replaces placeholder with MLX90614 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "OBJECT"){
    return obj();
  }
  else if(var == "AMBIENT"){
    return amb();
  }
  else if(var == "STATUS"){
    return stat();
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  mlx.begin();

  pinMode(buzzer, OUTPUT);
       
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/obj", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", obj().c_str());
  });
  server.on("/amb", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", amb().c_str());
  });
   server.on("/stat", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", stat().c_str());
  });
  // Start server
  server.begin();
}
 
void loop(){
 
}
