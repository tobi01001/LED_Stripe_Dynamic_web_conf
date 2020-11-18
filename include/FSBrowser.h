#include <LittleFS.h>
#include <Arduino.h>
//holds the current upload
File fsUploadFile;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  //if(server.hasArg("download")) return "application/octet-stream";
  //else 
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

/*
void handleFileRead(AsyncWebServerRequest *request){
  #ifdef DEBUG
  Serial.println("handleFileRead: " + path);
  #endif
  if (!handleFileRead(request)) server.send(404, "text/plain", "FileNotFound");
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(LittleFS.exists(pathWithGz) || LittleFS.exists(path)){
    if(LittleFS.exists(pathWithGz))
      path += ".gz";
    File file = LittleFS.open(path, "r");
    //size_t sent = server.streamFile(file, contentType);
    server.send(LittleFS, file, contentType)
    //server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
*/
void handleFileUpload(AsyncWebServerRequest *request){
  //webServer.sendHeader("Access-Control-Allow-Origin", "*");
  //webServer.send(200, "text/plain", "");
  if(request->url() != "/edit")
  {
    request->send(404, "text/html", "Did not work: " + request->url());
    return;
  } 
  else
  {
    request->send(200, "text/html", "That's awesome: " + request->url());
  }
  /*  
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    #ifdef DEBUG
    Serial.print("handleFileUpload Name: "); 
    Serial.println(filename);
    #endif
    fsUploadFile = LittleFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    #ifdef DEBUG
    Serial.print("handleFileUpload Size: "); 
    Serial.println(upload.totalSize);
    #endif
  }
  */
}

void handleFileDelete(AsyncWebServerRequest *request){
  if(request->params() == 0) {
    request->send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = request->getParam(0)->value();
  #ifdef DEBUG
  Serial.println("handleFileDelete: " + path);
  #endif
  if(path == "/")
    return request->send(500, "text/plain", "BAD PATH");
  if(!LittleFS.exists(path))
    return request->send(404, "text/plain", "FileNotFound");
  LittleFS.remove(path);
  request->send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(AsyncWebServerRequest *request){
  if(request->params() == 0)
    return request->send(500, "text/plain", "BAD ARGS");
  String path = request->getParam(0)->value(); //server.arg(0);
  #ifdef DEBUG 
  Serial.println("handleFileCreate: " + path);
  #endif
  if(path == "/")
    return request->send(500, "text/plain", "BAD PATH");
  if(LittleFS.exists(path))
    return request->send(500, "text/plain", "FILE EXISTS");
  File file = LittleFS.open(path, "w");
  if(file)
    file.close();
  else
    return request->send(500, "text/plain", "CREATE FAILED");
  request->send(200, "text/plain", "");
  path = String();
}



void handleFileList(AsyncWebServerRequest *request) {
  if(!request->hasParam("dir")) {request->send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = request->getParam("dir")->value();
  #ifdef DEBUG
  Serial.println("handleFileList: " + path);
  #endif
  Dir dir = LittleFS.openDir(path);
  path = String();
  String output = "[";
  while(dir.next()){
    bool isDir = false;
    if(dir.isDirectory()) isDir = true;
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name());
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  request->send(200, "text/json", output);
}

