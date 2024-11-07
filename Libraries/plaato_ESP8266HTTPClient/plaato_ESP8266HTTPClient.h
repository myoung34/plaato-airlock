/**
 * ESP8266HTTPClient.h
 *
 * Created on: 02.11.2015
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the ESP8266HTTPClient for Arduino.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef PLAATO_ESP8266HTTPClient_H_
#define PLAATO_ESP8266HTTPClient_H_

#include <ESP8266HTTPClient.h>


const unsigned char caCert[] PROGMEM = {
  0x30, 0x82, 0x03, 0xc5, 0x30, 0x82, 0x02, 0xad, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x10, 0x02, 0xac, 0x5c, 0x26, 0x6a, 0x0b, 0x40, 0x9b, 0x8f,
  0x0b, 0x79, 0xf2, 0xae, 0x46, 0x25, 0x77, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x6c,
  0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
  0x53, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0c,
  0x44, 0x69, 0x67, 0x69, 0x43, 0x65, 0x72, 0x74, 0x20, 0x49, 0x6e, 0x63,
  0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x10, 0x77,
  0x77, 0x77, 0x2e, 0x64, 0x69, 0x67, 0x69, 0x63, 0x65, 0x72, 0x74, 0x2e,
  0x63, 0x6f, 0x6d, 0x31, 0x2b, 0x30, 0x29, 0x06, 0x03, 0x55, 0x04, 0x03,
  0x13, 0x22, 0x44, 0x69, 0x67, 0x69, 0x43, 0x65, 0x72, 0x74, 0x20, 0x48,
  0x69, 0x67, 0x68, 0x20, 0x41, 0x73, 0x73, 0x75, 0x72, 0x61, 0x6e, 0x63,
  0x65, 0x20, 0x45, 0x56, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43, 0x41,
  0x30, 0x1e, 0x17, 0x0d, 0x30, 0x36, 0x31, 0x31, 0x31, 0x30, 0x30, 0x30,
  0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x33, 0x31, 0x31, 0x31, 0x31,
  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x30, 0x6c, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31,
  0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0c, 0x44, 0x69,
  0x67, 0x69, 0x43, 0x65, 0x72, 0x74, 0x20, 0x49, 0x6e, 0x63, 0x31, 0x19,
  0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x10, 0x77, 0x77, 0x77,
  0x2e, 0x64, 0x69, 0x67, 0x69, 0x63, 0x65, 0x72, 0x74, 0x2e, 0x63, 0x6f,
  0x6d, 0x31, 0x2b, 0x30, 0x29, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x22,
  0x44, 0x69, 0x67, 0x69, 0x43, 0x65, 0x72, 0x74, 0x20, 0x48, 0x69, 0x67,
  0x68, 0x20, 0x41, 0x73, 0x73, 0x75, 0x72, 0x61, 0x6e, 0x63, 0x65, 0x20,
  0x45, 0x56, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43, 0x41, 0x30, 0x82,
  0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82,
  0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xc6, 0xcc, 0xe5, 0x73, 0xe6,
  0xfb, 0xd4, 0xbb, 0xe5, 0x2d, 0x2d, 0x32, 0xa6, 0xdf, 0xe5, 0x81, 0x3f,
  0xc9, 0xcd, 0x25, 0x49, 0xb6, 0x71, 0x2a, 0xc3, 0xd5, 0x94, 0x34, 0x67,
  0xa2, 0x0a, 0x1c, 0xb0, 0x5f, 0x69, 0xa6, 0x40, 0xb1, 0xc4, 0xb7, 0xb2,
  0x8f, 0xd0, 0x98, 0xa4, 0xa9, 0x41, 0x59, 0x3a, 0xd3, 0xdc, 0x94, 0xd6,
  0x3c, 0xdb, 0x74, 0x38, 0xa4, 0x4a, 0xcc, 0x4d, 0x25, 0x82, 0xf7, 0x4a,
  0xa5, 0x53, 0x12, 0x38, 0xee, 0xf3, 0x49, 0x6d, 0x71, 0x91, 0x7e, 0x63,
  0xb6, 0xab, 0xa6, 0x5f, 0xc3, 0xa4, 0x84, 0xf8, 0x4f, 0x62, 0x51, 0xbe,
  0xf8, 0xc5, 0xec, 0xdb, 0x38, 0x92, 0xe3, 0x06, 0xe5, 0x08, 0x91, 0x0c,
  0xc4, 0x28, 0x41, 0x55, 0xfb, 0xcb, 0x5a, 0x89, 0x15, 0x7e, 0x71, 0xe8,
  0x35, 0xbf, 0x4d, 0x72, 0x09, 0x3d, 0xbe, 0x3a, 0x38, 0x50, 0x5b, 0x77,
  0x31, 0x1b, 0x8d, 0xb3, 0xc7, 0x24, 0x45, 0x9a, 0xa7, 0xac, 0x6d, 0x00,
  0x14, 0x5a, 0x04, 0xb7, 0xba, 0x13, 0xeb, 0x51, 0x0a, 0x98, 0x41, 0x41,
  0x22, 0x4e, 0x65, 0x61, 0x87, 0x81, 0x41, 0x50, 0xa6, 0x79, 0x5c, 0x89,
  0xde, 0x19, 0x4a, 0x57, 0xd5, 0x2e, 0xe6, 0x5d, 0x1c, 0x53, 0x2c, 0x7e,
  0x98, 0xcd, 0x1a, 0x06, 0x16, 0xa4, 0x68, 0x73, 0xd0, 0x34, 0x04, 0x13,
  0x5c, 0xa1, 0x71, 0xd3, 0x5a, 0x7c, 0x55, 0xdb, 0x5e, 0x64, 0xe1, 0x37,
  0x87, 0x30, 0x56, 0x04, 0xe5, 0x11, 0xb4, 0x29, 0x80, 0x12, 0xf1, 0x79,
  0x39, 0x88, 0xa2, 0x02, 0x11, 0x7c, 0x27, 0x66, 0xb7, 0x88, 0xb7, 0x78,
  0xf2, 0xca, 0x0a, 0xa8, 0x38, 0xab, 0x0a, 0x64, 0xc2, 0xbf, 0x66, 0x5d,
  0x95, 0x84, 0xc1, 0xa1, 0x25, 0x1e, 0x87, 0x5d, 0x1a, 0x50, 0x0b, 0x20,
  0x12, 0xcc, 0x41, 0xbb, 0x6e, 0x0b, 0x51, 0x38, 0xb8, 0x4b, 0xcb, 0x02,
  0x03, 0x01, 0x00, 0x01, 0xa3, 0x63, 0x30, 0x61, 0x30, 0x0e, 0x06, 0x03,
  0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x01, 0x86,
  0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05,
  0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,
  0x04, 0x16, 0x04, 0x14, 0xb1, 0x3e, 0xc3, 0x69, 0x03, 0xf8, 0xbf, 0x47,
  0x01, 0xd4, 0x98, 0x26, 0x1a, 0x08, 0x02, 0xef, 0x63, 0x64, 0x2b, 0xc3,
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,
  0x14, 0xb1, 0x3e, 0xc3, 0x69, 0x03, 0xf8, 0xbf, 0x47, 0x01, 0xd4, 0x98,
  0x26, 0x1a, 0x08, 0x02, 0xef, 0x63, 0x64, 0x2b, 0xc3, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00,
  0x03, 0x82, 0x01, 0x01, 0x00, 0x1c, 0x1a, 0x06, 0x97, 0xdc, 0xd7, 0x9c,
  0x9f, 0x3c, 0x88, 0x66, 0x06, 0x08, 0x57, 0x21, 0xdb, 0x21, 0x47, 0xf8,
  0x2a, 0x67, 0xaa, 0xbf, 0x18, 0x32, 0x76, 0x40, 0x10, 0x57, 0xc1, 0x8a,
  0xf3, 0x7a, 0xd9, 0x11, 0x65, 0x8e, 0x35, 0xfa, 0x9e, 0xfc, 0x45, 0xb5,
  0x9e, 0xd9, 0x4c, 0x31, 0x4b, 0xb8, 0x91, 0xe8, 0x43, 0x2c, 0x8e, 0xb3,
  0x78, 0xce, 0xdb, 0xe3, 0x53, 0x79, 0x71, 0xd6, 0xe5, 0x21, 0x94, 0x01,
  0xda, 0x55, 0x87, 0x9a, 0x24, 0x64, 0xf6, 0x8a, 0x66, 0xcc, 0xde, 0x9c,
  0x37, 0xcd, 0xa8, 0x34, 0xb1, 0x69, 0x9b, 0x23, 0xc8, 0x9e, 0x78, 0x22,
  0x2b, 0x70, 0x43, 0xe3, 0x55, 0x47, 0x31, 0x61, 0x19, 0xef, 0x58, 0xc5,
  0x85, 0x2f, 0x4e, 0x30, 0xf6, 0xa0, 0x31, 0x16, 0x23, 0xc8, 0xe7, 0xe2,
  0x65, 0x16, 0x33, 0xcb, 0xbf, 0x1a, 0x1b, 0xa0, 0x3d, 0xf8, 0xca, 0x5e,
  0x8b, 0x31, 0x8b, 0x60, 0x08, 0x89, 0x2d, 0x0c, 0x06, 0x5c, 0x52, 0xb7,
  0xc4, 0xf9, 0x0a, 0x98, 0xd1, 0x15, 0x5f, 0x9f, 0x12, 0xbe, 0x7c, 0x36,
  0x63, 0x38, 0xbd, 0x44, 0xa4, 0x7f, 0xe4, 0x26, 0x2b, 0x0a, 0xc4, 0x97,
  0x69, 0x0d, 0xe9, 0x8c, 0xe2, 0xc0, 0x10, 0x57, 0xb8, 0xc8, 0x76, 0x12,
  0x91, 0x55, 0xf2, 0x48, 0x69, 0xd8, 0xbc, 0x2a, 0x02, 0x5b, 0x0f, 0x44,
  0xd4, 0x20, 0x31, 0xdb, 0xf4, 0xba, 0x70, 0x26, 0x5d, 0x90, 0x60, 0x9e,
  0xbc, 0x4b, 0x17, 0x09, 0x2f, 0xb4, 0xcb, 0x1e, 0x43, 0x68, 0xc9, 0x07,
  0x27, 0xc1, 0xd2, 0x5c, 0xf7, 0xea, 0x21, 0xb9, 0x68, 0x12, 0x9c, 0x3c,
  0x9c, 0xbf, 0x9e, 0xfc, 0x80, 0x5c, 0x9b, 0x63, 0xcd, 0xec, 0x47, 0xaa,
  0x25, 0x27, 0x67, 0xa0, 0x37, 0xf3, 0x00, 0x82, 0x7d, 0x54, 0xd7, 0xa9,
  0xf8, 0xe9, 0x2e, 0x13, 0xa3, 0x77, 0xe8, 0x1f, 0x4a
};
const unsigned int caCertLen = 969;


class plaato_TransportTraits;
typedef std::unique_ptr<plaato_TransportTraits> plaato_TransportTraitsPtr;

class plaato_HTTPClient
{
public:
    plaato_HTTPClient();
    ~plaato_HTTPClient();

    bool begin(String url);
    bool begin(String url, String httpsFingerprint);
    bool begin(String host, uint16_t port, String uri = "/");
    bool begin(String host, uint16_t port, String uri, String httpsFingerprint);
    // deprecated, use the overload above instead
    bool begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint)  __attribute__ ((deprecated));

    void end(void);

    bool connected(void);

    void setReuse(bool reuse); /// keep-alive
    void setUserAgent(const String& userAgent);
    void setAuthorization(const char * user, const char * password);
    void setAuthorization(const char * auth);
    void setTimeout(uint16_t timeout);

    void useHTTP10(bool usehttp10 = true);

    /// request handling
    int GET();
    int POST(uint8_t * payload, size_t size);
    int POST(String payload);
    int PUT(uint8_t * payload, size_t size);
    int PUT(String payload);
    int PATCH(uint8_t * payload, size_t size);
    int PATCH(String payload);
    int sendRequest(const char * type, String payload);
    int sendRequest(const char * type, uint8_t * payload = NULL, size_t size = 0);
    int sendRequest(const char * type, Stream * stream, size_t size = 0);

    void addHeader(const String& name, const String& value, bool first = false, bool replace = true);

    /// Response handling
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount);
    String header(const char* name);   // get request header value by name
    String header(size_t i);              // get request header value by number
    String headerName(size_t i);          // get request header name by number
    int headers();                     // get header count
    bool hasHeader(const char* name);  // check if header exists


    int getSize(void);

    WiFiClient& getStream(void);
    WiFiClient* getStreamPtr(void);
    int writeToStream(Stream* stream);
    String getString(void);

    static String errorToString(int error);

protected:
    struct RequestArgument {
        String key;
        String value;
    };

    bool beginInternal(String url, const char* expectedProtocol);
    void clear();
    int returnError(int error);
    bool connect(void);
    bool sendHeader(const char * type);
    int handleHeaderResponse();
    int writeToStreamDataBlock(Stream * stream, int len);


    plaato_TransportTraitsPtr _transportTraits;
    std::unique_ptr<WiFiClient> _tcp;

    /// request handling
    String _host;
    uint16_t _port = 0;
    bool _reuse = false;
    uint16_t _tcpTimeout = HTTPCLIENT_DEFAULT_TCP_TIMEOUT;
    bool _useHTTP10 = false;

    String _uri;
    String _protocol;
    String _headers;
    String _userAgent = "ESP8266HTTPClient";
    String _base64Authorization;

    /// Response handling
    RequestArgument* _currentHeaders = nullptr;
    size_t           _headerKeysCount = 0;

    int _returnCode = 0;
    int _size = -1;
    bool _canReuse = false;
    transferEncoding_t _transferEncoding = HTTPC_TE_IDENTITY;
};



#endif /* PLAATO_ESP8266HTTPClient_H_ */
