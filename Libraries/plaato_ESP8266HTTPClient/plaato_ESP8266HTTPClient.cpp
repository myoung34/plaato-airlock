/**
 * ESP8266HTTPClient.cpp
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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <StreamString.h>
#include <base64.h>

#include "plaato_ESP8266HTTPClient.h"

class plaato_TransportTraits
{
public:
    virtual ~plaato_TransportTraits() 
    {
    }

    virtual std::unique_ptr<WiFiClient> create()
    {
        return std::unique_ptr<WiFiClient>(new WiFiClient());
    }

    virtual bool verify(WiFiClient&, const char*)
    {
        return true;
    }
};

class plaato_TLSTraits : public plaato_TransportTraits
{
public:
    plaato_TLSTraits(const String& fingerprint) :
        _fingerprint(fingerprint)
    {
    }

    std::unique_ptr<WiFiClient> create() override
    {
        return std::unique_ptr<WiFiClient>(new WiFiClientSecure());
    }

    bool verify(WiFiClient& client, const char* host) override
    {
        auto wcs = static_cast<WiFiClientSecure&>(client);
        // Load root certificate in DER format into WiFiClientSecure object
        bool res = wcs.setCACert_P(caCert, caCertLen);
        if (!res) {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][verify] Failed to load root CA certificate!");
        }
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][verify] Verifying Cert Chain...\n");
        return wcs.verifyCertChain(host);
    }

protected:
    String _fingerprint;
};

/**
 * constructor
 */
plaato_HTTPClient::plaato_HTTPClient()
{
}

/**
 * destructor
 */
plaato_HTTPClient::~plaato_HTTPClient()
{
    if(_tcp) {
        _tcp->stop();
    }
    if(_currentHeaders) {
        delete[] _currentHeaders;
    }
}

void plaato_HTTPClient::clear()
{
    _returnCode = 0;
    _size = -1;
    _headers = "";
}


bool plaato_HTTPClient::begin(String url, String httpsFingerprint)
{
    _transportTraits.reset(nullptr);
    _port = 443;
    if (httpsFingerprint.length() == 0) {
        return false;
    }
    if (!beginInternal(url, "https")) {
        return false;
    }
    _transportTraits = plaato_TransportTraitsPtr(new plaato_TLSTraits(httpsFingerprint));
    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] httpsFingerprint: %s\n", httpsFingerprint.c_str());
    return true;
}

/**
 * parsing the url for all needed parameters
 * @param url String
 */
bool plaato_HTTPClient::begin(String url)
{
    _transportTraits.reset(nullptr);
    _port = 80;
    if (!beginInternal(url, "http")) {
        return false;
    }
    _transportTraits = plaato_TransportTraitsPtr(new plaato_TransportTraits());
    return true;
}

bool plaato_HTTPClient::beginInternal(String url, const char* expectedProtocol)
{
    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] url: %s\n", url.c_str());
    clear();

    // check for : (http: or https:
    int index = url.indexOf(':');
    if(index < 0) {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] failed to parse protocol\n");
        return false;
    }

    _protocol = url.substring(0, index);
    url.remove(0, (index + 3)); // remove http:// or https://

    index = url.indexOf('/');
    String host = url.substring(0, index);
    url.remove(0, index); // remove host part

    // get Authorization
    index = host.indexOf('@');
    if(index >= 0) {
        // auth info
        String auth = host.substring(0, index);
        host.remove(0, index + 1); // remove auth part including @
        _base64Authorization = base64::encode(auth);
    }

    // get port
    index = host.indexOf(':');
    if(index >= 0) {
        _host = host.substring(0, index); // hostname
        host.remove(0, (index + 1)); // remove hostname + :
        _port = host.toInt(); // get port
    } else {
        _host = host;
    }
    _uri = url;
    if (_protocol != expectedProtocol) {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] unexpected protocol: %s, expected %s\n", _protocol.c_str(), expectedProtocol);
        return false;
    }
    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] host: %s port: %d url: %s\n", _host.c_str(), _port, _uri.c_str());
    return true;
}

bool plaato_HTTPClient::begin(String host, uint16_t port, String uri)
{
    clear();
    _host = host;
    _port = port;
    _uri = uri;
    _transportTraits = plaato_TransportTraitsPtr(new plaato_TransportTraits());
    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] host: %s port: %d uri: %s\n", host.c_str(), port, uri.c_str());
    return true;
}

