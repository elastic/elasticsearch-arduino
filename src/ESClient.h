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

#ifndef ESClient_h
#define ESClient_h

#include <ArduinoHttpClient.h>
#include <Client.h>
// HTTP response from Elasticsearch
struct ESResponse {
    int statusCode;
    String body;
};

class ESClient {
public:

    const String version = "0.1.0";

    // Constructor
    ESClient(Client& wifi, char* serverAddress, int port);

    void setElasticCloudApiKey(String apiKey);
    void setBasicAuth(String username, String password);
    void setOAuth2Token(String token);  

    ESResponse index(String index, String body, String queryString = ""); 
    ESResponse get(String index, String id, String queryString = "");
    ESResponse del(String index, String id, String queryString = "");
    ESResponse update(String index, String id, String body, String queryString = "");
    ESResponse search(String index, String body, String queryString = "");
    ESResponse sendRequest(String method, String url, String body = "", String queryString = "", String contentType = "application/json");

protected:

    HttpClient client;

    String elasticCloudApiKey;
    String basicUsername;
    String basicPassword;
    String oauth2Token;
};
#endif
