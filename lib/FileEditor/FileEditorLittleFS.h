#ifndef FileEditorLittleFS_H_
#define FileEditorLittleFS_H_
#include <ESPAsyncWebServer.h>

class FileEditorLittleFS: public AsyncWebHandler {
  private:
    fs::FS _fs;
    String _username;
    String _password; 
    bool _authenticated;
    uint32_t _startTime;
    String processTemplateProcessor(const String& var);
  public:
    FileEditorLittleFS(const fs::FS& fs, const String& username=String(), const String& password=String());
    virtual bool canHandle(AsyncWebServerRequest *request) override final;
    virtual void handleRequest(AsyncWebServerRequest *request) override final;
    virtual void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) override final;
    virtual bool isRequestHandlerTrivial() override final {return false;}
};

#endif