bool plaato_HTTPClient::begin(String host, uint16_t port, String uri, bool https, String httpsFingerprint)
{
    if (https) {
        return begin(host, port, uri, httpsFingerprint);
    } else {
        return begin(host, port, uri);
    }
}

bool plaato_HTTPClient::begin(String host, uint16_t port, String uri, String httpsFingerprint)
{
    clear();
    _host = host;
    _port = port;
    _uri = uri;

    if (httpsFingerprint.length() == 0) {
        return false;
    }
    _transportTraits = plaato_TransportTraitsPtr(new plaato_TLSTraits(httpsFingerprint));
    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][begin] host: %s port: %d url: %s httpsFingerprint: %s\n", host.c_str(), port, uri.c_str(), httpsFingerprint.c_str());
    return true;
}

/**
 * end
 * called after the payload is handled
 */
void plaato_HTTPClient::end(void)
{
    if(connected()) {
        if(_tcp->available() > 0) {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][end] still data in buffer (%d), clean up.\n", _tcp->available());
            while(_tcp->available() > 0) {
                _tcp->read();
            }
        }
        if(_reuse && _canReuse) {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][end] tcp keep open for reuse\n");
        } else {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][end] tcp stop\n");
            _tcp->stop();
        }
    } else {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][end] tcp is closed\n");
    }
}

/**
 * connected
 * @return connected status
 */
bool plaato_HTTPClient::connected()
{
    if(_tcp) {
        return (_tcp->connected() || (_tcp->available() > 0));
    }
    return false;
}

/**
 * try to reuse the connection to the server
 * keep-alive
 * @param reuse bool
 */
void plaato_HTTPClient::setReuse(bool reuse)
{
    _reuse = reuse;
}

/**
 * set User Agent
 * @param userAgent const char *
 */
void plaato_HTTPClient::setUserAgent(const String& userAgent)
{
    _userAgent = userAgent;
}

/**
 * set the Authorizatio for the http request
 * @param user const char *
 * @param password const char *
 */
void plaato_HTTPClient::setAuthorization(const char * user, const char * password)
{
    if(user && password) {
        String auth = user;
        auth += ":";
        auth += password;
        _base64Authorization = base64::encode(auth);
    }
}

/**
 * set the Authorizatio for the http request
 * @param auth const char * base64
 */
void plaato_HTTPClient::setAuthorization(const char * auth)
{
    if(auth) {
        _base64Authorization = auth;
    }
}

/**
 * set the timeout for the TCP connection
 * @param timeout unsigned int
 */
void plaato_HTTPClient::setTimeout(uint16_t timeout)
{
    _tcpTimeout = timeout;
    if(connected()) {
        _tcp->setTimeout(timeout);
    }
}

/**
 * use HTTP1.0
 * @param timeout
 */
void plaato_HTTPClient::useHTTP10(bool useHTTP10)
{
    _useHTTP10 = useHTTP10;
}

/**
 * send a GET request
 * @return http code
 */
int plaato_HTTPClient::GET()
{
    return sendRequest("GET");
}

/**
 * sends a post request to the server
 * @param payload uint8_t *
 * @param size size_t
 * @return http code
 */
int plaato_HTTPClient::POST(uint8_t * payload, size_t size)
{
    return sendRequest("POST", payload, size);
}

int plaato_HTTPClient::POST(String payload)
{
    return POST((uint8_t *) payload.c_str(), payload.length());
}

/**
 * sends a put request to the server
 * @param payload uint8_t *
 * @param size size_t
 * @return http code
 */
int plaato_HTTPClient::PUT(uint8_t * payload, size_t size) {
    return sendRequest("PUT", payload, size);
}

int plaato_HTTPClient::PUT(String payload) {
    return PUT((uint8_t *) payload.c_str(), payload.length());
}

/**
 * sends a patch request to the server
 * @param payload uint8_t *
 * @param size size_t
 * @return http code
 */
int plaato_HTTPClient::PATCH(uint8_t * payload, size_t size) {
    return sendRequest("PATCH", payload, size);
}

int plaato_HTTPClient::PATCH(String payload) {
    return PATCH((uint8_t *) payload.c_str(), payload.length());
}

