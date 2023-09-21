// Licensed to Elasticsearch B.V. under one or more contributor
// license agreements. See the NOTICE file distributed with
// this work for additional information regarding copyright
// ownership. Elasticsearch B.V. licenses this file to you under
// the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

/*
  Simple sketch to send a temperature value to Elastic Cloud

  In this example we used the ArduinoJson library to create
  the body of the request
*/
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiSSLClient.h>
#include "ESClient.h" // The Arduino Elaticsearch client
#include "elasticsearch_config.h"

// WiFi settings
char ssid[] = WIFI_SECRET_SSID;
char pass[] = WIFI_SECRET_PASS;

// Elastic settings
char serverAddress[] = ELASTIC_ENDPOINT;
int serverPort = ELASTIC_PORT;

WiFiSSLClient wifi;

ESClient client = ESClient(wifi, serverAddress, serverPort); // Elasticsearch client
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  Serial.println("Started");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
  
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  client.setElasticCloudApiKey(ELASTIC_CLOUD_API_KEY); // Elastic Cloud API key
}

void loop() {
 
  float temperature;
  // Set the temperature from a sensor (removing the randomness)
  temperature = random(10,30) + random(0,100)/100.00;
  
  // Preprare the JSON with temperature and geopoint for Elasticsearch
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["device-id"] = DEVICE_ID;
  doc["location"]["type"] = "Point";
  doc["location"]["coordinates"][0] = DEVICE_GEO_LON;
  doc["location"]["coordinates"][1] = DEVICE_GEO_LAT;

  String temp;
  serializeJson(doc, temp);

  Serial.println("Sending to Elasticsearch:");
  Serial.println(temp);
  ESResponse indexResult;
  // Send the temperature to Elastic Cloud
  indexResult = client.index("temperature", temp, "pipeline=set-timestamp");
  
  DynamicJsonDocument result(1024);
  deserializeJson(result, indexResult.body);

  if (result["result"] == "created") {
    Serial.println("Created with _id: " + result["_id"].as<String>());
  } else {
    Serial.println("Error sending data: " + indexResult.body);

  }
  
  StaticJsonDocument<512> query;
  query["query"]["bool"]["filter"]["geo_distance"]["distance"] = DEVICE_GEO_DISTANCE;
  query["query"]["bool"]["filter"]["geo_distance"]["location"][0] = DEVICE_GEO_LON;
  query["query"]["bool"]["filter"]["geo_distance"]["location"][1] = DEVICE_GEO_LAT;
  query["aggs"]["avg_temp"]["avg"]["field"] = "temperature";
  query["size"] = 0;

  String search;
  serializeJson(query, search);
  Serial.println("Geo-location query:");
  Serial.println(search);

  ESResponse searchResult;
  // Send the temperature to Elastic Cloud
  searchResult = client.search("temperature", search);

  DynamicJsonDocument avg(512);
  deserializeJson(avg, searchResult.body);
  float avgTemp = avg["aggregations"]["avg_temp"]["value"];
  int numDevices = avg["hits"]["total"]["value"];
  Serial.println("Average temperature of " + String(numDevices) + " devices in " + DEVICE_GEO_DISTANCE + ": " + String(avgTemp));

  Serial.println("Wait 30 seconds");
  delay(30000);
}
