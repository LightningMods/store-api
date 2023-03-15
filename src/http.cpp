#include <stdarg.h>
#include <stdlib.h> // calloc
#include <atomic>
#include "jsmn.h"
#include "utils.h"
#include <curl/curl.h>
#include "fmt/format.h"
#include "fmt/printf.h"
#include "fmt/chrono.h"
#include <unistd.h>
#include "defines.h"

#define ORBIS_TRUE 1

struct dl_args {
    const char *src,
               *dst;
    int req,
        idx,    // thread index!
        connid,
        tmpid,
        status, // thread status
        g_idx,
        ret;  // global item index in icon panel
    double      progress;
    uint64_t    contentLength; 
    void *unused; // for cross compat with _v2
    bool is_threaded;
} dl_args;

int statusCode   = 0xFF;

int  contentLengthType;
uint64_t contentLength;

static int update_progress(void *bar, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
{


    int progress = 0;
    //only show progress every 10mbs to avoid spamming the log
    if (dltotal > 0 ) {
        // get current file pos
        progress = (double)(((float)dlnow / dltotal) * 100.f);
    }
    ProgSetMessagewText(progress, "Downloading The Homebrew Store");

    return 0;
}

long curlDownloadFile(const char * url,
  const char * file) {
  CURL * curl = NULL;

  FILE * imageFD = fopen(file, "wb");
  if (!imageFD) {
    log_for_api("Failed to open file: %s | %s", file, strerror(errno));
    return -999;
  }

  progstart((char * )
    "starting download");

  CURLcode res = (CURLcode) - 1;
  log_for_api("Downloading: %s | %p | %p", url, curl_easy_init, curl_easy_strerror);
  curl = curl_easy_init();
  log_for_api("curl_easy_init: %p", curl);
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set user agent string
    std::string ua = fmt::format("Store API - FW: {0:x}", ps4_fw_version());
    log_for_api("[HTTP] User Agent set to %s", ua.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ua.c_str());
    // not sure how to use this when enabled
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    // not sure how to use this when enabled
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    // Set SSL VERSION to TLS 1.2
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    // Set timeout for the connection to build
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    // Follow redirects (?)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    // The function that will be used to write the data 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    // The data filedescriptor which will be written to
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, imageFD);
    // maximum number of redirects allowed
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20);
    // request using SSL for the FTP transfer if available
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

    /* pass the struct pointer into the xferinfo function */
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, update_progress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      log_for_api("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    } 
    else {
      long httpresponsecode = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, & httpresponsecode);
      // close filedescriptor
      fclose(imageFD), imageFD = NULL;
      // cleanup
      curl_easy_cleanup(curl);
      log_for_api("httpresponsecode: %lu", httpresponsecode);
      sceMsgDialogTerminate();
      return httpresponsecode;
    }

  }
  else {
    log_for_api("curl_easy_init() failed: %s", curl_easy_strerror(res));
  }

  // close filedescriptor
  fclose(imageFD);
  // cleanup
  curl_easy_cleanup(curl);
  unlink(file);
  sceMsgDialogTerminate();

  return res;
}

// the _v2 version is used in download panel
int dl_from_url(const char *url_, const char *dst_)
{
    long code = curlDownloadFile(url_, dst_);
    if ((int)code != 200){
        log_for_api("curlDownloadFile failed: %d", code);
        return (int)code;
    }
    else
        log_for_api("curlDownloadFile success");

    return 0;
}
#define BUFFER_SIZE (0x10000) /* 256kB */

struct write_result {
char *data;
int pos;
};

static size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {

struct write_result *result = (struct write_result *)stream;

/* Will we overflow on this write? */
if(result->pos + size * nmemb >= BUFFER_SIZE - 1) {
   log_for_api("curl error: too small buffer\n");
   return 0;
}

/* Copy curl's stream buffer into our own buffer */
memcpy(result->data + result->pos, ptr, size * nmemb);

/* Advance the position */
result->pos += size * nmemb;

return size * nmemb;

}

std::string check_from_url(std::string &url_)
{
    char  RES_STR[255];
    char  JSON[BUFFER_SIZE];
    jsmn_parser p;
    int r = -1;
    jsmntok_t t[128]; /* We expect no more than 128 tokens */
    CURL* curl;
    CURLcode res;
    long http_code = 0;
    
    memset(JSON, 0, sizeof(JSON));

    std::string ua = fmt::format("Store API version {0:#X} - FW: {1:#X}", API_VERSION, ps4_fw_version());
    curl = curl_easy_init();
    if (curl)
    {
        
       struct write_result write_results = {
        .data = &JSON[0],
        .pos = 0
       };
       curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
       curl_easy_setopt(curl, CURLOPT_USERAGENT, ua.c_str());
       curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
       curl_easy_setopt(curl, CURLOPT_NOPROGRESS,   1l);
       curl_easy_setopt(curl, CURLOPT_NOBODY, 0l);
       curl_easy_setopt(curl, CURLOPT_HEADER, 0L); 
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
       curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_results);

       res = curl_easy_perform(curl);
       if(!res) {
          log_for_api("RESPONSE: %s", JSON);
       }
       else{
        log_for_api( "[StoreCore][HTTP] curl_easy_perform() failed with code %i from Function %s url: %s : error: %s", res, "ini_dl_req", url_.c_str(), curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return std::string();
       }
    
       curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
       if(http_code != 200){
          curl_easy_cleanup(curl);
          return std::string();
       }
       curl_easy_cleanup(curl);
    }
    else{
        log_for_api( "[StoreCore][HTTP] curl_easy_perform() failed Function %s url: %s", "ini_dl_req", url_.c_str());
        curl_easy_cleanup(curl);
        return std::string();
    }

    jsmn_init(&p);
    r = jsmn_parse(&p, JSON, strlen(JSON), t, sizeof(t) / sizeof(t[0]));
    if (r  < 0) {
        log_for_api("could not parse Update json");
        return std::string();
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        log_for_api("JSON Update Object not found");
        return std::string();
    }

    for (int i = 1; i < r; i++) {
        if (jsoneq(JSON, &t[i], "hash") == 0) {
            snprintf(RES_STR, 255, "%.*s", t[i + 1].end - t[i + 1].start, JSON + t[i + 1].start);
            return std::string(RES_STR);
        }
        else
            log_for_api("Update format not found");
    }//
    return std::string();
}
