#include <stdio.h>
#include <stdlib.h>
#include <md5.h>
#include <errno.h>
#include <sys/time.h>
#include <orbis/Sysmodule.h>
#include <stdlib.h>
#include <pthread.h>
#include "utils.h"
#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <unistd.h>
#include <dialog.h>
#include <sstream>
#include "installpkg.h"
#include <sys/param.h>
#include <sys/mount.h>
#include <signal.h>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include <orbis/libkernel.h>
#include "fmt/format.h"
#include "fmt/printf.h"
#include "fmt/chrono.h"

/*================== MISC GOLBAL VAR ============*/
uint32_t sdkVersion = -1;
bool sceAppInst_done = false;

extern "C" {
int sceSystemServiceKillApp(uint32_t appid, int opt,int method, int reason);
int sceKernelOpenEventFlag(void* event, const char* name);
int sceKernelCloseEventFlag(void* event);
int sceUserServiceGetForegroundUser(uint32_t *userid);
int32_t sceSystemServiceHideSplashScreen();
int32_t sceSystemServiceParamGetInt(int32_t paramId, int32_t *value);
int sceSystemServiceGetAppIdOfMiniApp();
int sceSystemServiceGetAppIdOfBigApp();
}

bool IS_ERROR(uint32_t a1)
{
    return a1 & 0x80000000;
}

bool if_exists(const char* path)
{
    struct stat   buffer;   
    return (stat(path, &buffer) == 0);
}

void log_for_api(const char* format, ...)
{
    // Check the length of the format string.
    // If it is too long, return immediately.
    if (strlen(format) > 1000) {
        return;
    }

    // Initialize a buffer to hold the formatted log message.
    char buff[1024];

    // Initialize a va_list to hold the variable arguments passed to the function.
    va_list args;
    va_start(args, format);

    // Format the log message using the format string and the variable arguments.
    vsprintf(buff, format, args);

    // Clean up the va_list.
    va_end(args);

    // Append a newline character to the log message.
    strcat(buff, "\n");

    // Output the log message using sceKernelDebugOutText.
    sceKernelDebugOutText(0, buff);

    // Open the log file for writing.
    int fd = sceKernelOpen("/data/store_api.log", O_WRONLY | O_CREAT | O_APPEND, 0777);

    // If the file was successfully opened, write the log message to the file
    // and then close the file.
    if (fd >= 0) {
        sceKernelWrite(fd, buff, strlen(buff));
        sceKernelClose(fd);
    }
}


bool copyFile(std::string source, std::string dest)
{
    std::string buf;
    int src = sceKernelOpen(source.c_str(), 0x0000, 0);
    if (src > 0)
    {
        
        int out = sceKernelOpen(dest.c_str(), 0x0001 | 0x0200 | 0x0400, 0777);
        if (out > 0)
        {   

            size_t bytes = 0;
            std::vector<char> buffer(MB(10));
            if (buffer.size() > 0)
            {
                while (0 < (bytes = sceKernelRead(src, &buffer[0], MB(10)))){
                    sceKernelWrite(out, &buffer[0], bytes);
                }
                buffer.clear();
            }
        }
        else
        {
            fmt::print("Could copy file: {} Reason: {x}", src, out);
            sceKernelClose(src);
            return false;
        }

        sceKernelClose(src);
        sceKernelClose(out);
        sceMsgDialogTerminate();
        return true;
    }
    else
    {
        fmt::print("Could copy file: {} Reason: {x}", dest, src);
        return false;
    }
}

bool MD5_hash_compare(const char* filename, const char* hash)
{
   std::stringstream ss;
   std::streampos fsize = 0;
   std::ifstream file(filename, std::ios::binary);
   if (!file.is_open() || file.bad()) {
	log_for_api("Could not open file %s", filename);
	return false;
    }
    MD5_CTX ctx;
    MD5_Init(&ctx);
    char buf[32768];

    while (file.read(buf, sizeof(buf)))
	   MD5_Update(&ctx, buf, sizeof(buf));

    fsize = file.tellg();
    file.seekg( 0, std::ios::end );
    fsize = file.tellg() - fsize;
	
    MD5_Update(&ctx, buf, fsize);
    unsigned char result[16];
    MD5_Final(result, &ctx);
    file.close();

     for (int i = 0; i < 16; i++)
         ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
	
     log_for_api("[MD5] File hash: %s vs hash: %s", ss.str().c_str(), hash);

     return (ss.str() != hash);
}

bool is_apputil_init()
{
    int ret = sceAppInstUtilInitialize();
    if (ret) {
        log_for_api( " sceAppInstUtilInitialize failed: 0x%08X", ret);
        return false;
    }
    else
        sceAppInst_done = true;

    return true;
}
extern "C" int	sysctlbyname(const char *, void *, size_t *, const void *, size_t);

uint32_t ps4_fw_version(void)
{
    //cache the FW Version
    if (0 < sdkVersion) {
        size_t   len = 4;
        // sysKernelGetLowerLimitUpdVersion == machdep.lower_limit_upd_version
        // rewrite of sysKernelGetLowerLimitUpdVersion
        sysctlbyname("machdep.lower_limit_upd_version", &sdkVersion, &len, NULL, 0);
    }

    // FW Returned is in HEX
    return sdkVersion;
}

