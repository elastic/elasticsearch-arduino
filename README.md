## Elasticsearch Arduino Client

This is an **experimental library** to connect [Arduino](https://www.arduino.cc/) boards to [Elasticsearch](https://github.com/elastic/elasticsearch) and [Elastic Cloud](https://www.elastic.co/cloud/).

### Configuration using Elastic Cloud

[Elastic Cloud](https://www.elastic.co/cloud/) is the cloud solution offered by Elastic.

You can connect your Arduino to Elastic Cloud in order to send data (e.g. coming from sensors)
or to search for aggregated data (e.g. the average between two dates).

Using this library you can use all the features of Elasticsearch to analyze and consume data coming from your Arduino boards.
Moreover, using [Kibana](https://www.elastic.co/kibana/) included in Elastic Cloud, you can build advanced dashboard to monitor your data in real-time.

To connect your Arduino to Elastic Cloud you need the following information:
- the Elasticsearch server address (i.e. public URL);
- the Elastic Cloud API key;

You can retrieve these information from your Elastic Cloud dashboard. You need to open the 
**Manage Deployments** page and **Copy endpoint** of Elasticsearch (see screenshot below).

![Endpoint](docs/images/copy_endpoint.png)

This endpoint is the server address of Arduino (e.g. https://my-deploy.es.us-central1.gcp.cloud.es.io).

After this step, you need to generate an `API key` in the `Management` page under the section 
`Security`.

![Security](docs/images/create_api_key.png)

When you click on `Create API key` button you can choose a name and set the 
other options (for example, restrict privileges, expire after time, and so on).

![Choose an API name](docs/images/api_key_name.png)

After this step you will get the `API key`in the API keys page. 

![API key](docs/images/cloud_api_key.png)

**IMPORTANT**: you need to copy and store the `API key`in a secure place, since 
you will not be able to view it again in Elastic Cloud.

When you have the Elasticsearch address and the `API Key` you can store these
information in the `elasticsearch_config.h` (see the [examples](examples)).

```
#define ELASTIC_ENDPOINT ""
#define ELASTIC_CLOUD_API_KEY ""
#define ELASTIC_PORT 443
```

The `ELASTIC_PORT` is 443 in case of Elastic Cloud (using https).

### Configuration using Elasticsearch on-premise

You can use the Elasticsearch Arduino client also to connect to a local Elasticsearch
managed by you.

You can use two different approaches using Basic Authentication or OAuth2 token.

#### Basic authentication

Usually with basic authentication you have a `username` and a `password` to be used to
connect to Elasticsearch. These informations are reported when you execute Elasticsearch
server. With these information you can use the `ESClient::setBasicAuth(username, password)`
to configure the connection.

We suggest to store the `username` and `password` in the `elasticsearch_config.h`:

```
#define ELASTIC_USERNAME ""
#define ELASTIC_PASSWORD ""
```

#### OAuth2 token

ELasticsearch supports the OAuth2 Bearer authentication. You need to have a valid token to
be able to connect to Elasticsearch. This token can be generated using the procedure
reported in the [official documentation](https://www.elastic.co/guide/en/elasticsearch/reference/current/security-api-get-token.html).

When you have a valid token, you can use the `ESClient::setOAuth2Token(token)` to configure
the connection.

We suggest to store the OAuth2 token in the `elasticsearch_config.h`:

```
#define ELASTIC_OAUTH2_TOKEN ""
```

### Arduino models tested

The library has been tested with the following Arduino models:

- [MKR WiFi 1010](https://docs.arduino.cc/hardware/mkr-wifi-1010)
- [NANO RP2040 Connect](https://docs.arduino.cc/hardware/nano-rp2040-connect)
- [Portenta H7](https://www.arduino.cc/pro/hardware-product-portenta-h7)

### Examples

In the `examples` folder you can find some sketch examples.
The [geo_data_index_and_search.ino](/examples/geo_data_index_and_search.ino) send a temperature
in Elastic Cloud every 30 seconds.

After sending the data it also performs a geographic query to retrieve the average
temperature of the neighbour devices in a range of 50 Km.
This feedback information can be useful to adjust other parameter of the device.

The examples are based on the following `temperature` time series mapping in Elaticsearch:

```
PUT /temperature
{
  "mappings": {
    "properties": {
      "temperature": { "type": "float" }, 
      "timestamp":   { "type": "date" }, 
      "location":    { "type": "geo_point" },
      "device-id":   { "type": "keyword" }
    }
  }
}
```

To automatically insert a timestamp each time a device send a temperature to Elastic Cloud
we used a custom [ingest pipeline](https://www.elastic.co/guide/en/elasticsearch/reference/current/ingest.html).

This pipeline is performed before storing the input data to Elastic Cloud. In our case, we
wanted to add a simple `timestamp` so we created this `set-timestamp` pipeline:

```
PUT _ingest/pipeline/set-timestamp
{
  "description": "sets the timestamp",
  "processors": [
    {
      "set": {
        "field": "timestamp",
        "value": "{{{_ingest.timestamp}}}"
      }
    }
  ]
}
```

We can send data to Elastic Cloud using the `?pipeline=set-timestamp` query string, 
during an `index` operation.

To retrieve the average temperature of the neighbour devices in a range of 50 Km we can use the
following [geo-distance query](https://www.elastic.co/guide/en/elasticsearch/reference/current/query-dsl-geo-distance-query.html):

```
GET /temperature/_search
{
  "query": {
    "bool": {
      "must": {
        "match_all": {}
      },
      "filter": {
        "geo_distance": {
          "distance": "50km",
          "location": [-71.34, 41.12]
        }
      }
    }
  },
  "aggs": {
    "avg_temp": { "avg": { "field": "temperature" } }
  }
}

```

Here the location `[-71.34, 41.12]` represents longitude and latitude of the Arduino device.

### License

Copyright 2023 [Elasticsearch B.V.](https://www.elastic.co/) Licensed under the Apache License, Version 2.0.
