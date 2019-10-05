#include "RestClient.h"

#define DEFAULT_MIME "application/x-www-form-urlencoded"

RestClient::RestClient(const char* _host, Client& client) {
  host = _host;
  port = 80;
  num_headers = 0;
  setContentType(DEFAULT_MIME);
  setClient(client);
}

RestClient::RestClient(const char* _host, int _port, Client& client) {
  host = _host;
  port = _port;
  num_headers = 0;
  setContentType(DEFAULT_MIME);
  setClient(client);
}

RestClient& RestClient::setClient(Client& client) {
  this->client = &client;
  return *this;
}

// GET path
int RestClient::get(const char* path) {
  return request("GET", path, NULL, NULL, 0);
}

//GET path with response
int RestClient::get(const char* path, char* response, int length) {
  return request("GET", path, NULL, response, length);
}

// POST path and body
int RestClient::post(const char* path, const char* body) {
  return request("POST", path, body, NULL, 0);
}

// POST path and body with response
int RestClient::post(const char* path, const char* body, char* response, int length) {
  return request("POST", path, body, response, length);
}

// PUT path and body
int RestClient::put(const char* path, const char* body) {
  return request("PUT", path, body, NULL, 0);
}

// PUT path and body with response
int RestClient::put(const char* path, const char* body, char* response, int length) {
  return request("PUT", path, body, response, length);
}

// DELETE path
int RestClient::del(const char* path) {
  return request("DELETE", path, NULL, NULL, 0);
}

// DELETE path and response
int RestClient::del(const char* path, char* response, int length) {
  return request("DELETE", path, NULL, response, length);
}

// DELETE path and body
int RestClient::del(const char* path, const char* body) {
  return request("DELETE", path, body, NULL, 0);
}

// DELETE path and body with response
int RestClient::del(const char* path, const char* body, char* response, int length) {
  return request("DELETE", path, body, response, length);
}

void RestClient::write(const char* string) {
  client->print(string);
}

RestClient& RestClient::setHeader(const char* header) {
  headers[num_headers] = header;
  num_headers++;
  return *this;
}

RestClient& RestClient::setContentType(const char* contentTypeValue) {
  contentType = contentTypeValue;
  return *this;
}

// The mother- generic request method.
//
int RestClient::request(const char* method, const char* path,
  const char* body, char* response, int length) {

  int statusCode=client->connect(host, port);
  if (statusCode!=1) {
    return -1; // TIMED_OUT -1, INVALID_SERVER -2, TRUNCATED -3, INVALID_RESPONSE -4
  }
  // Make a HTTP request line:
  write(method);
  write(" ");
  write(path);
  write(" HTTP/1.1\r\n");
  for(int i = 0; i < num_headers; i++) {
    write(headers[i]);
    write("\r\n");
  }
  write("Host: ");
  write(host);
  write("\r\nConnection: close\r\n");

  if (body != NULL) {
    char contentLength[30];
    sprintf(contentLength, "Content-Length: %d\r\n", strlen(body)+1); //+1 because we are adding a CRLF after the body
    write(contentLength);

    write("Content-Type: ");
    write(contentType);
    write("\r\n");
  }

  write("\r\n");

  if (body != NULL) {
    write(body);
    write("\r\n\r\n"); // One CRLF is used to ensure the body end on one
  }

  //make sure you write all those bytes.
  client->flush();

  statusCode = readResponse(response, length);

  //cleanup
  num_headers = 0;
  client->stop();
  // delay(50);  Not sure why is here but I don't do repeated calls

  return statusCode;
}

int RestClient::readResponse(char* response, int length) {

  // an http request ends with a blank line
  boolean currentLineIsBlank = true;
  boolean httpBody = false;
  boolean inStatus = false;

  char statusCode[4];
  int i = 0;
  int code = 0;

  if (response) {
    memset(response, 0, length);
  }
	int writeIdx = 0;

  if (response == NULL) {
  } else {
  }

  while (client->connected()) {

    if (client->available()) {

      char c = client->read();

      if (c == ' ' && !inStatus) {
        inStatus = true;
      }

      if (inStatus && i < 3 && c != ' ') {
        statusCode[i] = c;
        i++;
      }
      if (i == 3) {
        statusCode[i] = '\0';
        code = atoi(statusCode);
      }

      if (response && httpBody && writeIdx < length - 1) {
        response[writeIdx++] = c;
      } else {
        if (c == '\n' && currentLineIsBlank) {
          httpBody = true;
        }

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
  }
  return code;
}