/**
 * sendRequest
 * @param type const char *     "GET", "POST", ....
 * @param payload String        data for the message body
 * @return
 */
int plaato_HTTPClient::sendRequest(const char * type, String payload)
{
    return sendRequest(type, (uint8_t *) payload.c_str(), payload.length());
}

/**
 * sendRequest
 * @param type const char *     "GET", "POST", ....
 * @param payload uint8_t *     data for the message body if null not send
 * @param size size_t           size for the message body if 0 not send
 * @return -1 if no info or > 0 when Content-Length is set by server
 */
int plaato_HTTPClient::sendRequest(const char * type, uint8_t * payload, size_t size)
{
    // connect to server
    if(!connect()) {
        return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }

    if(payload && size > 0) {
        addHeader(F("Content-Length"), String(size));
    }

    // send Header
    if(!sendHeader(type)) {
        return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
    }

    // send Payload if needed
    if(payload && size > 0) {
        if(_tcp->write(&payload[0], size) != size) {
            return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
        }
    }

    // handle Server Response (Header)
    return returnError(handleHeaderResponse());
}

/**
 * sendRequest
 * @param type const char *     "GET", "POST", ....
 * @param stream Stream *       data stream for the message body
 * @param size size_t           size for the message body if 0 not Content-Length is send
 * @return -1 if no info or > 0 when Content-Length is set by server
 */
int plaato_HTTPClient::sendRequest(const char * type, Stream * stream, size_t size)
{

    if(!stream) {
        return returnError(HTTPC_ERROR_NO_STREAM);
    }

    // connect to server
    if(!connect()) {
        return returnError(HTTPC_ERROR_CONNECTION_REFUSED);
    }

    if(size > 0) {
        addHeader("Content-Length", String(size));
    }

    // send Header
    if(!sendHeader(type)) {
        return returnError(HTTPC_ERROR_SEND_HEADER_FAILED);
    }

    int buff_size = HTTP_TCP_BUFFER_SIZE;

    int len = size;
    int bytesWritten = 0;

    if(len == 0) {
        len = -1;
    }

    // if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
    if((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
        buff_size = len;
    }

    // create buffer for read
    uint8_t * buff = (uint8_t *) malloc(buff_size);

    if(buff) {
        // read all data from stream and send it to server
        while(connected() && (stream->available() > -1) && (len > 0 || len == -1)) {

            // get available data size
            int sizeAvailable = stream->available();

            if(sizeAvailable) {

                int readBytes = sizeAvailable;

                // read only the asked bytes
                if(len > 0 && readBytes > len) {
                    readBytes = len;
                }

                // not read more the buffer can handle
                if(readBytes > buff_size) {
                    readBytes = buff_size;
                }

                // read data
                int bytesRead = stream->readBytes(buff, readBytes);

                // write it to Stream
                int bytesWrite = _tcp->write((const uint8_t *) buff, bytesRead);
                bytesWritten += bytesWrite;

                // are all Bytes a writen to stream ?
                if(bytesWrite != bytesRead) {
                    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] short write, asked for %d but got %d retry...\n", bytesRead, bytesWrite);

                    // check for write error
                    if(_tcp->getWriteError()) {
                        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] stream write error %d\n", _tcp->getWriteError());

                        //reset write error for retry
                        _tcp->clearWriteError();
                    }

                    // some time for the stream
                    delay(1);

                    int leftBytes = (readBytes - bytesWrite);

                    // retry to send the missed bytes
                    bytesWrite = _tcp->write((const uint8_t *) (buff + bytesWrite), leftBytes);
                    bytesWritten += bytesWrite;

                    if(bytesWrite != leftBytes) {
                        // failed again
                        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] short write, asked for %d but got %d failed.\n", leftBytes, bytesWrite);
                        free(buff);
                        return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
                    }
                }

                // check for write error
                if(_tcp->getWriteError()) {
                    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] stream write error %d\n", _tcp->getWriteError());
                    free(buff);
                    return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
                }

                // count bytes to read left
                if(len > 0) {
                    len -= readBytes;
                }

                delay(0);
            } else {
                delay(1);
            }
        }

        free(buff);

        if(size && (int) size != bytesWritten) {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] Stream payload bytesWritten %d and size %d mismatch!.\n", bytesWritten, size);
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] ERROR SEND PAYLOAD FAILED!");
            return returnError(HTTPC_ERROR_SEND_PAYLOAD_FAILED);
        } else {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] Stream payload written: %d\n", bytesWritten);
        }

    } else {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client][sendRequest] too less ram! need %d\n", HTTP_TCP_BUFFER_SIZE);
        return returnError(HTTPC_ERROR_TOO_LESS_RAM);
    }

    // handle Server Response (Header)
    return returnError(handleHeaderResponse());
}

