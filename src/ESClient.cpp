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

#include "ESClient.h"

// Constructor
ESClient::ESClient(Client& wifi, char* serverAddress, int port):
  client(wifi, serverAddress, port)
{
}

void ESClient::setElasticCloudApiKey(String apiKey){
  elasticCloudApiKey = apiKey;
}

void ESClient::setBasicAuth(String username, String password){
  basicUsername = username;
  basicPassword = password;
}

void ESClient::setOAuth2Token(String token){
  oauth2Token = token;
}
 
ESResponse ESClient::sendRequest(String method, String url, String body, String queryString, String contentType){
#ifdef LOGGING
    Serial.println("Sending " + method + " " + url);
    Serial.println("Content-type: " + contentType);
#endif

    if (queryString.length() > 0) {
      url += '?' + queryString;
    }
    client.connectionKeepAlive();
    client.beginRequest();
    if (method == "get") { 
      client.get(url);
    } else if (method == "post") {
      client.post(url);
    } else if (method == "put") {
      client.put(url);
    } else if (method == "delete") {
      client.del(url);
    }

    // Elastic Cloud API key
    if (elasticCloudApiKey.length() > 0) {
      client.sendHeader("Authorization", "ApiKey " + elasticCloudApiKey);
    }
    // Basic authentication
    if (basicUsername.length() > 0 && basicPassword.length() > 0) {
      client.sendBasicAuth(basicUsername, basicPassword);
    }
    // OAuth2 token
    if (oauth2Token.length() > 0) {
      client.sendHeader("Authorization", "Bearer " + oauth2Token);
    }
    // Add x-elastic-client-meta header
    client.sendHeader("x-elastic-client-meta", "arduino=" + version);

    // Add the body, if not empty
    if (body.length() > 0) {
#ifdef LOGGING
      Serial.println("Body: " + body);
#endif
      client.sendHeader("Content-Type", contentType);
      client.sendHeader("Content-Length", body.length());
      client.beginBody();
      client.print(body);
    }
    client.endRequest();

    ESResponse result;
    result.statusCode = client.responseStatusCode();
    result.body = client.responseBody();
#ifdef LOGGING
    Serial.println("Response status code: " + result.statusCode);
    Serial.println("Response body: " + result.body);
#endif    
    return result;
}

// Elasticsearch Index API
// @see https://www.elastic.co/guide/en/elasticsearch/reference/current/docs-index_.html
ESResponse ESClient::index(String index, String body, String queryString){
    return sendRequest("post", "/" + index + "/_doc", body, queryString);
}

// Elasticsearch Get API
// @see https://www.elastic.co/guide/en/elasticsearch/reference/current/docs-get.html
ESResponse ESClient::get(String index, String id, String queryString){
    return sendRequest("get", "/" + index + "/_doc/" + id, "", queryString);
}

// Elasticsearch Delete API
// @see https://www.elastic.co/guide/en/elasticsearch/reference/current/docs-delete.html
ESResponse ESClient::del(String index, String id, String queryString){
    return sendRequest("delete", "/" + index + "/_doc/" + id, "", queryString);
}

// Elasticsearch Update API
// @see https://www.elastic.co/guide/en/elasticsearch/reference/current/docs-update.html
ESResponse ESClient::update(String index, String id, String body, String queryString){
    return sendRequest("post", "/" + index + "/_update/" + id, body, queryString);
}

// Elasupdateticsearch Search API
// @see https://www.elastic.co/guide/en/elasticsearch/reference/current/search-search.html
ESResponse ESClient::search(String index, String body, String queryString){
    return sendRequest("post", "/" + index + "/_search", body, queryString);
}