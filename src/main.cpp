#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include "utils.h"
#include "defines.h"

bool fetch_json(const char* url, char *json, const char* ua)
extern "C" {
  /* SCE PREFIX WRAPPER TO MOCK THEM */
  // query can either be the app title id or app name
  struct jbc_cred cred, root_cred;
  bool sceStoreApiFetchJson(const char* ua, const char* url, char *json){
       return fetch_json(url, json, ua);
  }
  bool sceStoreApiLaunchStore(const char * query) {
    bool ret = false;
    log_for_api("sceStoreApiLaunchStore(\"%s\") | API version: 0x%x", query, API_VERSION);

    if (!is_jailbroken()) {

      if (is_mira_available()) {
        log_for_api("Mira is available, using it");
        mira_get_perms();
        ret = Launch_Store_URI(query);
      } 
	  else {

        log_for_api("Mira is not available, using libjbc");
        jbc_get_cred(&cred);
        root_cred = cred;
        jbc_jailbreak_cred( & root_cred);
        root_cred.cdir = cred.rdir;
        jbc_set_cred(&root_cred);
        ret = Launch_Store_URI(query);
        jbc_set_cred(&cred);
      }
    } 
	else {
      log_for_api("Already jailbroken, running func");
      ret = Launch_Store_URI(query);
    }

    return ret;
  }

  update_ret sceStoreApiCheckUpdate(const char * tid) {

    update_ret ret = NO_UPDATE;

    log_for_api("sceStoreApiCheckUpdate(\"%s\") | API version: 0x%x", tid, API_VERSION);
    if (!is_jailbroken()) {

      if (is_mira_available()) {

        log_for_api("Mira is available, using it");
        mira_get_perms();
        ret = check_update_from_url(tid);

      } 
	  else {
        log_for_api("Mira is not available, using libjbc");
        jbc_get_cred(&cred);
        root_cred = cred;
        jbc_jailbreak_cred(&root_cred);
        root_cred.cdir = cred.rdir;
        jbc_set_cred(&root_cred);
        ret = check_update_from_url(tid);
        jbc_set_cred(&cred);
      }
    } 
	else {
      log_for_api("Already jailbroken, running func");
      ret = check_update_from_url(tid);
    }

    return ret;
  }

}