/**
 * size of message body / payload
 * @return -1 if no info or > 0 when Content-Length is set by server
 */
int plaato_HTTPClient::getSize(void)
{
    return _size;
}

/**
 * returns the stream of the tcp connection
 * @return WiFiClient
 */
WiFiClient& plaato_HTTPClient::getStream(void)
{
    if(connected()) {
        return *_tcp;
    }

    DEBUG_HTTPCLIENT("[plaato_HTTP-Client] getStream: not connected\n");
    static WiFiClient empty;
    return empty;
}

/**
 * returns the stream of the tcp connection
 * @return WiFiClient *
 */
WiFiClient* plaato_HTTPClient::getStreamPtr(void)
{
    if(connected()) {
        return _tcp.get();
    }

    DEBUG_HTTPCLIENT("[plaato_HTTP-Client] getStreamPtr: not connected\n");
    return nullptr;
}

/**
 * write all  message body / payload to Stream
 * @param stream Stream *
 * @return bytes written ( negative values are error codes )
 */
int plaato_HTTPClient::writeToStream(Stream * stream)
{

    if(!stream) {
        return returnError(HTTPC_ERROR_NO_STREAM);
    }

    if(!connected()) {
        return returnError(HTTPC_ERROR_NOT_CONNECTED);
    }

    // get length of document (is -1 when Server sends no Content-Length header)
    int len = _size;
    int ret = 0;

    if(_transferEncoding == HTTPC_TE_IDENTITY) {
        ret = writeToStreamDataBlock(stream, len);

        // have we an error?
        if(ret < 0) {
            return returnError(ret);
        }
    } else if(_transferEncoding == HTTPC_TE_CHUNKED) {
        int size = 0;
        while(1) {
            if(!connected()) {
                return returnError(HTTPC_ERROR_CONNECTION_LOST);
            }
            String chunkHeader = _tcp->readStringUntil('\n');

            if(chunkHeader.length() <= 0) {
                return returnError(HTTPC_ERROR_READ_TIMEOUT);
            }

            chunkHeader.trim(); // remove \r

            // read size of chunk
            len = (uint32_t) strtol((const char *) chunkHeader.c_str(), NULL, 16);
            size += len;
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client] read chunk len: %d\n", len);

            // data left?
            if(len > 0) {
                int r = writeToStreamDataBlock(stream, len);
                if(r < 0) {
                    // error in writeToStreamDataBlock
                    return returnError(r);
                }
                ret += r;
            } else {

                // if no length Header use global chunk size
                if(_size <= 0) {
                    _size = size;
                }

                // check if we have write all data out
                if(ret != _size) {
                    return returnError(HTTPC_ERROR_STREAM_WRITE);
                }
                break;
            }

            // read trailing \r\n at the end of the chunk
            char buf[2];
            auto trailing_seq_len = _tcp->readBytes((uint8_t*)buf, 2);
            if (trailing_seq_len != 2 || buf[0] != '\r' || buf[1] != '\n') {
                return returnError(HTTPC_ERROR_READ_TIMEOUT);
            }

            delay(0);
        }
    } else {
        return returnError(HTTPC_ERROR_ENCODING);
    }

    end();
    return ret;
}

/**
 * return all payload as String (may need lot of ram or trigger out of memory!)
 * @return String
 */
String plaato_HTTPClient::getString(void)
{
    StreamString sstring;

    if(_size) {
        // try to reserve needed memmory
        if(!sstring.reserve((_size + 1))) {
            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][getString] not enough memory to reserve a string! need: %d\n", (_size + 1));
            return "";
        }
    }

    writeToStream(&sstring);
    return sstring;
}

/**
 * converts error code to String
 * @param error int
 * @return String
 */
