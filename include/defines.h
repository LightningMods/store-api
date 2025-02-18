#pragma once

#define ITEMZ_CUSTOM_SFO "/user/app/ITEM00001/sfos"

#define SCE_SYSMODULE_INTERNAL_SYS_CORE 0x80000004
#define SCE_SYSMODULE_INTERNAL_NETCTL 0x80000009
#define SCE_SYSMODULE_INTERNAL_HTTP 0x8000000A
#define SCE_SYSMODULE_INTERNAL_SSL 0x8000000B
#define SCE_SYSMODULE_INTERNAL_AUDIOOUT 0x80000001
#define SCE_SYSMODULE_INTERNAL_AUDIOIN 0x80000002
#define SCE_SYSMODULE_INTERNAL_NP_COMMON 0x8000000C
#define SCE_SYSMODULE_INTERNAL_SYSTEM_SERVICE 0x80000010
#define SCE_SYSMODULE_INTERNAL_USER_SERVICE 0x80000011
#define SCE_SYSMODULE_INTERNAL_APPINSTUTIL 0x80000014
#define SCE_SYSMODULE_INTERNAL_NET 0x8000001C
#define SCE_SYSMODULE_INTERNAL_IPMI 0x8000001D
#define SCE_SYSMODULE_INTERNAL_VIDEO_OUT 0x80000022
#define SCE_SYSMODULE_INTERNAL_BGFT 0x8000002A
#define SCE_SYSMODULE_INTERNAL_PRECOMPILED_SHADERS 0x80000064

#define SCE_LNC_ERROR_APP_NOT_FOUND 0x80940031 // Usually happens if you to launch an app not in app.db
#define SCE_LNC_UTIL_ERROR_ALREADY_INITIALIZED 0x80940018
#define SCE_LNC_UTIL_ERROR_ALREADY_RUNNING 0x8094000c
#define SCE_LNC_UTIL_ERROR_ALREADY_RUNNING_KILL_NEEDED 0x80940010
#define SCE_LNC_UTIL_ERROR_ALREADY_RUNNING_SUSPEND_NEEDED 0x80940011
#define SCE_LNC_UTIL_ERROR_APP_ALREADY_RESUMED 0x8094001e
#define SCE_LNC_UTIL_ERROR_APP_ALREADY_SUSPENDED 0x8094001d
#define SCE_LNC_UTIL_ERROR_APP_NOT_IN_BACKGROUND 0x80940015
#define SCE_LNC_UTIL_ERROR_APPHOME_EBOOTBIN_NOT_FOUND 0x80940008
#define SCE_LNC_UTIL_ERROR_APPHOME_PARAMSFO_NOT_FOUND 0x80940009
#define SCE_LNC_UTIL_ERROR_CANNOT_RESUME_INITIAL_USER_NEEDED 0x80940012
#define SCE_LNC_UTIL_ERROR_DEVKIT_EXPIRED 0x8094000b
#define SCE_LNC_UTIL_ERROR_IN_LOGOUT_PROCESSING 0x8094001a
#define SCE_LNC_UTIL_ERROR_IN_SPECIAL_RESUME 0x8094001b
#define SCE_LNC_UTIL_ERROR_INVALID_PARAM 0x80940005
#define SCE_LNC_UTIL_ERROR_INVALID_STATE 0x80940019
#define SCE_LNC_UTIL_ERROR_INVALID_TITLE_ID 0x8094001c
#define SCE_LNC_UTIL_ERROR_LAUNCH_DISABLED_BY_MEMORY_MODE 0x8094000d
#define SCE_LNC_UTIL_ERROR_NO_APP_INFO 0x80940004
#define SCE_LNC_UTIL_ERROR_NO_LOGIN_USER 0x8094000a
#define SCE_LNC_UTIL_ERROR_NO_SESSION_MEMORY 0x80940002
#define SCE_LNC_UTIL_ERROR_NO_SFOKEY_IN_APP_INFO 0x80940014
#define SCE_LNC_UTIL_ERROR_NO_SHELL_UI 0x8094000e
#define SCE_LNC_UTIL_ERROR_NOT_ALLOWED 0x8094000f
#define SCE_LNC_UTIL_ERROR_NOT_INITIALIZED 0x80940001
#define SCE_LNC_UTIL_ERROR_OPTICAL_DISC_DRIVE 0x80940013
#define SCE_LNC_UTIL_ERROR_SETUP_FS_SANDBOX 0x80940006
#define SCE_LNC_UTIL_ERROR_SUSPEND_BLOCK_TIMEOUT 0x80940017
#define SCE_LNC_UTIL_ERROR_VIDEOOUT_NOT_SUPPORTED 0x80940016
#define SCE_LNC_UTIL_ERROR_WAITING_READY_FOR_SUSPEND_TIMEOUT 0x80940021
#define SCE_APP_INSTALLER_ERROR_DISC_NOT_INSERTED 0x80A30005
#define SCE_SYSCORE_ERROR_LNC_INVALID_STATE 0x80aa000a
#define SCE_LNC_UTIL_ERROR_NOT_INITIALIZED 0x80940001
#define ORBIS_KERNEL_EAGAIN 0x80020023
#define SFO_PATCH_FLAG_REMOVE_COPY_PROTECTION (1 << 0)
#define SFO_ACCOUNT_ID_SIZE 8
#define SFO_PSID_SIZE 16
#define SFO_DIRECTORY_SIZE 32

#define asset_path(x) "/mnt/sandbox/pfsmnt/" ITEMZFLOW_TID "-app0/assets/" x
#define APP_PATH(x) "/user/app/ITEM00001/" x
