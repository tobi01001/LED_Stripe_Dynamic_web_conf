#include "FileEditorLittleFS.h"
#include <FS.h>

#define SPIFFS_MAXLENGTH_FILEPATH 32
const char *excludeListFile = "/.exclude.files";

typedef struct ExcludeListS {
    char *item;
    ExcludeListS *next;
} ExcludeList;

static ExcludeList *excludes = NULL;

static bool matchWild(const char *pattern, const char *testee) {
  const char *nxPat = NULL, *nxTst = NULL;

  while (*testee) {
    if (( *pattern == '?' ) || (*pattern == *testee)){
      pattern++;testee++;
      continue;
    }
    if (*pattern=='*'){
      nxPat=pattern++; nxTst=testee;
      continue;
    }
    if (nxPat){ 
      pattern = nxPat+1; testee=++nxTst;
      continue;
    }
    return false;
  }
  while (*pattern=='*'){pattern++;}  
  return (*pattern == 0);
}

static bool addExclude(const char *item){
    size_t len = strlen(item);
    if(!len){
        return false;
    }
    ExcludeList *e = (ExcludeList *)malloc(sizeof(ExcludeList));
    if(!e){
        return false;
    }
    e->item = (char *)malloc(len+1);
    if(!e->item){
        free(e);
        return false;
    }
    memcpy(e->item, item, len+1);
    e->next = excludes;
    excludes = e;
    return true;
}

static void loadExcludeList(fs::FS &_fs, const char *filename){
    static char linebuf[SPIFFS_MAXLENGTH_FILEPATH];
    fs::File excludeFile=_fs.open(filename, "r");
    if(!excludeFile){
        return;
    }
    
    if (excludeFile.size() > 0){
      uint8_t idx;
      bool isOverflowed = false;
      while (excludeFile.available()){
        linebuf[0] = '\0';
        idx = 0;
        int lastChar;
        do {
          lastChar = excludeFile.read();
          if(lastChar != '\r'){
            linebuf[idx++] = (char) lastChar;
          }
        } while ((lastChar >= 0) && (lastChar != '\n') && (idx < SPIFFS_MAXLENGTH_FILEPATH));

        if(isOverflowed){
          isOverflowed = (lastChar != '\n');
          continue;
        }
        isOverflowed = (idx >= SPIFFS_MAXLENGTH_FILEPATH);
        linebuf[idx-1] = '\0';
        if(!addExclude(linebuf)){
            excludeFile.close();
            return;
        }
      }
    }
    excludeFile.close();
}

static bool isExcluded(fs::FS &_fs, const char *filename) {
  if(excludes == NULL){
      loadExcludeList(_fs, excludeListFile);
  }
  ExcludeList *e = excludes;
  while(e != NULL){
    if (matchWild(e->item, filename)){
      return true;
    }
    e = e->next;
  }
  return false;
}

FileEditorLittleFS::FileEditorLittleFS(const fs::FS& fs, const String& username, const String& password)
:_fs(fs), _username(username), _password(password), _authenticated(false), _startTime(millis())
{}

bool FileEditorLittleFS::canHandle(AsyncWebServerRequest *request){
  if(request->url().equalsIgnoreCase("/edit")){
    if(request->method() == HTTP_GET){
      if(request->hasParam("list"))
        return true;
      if(request->hasParam("edit")){
        request->_tempFile = _fs.open(request->arg("edit"), "r");
        if(!request->_tempFile){
          return false;
        }
      }
      if(request->hasParam("download")){
        request->_tempFile = _fs.open(request->arg("download"), "r");
        if(!request->_tempFile){
          return false;
        }
      }
      request->addInterestingHeader("If-Modified-Since");
      return true;
    }
    else if(request->method() == HTTP_POST)
      return true;
    else if(request->method() == HTTP_DELETE)
      return true;
    else if(request->method() == HTTP_PUT)
      return true;
  }
  return false;
}

void FileEditorLittleFS::handleRequest(AsyncWebServerRequest *request){
  if(_username.length() && _password.length() && !request->authenticate(_username.c_str(), _password.c_str()))
    return request->requestAuthentication();

  if(request->method() == HTTP_GET){
    if(request->hasParam("list")){
      String path = request->getParam("list")->value();
      String output = "[";
      
      // Improved LittleFS directory listing for ESP8266
      Dir dir = _fs.openDir(path);
      
      while(dir.next()){
        fs::File entry = dir.openFile("r");
        
        if (isExcluded(_fs, entry.name())) {
            entry.close();
            continue;
        }
        
        if (output != "[") output += ',';
        output += "{\"type\":\"";
        
        // Check if it's a directory by trying to open it as directory
        String fullPath = path;
        if (!fullPath.endsWith("/")) fullPath += "/";
        fullPath += String(entry.name());
        
        Dir testDir = _fs.openDir(fullPath);
        bool isDirectory = testDir.next(); // If we can list contents, it's a directory
        
        output += isDirectory ? "dir" : "file";
        output += "\",\"name\":\"";
        output += String(entry.name());
        output += "\",\"size\":";
        output += String(entry.size());
        output += ",\"path\":\"";
        output += fullPath;
        output += "\"}";
        
        entry.close();
      }
      
      output += "]";
      request->send(200, "application/json", output);
      output = String();
    }
    else if(request->hasParam("edit") || request->hasParam("download")){
      request->send(request->_tempFile, request->_tempFile.name(), String(), request->hasParam("download"));
    }
    else {
      request->send(200, "text/html", "ESP File Editor - use /edit?list=/ for directory listing");
    }
  }
  else if(request->method() == HTTP_DELETE){
    if(request->hasParam("path")){
      String path = request->arg("path");
      if(_fs.remove(path)){
        request->send(200, "text/plain", "Deleted successfully");
      } else {
        request->send(500, "text/plain", "Delete failed");
      }
    } else {
      request->send(400, "text/plain", "Bad request - missing path parameter");
    }
  }
  else if(request->method() == HTTP_PUT){
    if(request->hasParam("path")){
      String path = request->arg("path");
      if(path.indexOf('/') == 0){
        fs::File f = _fs.open(path, "w");
        if(f){
          f.close();
          request->send(200, "text/plain", "File created successfully");
        } else {
          request->send(500, "text/plain", "Create failed");
        }
      } else {
        request->send(400, "text/plain", "Bad path");
      }
    } else {
      request->send(400, "text/plain", "Bad request - missing path parameter");
    }
  }
  else if(request->method() == HTTP_POST){
    request->send(200, "text/plain", "File uploaded successfully");
  }
}

void FileEditorLittleFS::handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
  if(!index){
    request->_tempFile = _fs.open(filename, "w");
  }
  if(len){
    request->_tempFile.write(data, len);
  }
  if(final){
    request->_tempFile.close();
  }
}