String plaato_HTTPClient::errorToString(int error)
{
    switch(error) {
    case HTTPC_ERROR_CONNECTION_REFUSED:
        return F("connection refused");
    case HTTPC_ERROR_SEND_HEADER_FAILED:
        return F("send header failed");
    case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
        return F("send payload failed");
    case HTTPC_ERROR_NOT_CONNECTED:
        return F("not connected");
    case HTTPC_ERROR_CONNECTION_LOST:
        return F("connection lost");
    case HTTPC_ERROR_NO_STREAM:
        return F("no stream");
    case HTTPC_ERROR_NO_HTTP_SERVER:
        return F("no HTTP server");
    case HTTPC_ERROR_TOO_LESS_RAM:
        return F("too less ram");
    case HTTPC_ERROR_ENCODING:
        return F("Transfer-Encoding not supported");
    case HTTPC_ERROR_STREAM_WRITE:
        return F("Stream write error");
    case HTTPC_ERROR_READ_TIMEOUT:
        return F("read Timeout");
    default:
        return String();
    }
}

/**
 * adds Header to the request
 * @param name
 * @param value
 * @param first
 */
void plaato_HTTPClient::addHeader(const String& name, const String& value, bool first, bool replace)
{
    // not allow set of Header handled by code
    if(!name.equalsIgnoreCase(F("Connection")) &&
       !name.equalsIgnoreCase(F("User-Agent")) &&
       !name.equalsIgnoreCase(F("Host")) &&
       !(name.equalsIgnoreCase(F("Authorization")) && _base64Authorization.length())){

        String headerLine = name;
        headerLine += ": ";

        if (replace) {
            int headerStart = _headers.indexOf(headerLine);
            if (headerStart != -1) {
                int headerEnd = _headers.indexOf('\n', headerStart);
                _headers = _headers.substring(0, headerStart) + _headers.substring(headerEnd + 1);
            }
        }

        headerLine += value;
        headerLine += "\r\n";
        if(first) {
            _headers = headerLine + _headers;
        } else {
            _headers += headerLine;
        }
    }

}

void plaato_HTTPClient::collectHeaders(const char* headerKeys[], const size_t headerKeysCount)
{
    _headerKeysCount = headerKeysCount;
    if(_currentHeaders) {
        delete[] _currentHeaders;
    }
    _currentHeaders = new RequestArgument[_headerKeysCount];
    for(size_t i = 0; i < _headerKeysCount; i++) {
        _currentHeaders[i].key = headerKeys[i];
    }
}

String plaato_HTTPClient::header(const char* name)
{
    for(size_t i = 0; i < _headerKeysCount; ++i) {
        if(_currentHeaders[i].key == name) {
            return _currentHeaders[i].value;
        }
    }
    return String();
}

String plaato_HTTPClient::header(size_t i)
{
    if(i < _headerKeysCount) {
        return _currentHeaders[i].value;
    }
    return String();
}

String plaato_HTTPClient::headerName(size_t i)
{
    if(i < _headerKeysCount) {
        return _currentHeaders[i].key;
    }
    return String();
}

int plaato_HTTPClient::headers()
{
    return _headerKeysCount;
}

bool plaato_HTTPClient::hasHeader(const char* name)
{
    for(size_t i = 0; i < _headerKeysCount; ++i) {
        if((_currentHeaders[i].key == name) && (_currentHeaders[i].value.length() > 0)) {
            return true;
        }
    }
    return false;
}

/**
 * init TCP connection and handle ssl verify if needed
 * @return true if connection is ok
 */
bool plaato_HTTPClient::connect(void)
{

    if(connected()) {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client] connect. already connected, try reuse!\n");
        while(_tcp->available() > 0) {
            _tcp->read();
        }
        return true;
    }

    if (!_transportTraits) {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client] connect: HTTPClient::begin was not called or returned error\n");
        return false;
    }

    _tcp = _transportTraits->create();

    if(!_tcp->connect(_host.c_str(), _port)) {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client] failed connect to %s:%u\n", _host.c_str(), _port);
        return false;
    }

    DEBUG_HTTPCLIENT("[plaato_HTTP-Client] connected to %s:%u\n", _host.c_str(), _port);

    if (!_transportTraits->verify(*_tcp, _host.c_str())) {
        DEBUG_HTTPCLIENT("[plaato_HTTP-Client] transport level verify failed\n");
        _tcp->stop();
        return false;
    }

    // set Timeout for readBytesUntil and readStringUntil
    _tcp->setTimeout(_tcpTimeout);

