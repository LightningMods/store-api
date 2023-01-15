#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include "utils.h"
#include <orbis/Sysmodule.h>
#include <orbis/libkernel.h>
#include "fmt/format.h"

#define API_VERSION 0x1002

extern "C" {
  /* SCE PREFIX WRAPPER TO MOCK THEM */
  // query can either be the app title id or app name
  struct jbc_cred cred, root_cred;
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
  
// LAPY UNIVERSAL WRAPPERS  
int FreeUnjail(int q) {

	log_for_api("FreeUnjail(\"%i\")", q);
  log_for_api("Nobody needs the fw version anymore its not 2018 anymore");
  if (!is_jailbroken()) {
      if (is_mira_available()) {
        log_for_api("Mira is available, using it");
        mira_get_perms();

      } 
	  else {
        log_for_api("Mira is not available, using libjbc");
        jbc_get_cred(&cred);
        root_cred = cred;
        jbc_jailbreak_cred(&root_cred);
        root_cred.cdir = cred.rdir;
        jbc_set_cred(&root_cred);
        
      }
    } 

	return 0;
}

int FreeMount(){
	log_for_api("FreeMount");
  if (!!mountfs("/dev/da0x4.crypt", "/system", "exfatfs", "511", MNT_UPDATE) ||
  !!mountfs("/dev/da0x5.crypt", "/system_ex", "exfatfs", "511", MNT_UPDATE)){
      log_for_api("mounting /system failed with %s.", strerror(errno));
      return -1;
  }
  else
      log_for_api("mounting /system succeeded.");

  log_for_api("returning...");

	return 0;
}

// This is mostly for C# / Unity to load C pre-reqs
void Load_Store_API_Preqs_BigApp() {
  log_for_api("Load_Store_API_Preqs()");
  sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG);
  log_for_api("Initiating Native Dialogs ... ");
  sceCommonDialogInitialize();
  sceMsgDialogInitialize();
  log_for_api("Native Dialogs Initiated");
  sceKernelLoadStartModule("/system/common/lib/libSceAppInstUtil.sprx", 0, NULL, 0, NULL, NULL);
  sceKernelLoadStartModule("/system/common/lib/libSceBgft.sprx", 0, NULL, 0, NULL, NULL);
}
}