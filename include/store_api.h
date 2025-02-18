#include <stdint.h>

typedef enum{
    UPDATE_FOUND,
    UPDATE_ERROR,
    NO_UPDATE,
    APP_NOT_INSTALLED,  //
} update_ret;

#define API_VERSION 0x1004

#ifdef __cplusplus 
extern "C" {
#endif
bool sceStoreApiLaunchStore(const char* query);
update_ret sceStoreApiCheckUpdate(const char* tid);
#if defined(__cplusplus)
}  
#endif