#ifdef ESP8266
    _tcp->setNoDelay(true);
#endif
    return connected();
}

/**
 * sends HTTP request header
 * @param type (GET, POST, ...)
 * @return status
 */
bool plaato_HTTPClient::sendHeader(const char * type)
{
    if(!connected()) {
        return false;
    }

    String header = String(type) + " " + _uri + F(" HTTP/1.");

    if(_useHTTP10) {
        header += "0";
    } else {
        header += "1";
    }

    header += String(F("\r\nHost: ")) + _host;
    if (_port != 80 && _port != 443)
    {
        header += ':';
        header += String(_port);
    }
    header += String(F("\r\nUser-Agent: ")) + _userAgent +
              F("\r\nConnection: ");

    if(_reuse) {
        header += F("keep-alive");
    } else {
        header += F("close");
    }
    header += "\r\n";

    if(!_useHTTP10) {
        header += F("Accept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\n");
    }

    if(_base64Authorization.length()) {
        _base64Authorization.replace("\n", "");
        header += F("Authorization: Basic ");
        header += _base64Authorization;
        header += "\r\n";
    }

    header += _headers + "\r\n";

    return (_tcp->write((const uint8_t *) header.c_str(), header.length()) == header.length());
}

/**
 * reads the response from the server
 * @return int http code
 */
int plaato_HTTPClient::handleHeaderResponse()
{

    if(!connected()) {
        return HTTPC_ERROR_NOT_CONNECTED;
    }

    String transferEncoding;
    _returnCode = -1;
    _size = -1;
    _transferEncoding = HTTPC_TE_IDENTITY;
    unsigned long lastDataTime = millis();

    while(connected()) {
        size_t len = _tcp->available();
        if(len > 0) {
            String headerLine = _tcp->readStringUntil('\n');
            headerLine.trim(); // remove \r

            lastDataTime = millis();

            DEBUG_HTTPCLIENT("[plaato_HTTP-Client][handleHeaderResponse] RX: '%s'\n", headerLine.c_str());

            if(headerLine.startsWith("HTTP/1.")) {
                _returnCode = headerLine.substring(9, headerLine.indexOf(' ', 9)).toInt();
            } else if(headerLine.indexOf(':')) {
                String headerName = headerLine.substring(0, headerLine.indexOf(':'));
                String headerValue = headerLine.substring(headerLine.indexOf(':') + 1);
                headerValue.trim();

                if(headerName.equalsIgnoreCase("Content-Length")) {
                    _size = headerValue.toInt();
                }

                if(headerName.equalsIgnoreCase("Connection")) {
                    _canReuse = headerValue.equalsIgnoreCase("keep-alive");
                }

                if(headerName.equalsIgnoreCase("Transfer-Encoding")) {
                    transferEncoding = headerValue;
                }

                for(size_t i = 0; i < _headerKeysCount; i++) {
                    if(_currentHeaders[i].key.equalsIgnoreCase(headerName)) {
                        _currentHeaders[i].value = headerValue;
                        break;
                    }
                }
            }

            if(headerLine == "") {
                DEBUG_HTTPCLIENT("[plaato_HTTP-Client][handleHeaderResponse] code: %d\n", _returnCode);

                if(_size > 0) {
                    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][handleHeaderResponse] size: %d\n", _size);
                }

                if(transferEncoding.length() > 0) {
                    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][handleHeaderResponse] Transfer-Encoding: %s\n", transferEncoding.c_str());
                    if(transferEncoding.equalsIgnoreCase("chunked")) {
                        _transferEncoding = HTTPC_TE_CHUNKED;
                    } else {
                        return HTTPC_ERROR_ENCODING;
                    }
                } else {
                    _transferEncoding = HTTPC_TE_IDENTITY;
                }

                if(_returnCode) {
                    return _returnCode;
                } else {
                    DEBUG_HTTPCLIENT("[plaato_HTTP-Client][handleHeaderResponse] Remote host is not an HTTP Server!");
                    return HTTPC_ERROR_NO_HTTP_SERVER;
                }
            }

        } else {
            if((millis() - lastDataTime) > _tcpTimeout) {
                return HTTPC_ERROR_READ_TIMEOUT;
            }
            delay(0);
        }
    }

    return HTTPC_ERROR_CONNECTION_LOST;
}

