# store-api
Homebrew Store API library that any PS4 Homebrew app can implement (works with sandboxed apps too)

[app_latest]: https://github.com/LightningMods/store-api/releases/latest
[app_license]: https://github.com/LightningMods/store-api/blob/master/LICENSE
[img_downloads]: https://img.shields.io/github/downloads/LightningMods/store-api/total.svg?maxAge=3600
[app_downloads]: https://github.com/LightningMods/store-api/releases
[app_latest]: https://github.com/LightningMods/store-api/releases/latest
[app_license]: https://github.com/LightningMods/store-api/blob/master/LICENSE
[img_downloads]: https://img.shields.io/github/downloads/LightningMods/store-api/total.svg?maxAge=3600
[img_latest]: https://img.shields.io/github/release/LightningMods/store-api.svg?maxAge=3600
[img_license]: https://img.shields.io/github/license/LightningMods/store-api.svg?maxAge=2592000

[![License][img_license]][app_license]
[![Build Library](https://github.com/LightningMods/store-api/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/LightningMods/store-api/actions/workflows/c-cpp.yml)

### Building
**FIRST** Download the Linux [create_fself with multi-lib support](https://github.com/LightningMods/OpenOrbis-PS4-Toolchain/releases/download/v0.50001/create-fself)
OR compile it yourself using [This multi-lib pull request](https://github.com/OpenOrbis/create-fself/pull/5)

**Alternatively** you can Download [the HB Store's OOSDK](https://github.com/LightningMods/OpenOrbis-PS4-Toolchain/releases/download/v0.50001/)

Run `make` to create a static library and a prx
Run `make install` to install the static library and store_api.h header

### Developer Instructions
the following PS4 system libs are used by this library `-lSceBgft -lSceAppInstUtil`
`make install` will install the store_api.h header to the SDK so you can include it in your 
project by doing `<store_api.h>` and include the static lib `-lstore_api` you can either use the static (.a) or dynamic (.prx) libs

#### System/mini apps
The following needs to be added to your app failure to do so can result in crashing
```c
#define SCE_SYSMODULE_INTERNAL_NET 0x8000001C
#define SCE_SYSMODULE_INTERNAL_BGFT 0x8000002A
#define SCE_SYSMODULE_INTERNAL_NETCTL 0x80000009
#define SCE_SYSMODULE_INTERNAL_COMMON_DIALOG 0x80000018
#define SCE_SYSMODULE_INTERNAL_APPINSTUTIL 0x80000014
#define ORBIS_SYSMODULE_MESSAGE_DIALOG 0x00A4 // libSceMsgDialog.sprx


sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_BGFT);
sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_NET);
sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_NETCTL);
sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_COMMON_DIALOG);
sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_APPINSTUTIL);
sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG);
// call for the Store api message dialogs
sceCommonDialogInitialize();
    
```
#### Big apps
Works in sandboxed games as well

```c
#define ORBIS_SYSMODULE_MESSAGE_DIALOG 0x00A4 // libSceMsgDialog.sprx

sceSysmoduleLoadModule(ORBIS_SYSMODULE_MESSAGE_DIALOG);
// call for the Store api message dialogs
sceCommonDialogInitialize();
```

#### Errors and reporting
- every API error is logged to `/data/store_api.log` and UART
- Join this [Discord server](https://discord.gg/fcUg6yP5Gy) for API Questions or issues

### Example code

```c
// include our API header with enum
#include <store_api.h>

int main(){
   // check for an update using a title id
   if(sceStoreApiCheckUpdate("LAPY20001") == UPDATE_FOUND){
       // update found now launch store with either the title id or app name
       If(!sceStoreApiLaunchStore("LAPY20001")){
         // ERROR HANDLING HERE
         return -1;
        }
         
   }
   return 0;
}

```

### Required external libs
- [polarSSL](https://github.com/bucanero/oosdk_libraries/tree/master/polarssl-1.3.9) library
- [libcurl](https://github.com/bucanero/oosdk_libraries/tree/master/curl-7.64.1)
- [{fmt}](https://fmt.dev/latest/index.html)
- [libjbc](https://github.com/sleirsgoevy/ps4-libjbc)