int Confirmation_Msg(std::string msg)
{

    sceMsgDialogTerminate();
    //ds

    sceMsgDialogInitialize();
    OrbisMsgDialogParam param;
    OrbisMsgDialogParamInitialize(&param);
    param.mode = ORBIS_MSG_DIALOG_MODE_USER_MSG;

    OrbisMsgDialogUserMessageParam userMsgParam;
    memset(&userMsgParam, 0, sizeof(userMsgParam));
    userMsgParam.msg = msg.c_str();
    userMsgParam.buttonType = ORBIS_MSG_DIALOG_BUTTON_TYPE_YESNO;
    param.userMsgParam = &userMsgParam;
    // cv   
    if (0 < sceMsgDialogOpen(&param))
        return NO;

    OrbisCommonDialogStatus stat;
    //
    while (1) {
        stat = sceMsgDialogUpdateStatus();
        if (stat == ORBIS_COMMON_DIALOG_STATUS_FINISHED) {

            OrbisMsgDialogResult result;
            memset(&result, 0, sizeof(result));

            sceMsgDialogGetResult(&result);

            return result.buttonId;
        }
    }
    //c
    return NO;
}
void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
) {
    // Silently ignore silly per-line values.

    if (perLine < 4 || perLine > 64) perLine = 16;

    int i;
    unsigned char buff[perLine+1];
    const unsigned char * pc = (const unsigned char *)addr;

    // Output description if given.

    if (desc != NULL) printf ("%s:\n", desc);

    // Length checks.

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.

    for (i = 0; i < len; i++) {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.

            if (i != 0) printf ("  %s\n", buff);

            // Output the offset of current line.

            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf (" %02x", pc[i]);

        // And buffer a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.

    while ((i % perLine) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII buffer.

    printf ("  %s\n", buff);
}

void* find_sceLncUtilLaunchApp(void* addr){

    for(int i = 0x40; i < 0x50; i++){
        void *addrp = (void*)((uintptr_t)addr + i);
        const unsigned char * pc = (const unsigned char *)addrp;
  
        hexDump("Searching for sceLncUtilLaunchApp",(void*)((uintptr_t)addrp + 2), 0x3, 0x3);
        if (pc[0] == 0xE8 && /*Skip first byte*/ pc[2] == 0x10 && pc[3] == 0x00 && pc[4] == 0x00){
            return addrp;
        }
        else
            log_for_api("opcodes(0x%x) %x %x %x", i, pc[0], pc[2], pc[3]);
    }

    return NULL;
}
uint32_t (*sceLncUtilLaunchApp)(const char* tid, const char* argv[], LncAppParam* param);
bool Launch_Store_URI(const char* query)
{
    int ret = -1;
    uint32_t userId = 0;
    std::string uri = query;
    if(!if_exists("/user/app/NPXS39041/app.pkg")){
        log_for_api("[STORE_URI] Store is NOT INSTALLED!!! ");

        if(Confirmation_Msg("The Store isn't installed, would you like to install it now?") == YES &&
         (ret = dl_from_url("https://pkg-zone.com/Store-R2.pkg", "/user/app/store_download.pkg")) == 0){

            log_for_api("[STORE_URI] Store is downloaded, installing.... ");
            if((ret = pkginstall("/user/app/store_download.pkg", "Homebrew Store")) == 0){
                log_for_api("[STORE_URI] Store is installed, going on.... ");
            }
            else{
                log_for_api("[STORE_URI] Store is NOT installed, err: %i, Returning....", ret );
                return false;
            }
        }
        else{
            log_for_api("[STORE_URI] Store is NOT downloaded, err: %i, Returning....", ret);
            return false;
        }
    }

    int libcmi = sceKernelLoadStartModule("/system/common/lib/libSceSystemService.sprx", 0, NULL, 0, 0, 0);
    if (libcmi > 0)
    {
        sceUserServiceGetForegroundUser(&userId);
        log_for_api("[STORE_URI] Foreground UserId: %d", userId);

        LncAppParam param;
        param.sz = sizeof(LncAppParam);
        param.user_id = userId;
        param.app_opt = 0;
        param.crash_report = 0;
        param.check_flag = SkipSystemUpdateCheck;

         const char* argv[] =
         {
           uri.c_str(),
           NULL,
        };

        int StoreID = sceSystemServiceGetAppIdOfMiniApp();
        if ((StoreID & ~0xFFFFFF) == 0x60000000 && if_exists("/mnt/sandbox/pfsmnt/NPXS39041-app0/")) {
            log_for_api("[STORE_URI] Store already opened!! Closing,...");
            sceSystemServiceKillApp(StoreID, -1, 0, 0);
            log_for_api("[STORE_URI] Store Closed");
        }
        
        log_for_api("[STORE_URI] Opening Store...");
        // Launch Store with ARG Search
        sceKernelDlsym(libcmi, "sceSystemServiceLaunchApp", (void**)&sceLncUtilLaunchApp);
        if(sceLncUtilLaunchApp == NULL){
            log_for_api("[STORE_URI] sceLncUtilLaunchApp is NULL");
            return false;
        }
       
        if((sceLncUtilLaunchApp = (uint32_t (*)(const char*, const char**, LncAppParam*))
        find_sceLncUtilLaunchApp((void*)sceLncUtilLaunchApp)) == NULL){
            log_for_api("[STORE_URI] sceLncUtilLaunchApp NOT found");
            return false;
        }

        log_for_api("[STORE_URI] sceLncUtilLaunchApp found at %p", sceLncUtilLaunchApp);

        uint32_t sys_res = sceLncUtilLaunchApp(STORE_TID, argv, &param);
        if (IS_ERROR(sys_res)){
            log_for_api("[STORE_URI] sceSystemServiceLaunchApp failed: 0x%08X", sys_res);
            return false;
        }
        else
            log_for_api("[STORE_URI] sceSystemServiceLaunchApp succeeded");

    }

    return true;

}
static bool app_inst_util_is_exists(const char* title_id, bool* exists) {
    int flag;

    if (!title_id) return false;

    if (!sceAppInst_done) {
        log_for_api("Starting app_inst_util_init..");
        if (!app_inst_util_init()) {
            log_for_api("app_inst_util_init has failed...");
            return false;
        }
    }

    int ret = sceAppInstUtilAppExists(title_id, &flag);
    if (ret) {
        log_for_api("sceAppInstUtilAppExists failed: 0x%08X\n", ret);
        return false;
    }

    if (exists) *exists = flag;

    return true;
}

update_ret check_update_from_url(const char* tid)
{
    //ITEM00001
    bool app_exists = false;
    std::string http_req = fmt::format("https://api.pkg-zone.com/hashByTitleID/{}", tid);
    std::string dst_path = fmt::format("/user/app/{}/app.pkg", tid);
    log_for_api("[STORE_URI] Checking for updates from: %s", http_req.c_str());
    app_inst_util_is_exists(tid, &app_exists);
    log_for_api("[STORE_URI] Does %s exists? %s", tid, app_exists ? "Yes" : "No");
    std::string result = check_from_url(http_req);
    if (!result.empty() && app_exists) {
        bool ret = MD5_hash_compare(dst_path.c_str(), result.c_str());
        log_for_api("Update Status: %s", ret ? "UPDATE_REQUIRED": "NO_UPDATE");
        return (ret) ? UPDATE_FOUND : NO_UPDATE;  
    }
    else
        log_for_api("Failed to get response from Update Server");
     

    return UPDATE_ERROR;
}

bool is_mira_available(void)
{
    return false;
}

void mira_get_perms(void)
{
    // Not Implemented
}


bool is_jailbroken() {
   int fd = open("/user/.jb_check", O_WRONLY | O_CREAT | O_TRUNC, 0777);
   if (fd > 0) {
      unlink("/user/.jb_check");
      close(fd);
      return true; 
   }
   else
      return false;
}

void ProgSetMessagewText(int prog, const char* fmt, ...)
{

        char buff[300];
        memset(&buff[0], 0, sizeof(buff));

        va_list args;
        va_start(args, fmt);
        vsnprintf(&buff[0], 299, fmt, args);
        va_end(args);

        sceMsgDialogProgressBarSetValue(0, prog);
        sceMsgDialogProgressBarSetMsg(0, buff);
}


int progstart(char* format, ...)
{

    
    int ret = 0;

    char buff[1024];

    memset(buff, 0, 1024);

    va_list args;
    va_start(args, format);
    vsprintf(&buff[0], format, args);
    va_end(args);

    sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG);
    sceMsgDialogInitialize();

    OrbisMsgDialogParam dialogParam;
    OrbisMsgDialogParamInitialize(&dialogParam);
    dialogParam.mode = ORBIS_MSG_DIALOG_MODE_PROGRESS_BAR;

    OrbisMsgDialogProgressBarParam  progBarParam;
    memset(&progBarParam, 0, sizeof(OrbisMsgDialogProgressBarParam));

    dialogParam.progBarParam = &progBarParam;
    dialogParam.progBarParam->barType = ORBIS_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;
    dialogParam.progBarParam->msg = &buff[0];

    sceMsgDialogOpen(&dialogParam);

    return ret;
}

void loadmsg(std::string in)
{

    OrbisMsgDialogButtonsParam buttonsParam;
    OrbisMsgDialogUserMessageParam messageParam;
    OrbisMsgDialogParam dialogParam;

    sceMsgDialogInitialize();
    OrbisMsgDialogParamInitialize(&dialogParam);

    memset(&buttonsParam, 0x00, sizeof(buttonsParam));
    memset(&messageParam, 0x00, sizeof(messageParam));

    dialogParam.userMsgParam = &messageParam;
    dialogParam.mode = ORBIS_MSG_DIALOG_MODE_USER_MSG;

    messageParam.buttonType = ORBIS_MSG_DIALOG_BUTTON_TYPE_WAIT;
    messageParam.msg        = in.c_str();

    sceMsgDialogOpen(&dialogParam);

    return;
}
