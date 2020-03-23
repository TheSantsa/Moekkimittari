/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Replace with your network credentials
extern const char* ssid;
extern const char* password;

#define DHTPIN 5     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Lämpöchartti</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <script src = "https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.3/Chart.min.js"></script>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
  <style>
    canvas{
      -moz-user-select: none;
      -webkit-user-select: none;
      -ms-user-select: none;
    }
   
    /* Data Table Styling */
    #dataTable {
      font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
      border-collapse: collapse;
      width: auto;
    }
   
    #dataTable td, #dataTable th {
      border: 1px solid #ddd;
      padding: 8px;
    }
   
    #dataTable tr:nth-child(even){background-color: #f2f2f2;}
   
    #dataTable tr:hover {background-color: #ddd;}
   
    #dataTable th {
      padding-top: 12px;
      padding-bottom: 12px;
      text-align: left;
      background-color: #4CAF50;
      color: white;
    }
  </style>
</head>
<body>
  <h2>Mökkimittari 9000</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#f30101;"></i> 
    <span class="dht-labels">Lämpötila</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Ilmankosteus</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <div style="text-align:center;"></div>
    <div class="chart-container" style=" position: relative; height:350px; width:100%">
        <canvas id="Chart" width="400" height="400"></canvas>
    </div>
  <br>
  <script>
  //Graphs visit: https://www.chartjs.org
  var tempArray = [];
  var humArray = [];
  var timeStamp = [];
  function showGraph()
  {
      for (i = 0; i < arguments.length; i++) {
        tempArray.push(arguments[i]); 
        humArray.push(arguments[i]);      
      }
   
      var ctx = document.getElementById("Chart").getContext('2d');
      var Chart2 = new Chart(ctx, {
          type: 'line',
          data: {
              labels: timeStamp,  //Bottom Labeling
              datasets: [{
                  label: "Lämpötila [Celsius]",
                  yAxisID: "A",
                  fill: false,  //Try with true
                  backgroundColor: 'rgba( 243, 1, 1 , 1)', //Dot marker color
                  borderColor: 'rgba( 243, 1, 1 , 1)', //Graph Line Color
                  data: tempArray,
              },
              {
                  label: "Suhteellinen kosteus [prosenttia]",
                  yAxisID: "B",
                  fill: false,  //Try with true
                  backgroundColor: 'rgba( 0, 173, 214 , 1)', //Dot marker color
                  borderColor: 'rgba( 0, 173, 214 , 1)', //Graph Line Color
                  data: humArray,
              }],
          },
          
          options: {
              title: {
                      display: false,
                      text: "Lämpötila"
                  },
              animation: {
                          duration: 0
              },
              legend: {
                display: false
              },
              maintainAspectRatio: false,
              elements: {
              line: {
                      tension: 0.5 //Smoothening (Curved) of data lines
                  }
              },
              scales: {
                      yAxes: [{
                            id: "A",
                            position: "left",
                            scaleLabel:{
                              display: true,
                              labelString: "Lämpötila [Celsius]"
                            },
                            ticks: {
                                //beginAtZero:true
                            }
                          }, {
                            id: "B",
                            position: "right",
                            scaleLabel:{
                              display: true,
                              labelString: "Suhteellinen kosteus [prosenttia]"
                            },
                            ticks: {
                              //beginAtZero: true
                            }
                          }
                      ]
              }
          }
      });
   
  }
   
  //On Page load show graphs
  window.onload = function() {
  //  console.log(new Date().toLocaleTimeString());
    showGraph();
  };
   
  //Ajax script to get ADC voltage at every 5 Seconds 
  //Read This tutorial https://circuits4you.com/2018/02/04/esp8266-ajax-update-part-of-web-page-without-refreshing/
   
  setInterval(function() {
    // Call a function repetatively with 5 Second interval
    getData();
  }, 10000); //10000mSeconds update rate
   
  function getData() {
    var time = new Date().toLocaleTimeString();
    var TempValue = []
    var HumValue = []
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
       //Push the data in array
        TempValue = this.responseText; 
        tempArray.push(TempValue);        
      }
    };
    xhttp.open("GET", "/temperature", true); //Handle readADC server on ESP8266
    xhttp.send();
    //humidity
    var yhttp = new XMLHttpRequest();
    yhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
       //Push the data in array
        HumValue = this.responseText;
        humArray.push(HumValue);
      }
    };
    yhttp.open("GET", "/humidity", true); //Handle readADC server on ESP8266
    yhttp.send();
    timeStamp.push(time);
    showGraph();  //Update Graphs
  }
      
  </script>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return String(t);
  }
  else if (var == "HUMIDITY") {
    return String(h);
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", String(h).c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
  }
}