/**
 * write one Data Block to Stream
 * @param stream Stream *
 * @param size int
 * @return < 0 = error >= 0 = size written
 */
int plaato_HTTPClient::writeToStreamDataBlock(Stream * stream, int size)
{
    int buff_size = HTTP_TCP_BUFFER_SIZE;
    int len = size;
    int bytesWritten = 0;

    // if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
    if((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
        buff_size = len;
    }

    // create buffer for read
    uint8_t * buff = (uint8_t *) malloc(buff_size);

    if(buff) {
        // read all data from server
        while(connected() && (len > 0 || len == -1)) {

            // get available data size
            size_t sizeAvailable = _tcp->available();

            if(sizeAvailable) {

                int readBytes = sizeAvailable;

                // read only the asked bytes
                if(len > 0 && readBytes > len) {
                    readBytes = len;
                }

                // not read more the buffer can handle
                if(readBytes > buff_size) {
                    readBytes = buff_size;
                }

                // read data
                int bytesRead = _tcp->readBytes(buff, readBytes);

                // write it to Stream
                int bytesWrite = stream->write(buff, bytesRead);
                bytesWritten += bytesWrite;

                // are all Bytes a writen to stream ?
                if(bytesWrite != bytesRead) {
                    DEBUG_HTTPCLIENT("[HTTP-Client][writeToStream] short write asked for %d but got %d retry...\n", bytesRead, bytesWrite);

                    // check for write error
                    if(stream->getWriteError()) {
                        DEBUG_HTTPCLIENT("[HTTP-Client][writeToStreamDataBlock] stream write error %d\n", stream->getWriteError());

                        //reset write error for retry
                        stream->clearWriteError();
                    }

                    // some time for the stream
                    delay(1);

                    int leftBytes = (readBytes - bytesWrite);

                    // retry to send the missed bytes
                    bytesWrite = stream->write((buff + bytesWrite), leftBytes);
                    bytesWritten += bytesWrite;

                    if(bytesWrite != leftBytes) {
                        // failed again
                        DEBUG_HTTPCLIENT("[HTTP-Client][writeToStream] short write asked for %d but got %d failed.\n", leftBytes, bytesWrite);
                        free(buff);
                        return HTTPC_ERROR_STREAM_WRITE;
                    }
                }

                // check for write error
                if(stream->getWriteError()) {
                    DEBUG_HTTPCLIENT("[HTTP-Client][writeToStreamDataBlock] stream write error %d\n", stream->getWriteError());
                    free(buff);
                    return HTTPC_ERROR_STREAM_WRITE;
                }

                // count bytes to read left
                if(len > 0) {
                    len -= readBytes;
                }

                delay(0);
            } else {
                delay(1);
            }
        }

        free(buff);

        DEBUG_HTTPCLIENT("[HTTP-Client][writeToStreamDataBlock] connection closed or file end (written: %d).\n", bytesWritten);

        if((size > 0) && (size != bytesWritten)) {
            DEBUG_HTTPCLIENT("[HTTP-Client][writeToStreamDataBlock] bytesWritten %d and size %d mismatch!.\n", bytesWritten, size);
            return HTTPC_ERROR_STREAM_WRITE;
        }

    } else {
        DEBUG_HTTPCLIENT("[HTTP-Client][writeToStreamDataBlock] too less ram! need %d\n", HTTP_TCP_BUFFER_SIZE);
        return HTTPC_ERROR_TOO_LESS_RAM;
    }

    return bytesWritten;
}

/**
 * called to handle error return, may disconnect the connection if still exists
 * @param error
 * @return error
 */
int plaato_HTTPClient::returnError(int error)
{
    if(error < 0) {
        DEBUG_HTTPCLIENT("[HTTP-Client][returnError] error(%d): %s\n", error, errorToString(error).c_str());
        if(connected()) {
            DEBUG_HTTPCLIENT("[HTTP-Client][returnError] tcp stop\n");
            _tcp->stop();
        }
    }
    return error;
}
