#pragma once



#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include "store_api.h"
#include "dialog.h"

typedef uint32_t uid_t;
typedef uint32_t gid_t;

struct jbc_cred
{
    uid_t uid;
    uid_t ruid;
    uid_t svuid;
    gid_t rgid;
    gid_t svgid;
    uintptr_t prison;
    uintptr_t cdir;
    uintptr_t rdir;
    uintptr_t jdir;
    uint64_t sceProcType;
    uint64_t sonyCred;
    uint64_t sceProcCap;
};

#if defined(__cplusplus)
extern "C" {
#endif
#define DIM(x)  (sizeof(x)/sizeof(*(x)))
#define KB(x)   ((size_t) (x) << 10)
#define MB(x)   ((size_t) (x) << 20)
#define GB(x)   ((size_t) (x) << 30)
#define B2GB(x)   ((size_t) (x) >> 30)
#define B2MB(x)   ((size_t) (x) >> 20)


#define ORBIS_SYSMODULE_MESSAGE_DIALOG 0x00A4 // libSceMsgDialog.sprx
#define STORE_TID "NPXS39041"

//#define assert(expr) if (!(expr)) msgok(FATAL, "Assertion Failed!");

#define YES 1
#define NO  2
int Confirmation_Msg(std::string msgr);
enum Flag
{
    Flag_None = 0,
    SkipLaunchCheck = 1,
    SkipResumeCheck = 1,
    SkipSystemUpdateCheck = 2,
    RebootPatchInstall = 4,
    VRMode = 8,
    NonVRMode = 16
};

typedef struct _LncAppParam
{
    uint32_t sz;
    uint32_t user_id;
    uint32_t app_opt;
    uint64_t crash_report;
    enum Flag check_flag;
}
LncAppParam;


#define BETA 0
//#define LOCALHOST_WINDOWS 1

#define SCE_SYSMODULE_INTERNAL_COMMON_DIALOG 0x80000018
#define SCE_SYSMODULE_INTERNAL_SYSUTIL 0x80000018

// sysctl
uint32_t ps4_fw_version(void);
bool copy_dir(const char* sourcedir, const char* destdir);
extern bool is_connected_app;
bool MD5_hash_compare(const char* file1, const char* hash);
bool copyFile(std::string source, std::string dest, bool show_progress);
int sceLncUtilGetAppId(const char* TID);
int sceLncUtilGetAppTitleId(int appid, char* tid_out);
int sceAppInstUtilAppGetInsertedDiscTitleId(const char* tid);
int scePadGetHandle(int uid, int idc, int idc_1);
int sceAppInstUtilTerminate();
bool IS_ERROR(uint32_t ret);
bool MD5_file_compare(const char* file1, const char* file2);
update_ret check_update_from_url(const char* url);
bool if_exists(const char* path);
int sceAppInstUtilAppExists(const char* tid, int* flag);
uintptr_t jbc_get_prison0(void);
uintptr_t jbc_get_rootvnode(void);
int jbc_get_cred(struct jbc_cred*);
int jbc_jailbreak_cred(struct jbc_cred*);
int jbc_set_cred(const struct jbc_cred*);
bool is_jailbroken();
///int fuse_test();
#if defined(__cplusplus)
}
#endif

std::string check_from_url(std::string &url_);
int dl_from_url(const char *url_, const char *dst_);
bool Launch_Store_URI();
void log_for_api(const char* format, ...);
bool Launch_Store_URI(const char* query);
bool is_mira_available(void);
void mira_get_perms(void);
update_ret check_update_from_url(const char* tid);
void ProgSetMessagewText(int prog, const char* fmt, ...);
int progstart(char* format, ...);
void loadmsg(std::string in);
