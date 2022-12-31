#include <stdint.h>

enum update_ret{
    UPDATE_FOUND,
    UPDATE_ERROR,
    NO_UPDATE,
};
#ifdef __cplusplus 
extern "C" {
#endif
bool sceStoreApiLaunchStore(const char* query);
enum update_ret sceStoreApiCheckUpdate(const char* tid);
#if defined(__cplusplus)
}  
#endif
