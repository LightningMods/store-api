#include "utils.h"
#include <stdarg.h>
#include <installpkg.h>
#include <stdbool.h>
#include <string>
#include "fmt/format.h"
#include "fmt/printf.h"
#include "fmt/chrono.h"
#include <unistd.h>

int PKG_ERROR(const char* name, int ret)
{
    log_for_api( "%s error: %x", name, ret);
    return ret;
}

/* we use bgft heap menagement as init/fini as flatz already shown at 
 * https://github.com/flatz/ps4_remote_pkg_installer/blob/master/installer.c
 */

#define BGFT_HEAP_SIZE (1 * 1024 * 1024)

extern bool sceAppInst_done;
static bool   s_bgft_initialized = false;
static struct bgft_init_params  s_bgft_init_params;

bool app_inst_util_init(void) {
    int ret;

    if (sceAppInst_done) {
        goto done;
    }

    ret = sceAppInstUtilInitialize();
    if (ret) {
        log_for_api( "sceAppInstUtilInitialize failed: 0x%08X", ret);
        goto err;
    }

    sceAppInst_done = true;

done:
    return true;

err:
    sceAppInst_done = false;

    return false;
}

void app_inst_util_fini(void) {
    int ret;

    if (!sceAppInst_done) {
        return;
    }

    ret = sceAppInstUtilTerminate();
    if (ret) {
        log_for_api( "sceAppInstUtilTerminate failed: 0x%08X", ret);
    }

    sceAppInst_done = false;
}

bool bgft_init(void) {
    int ret;

    if (s_bgft_initialized) {
        goto done;
    }

    memset(&s_bgft_init_params, 0, sizeof(s_bgft_init_params));
    {
        s_bgft_init_params.heapSize = BGFT_HEAP_SIZE;
        s_bgft_init_params.heap = (uint8_t*)malloc(s_bgft_init_params.heapSize);
        if (!s_bgft_init_params.heap) {
            log_for_api( "No memory for BGFT heap.");
            goto err;
        }
        memset(s_bgft_init_params.heap, 0, s_bgft_init_params.heapSize);
    }

    ret = sceBgftServiceInit(&s_bgft_init_params);
    if (ret) {
        log_for_api( "sceBgftInitialize failed: 0x%08X", ret);
        goto err_bgft_heap_free;
    }

    s_bgft_initialized = true;

done:
    return true;

err_bgft_heap_free:
    if (s_bgft_init_params.heap) {
        free(s_bgft_init_params.heap);
        s_bgft_init_params.heap = NULL;
    }

    memset(&s_bgft_init_params, 0, sizeof(s_bgft_init_params));

err:
    s_bgft_initialized = false;

    return false;
}

void bgft_fini(void) {
    int ret;

    if (!s_bgft_initialized) {
        return;
    }

    ret = sceBgftServiceTerm();
    if (ret) {
        log_for_api( "sceBgftServiceTerm failed: 0x%08X", ret);
    }

    if (s_bgft_init_params.heap) {
        free(s_bgft_init_params.heap);
        s_bgft_init_params.heap = NULL;
    }

    memset(&s_bgft_init_params, 0, sizeof(s_bgft_init_params));

    s_bgft_initialized = false;
}

bool app_inst_util_is_exists(const char* title_id, bool* exists) {
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

void *install_prog(void* argument)
{
    struct install_args* args = (install_args*)argument;
    SceBgftTaskProgress progress_info;
    log_for_api("Starting PKG Install");

    progstart((char*)"Homebrew Store");


    int prog = 0;
    while (prog < 99)
    {
        memset(&progress_info, 0, sizeof(progress_info));

        int ret = sceBgftServiceDownloadGetProgress(args->task_id, &progress_info);
        if (ret) 
           return (void *)(intptr_t)PKG_ERROR("sceBgftDownloadGetProgress", ret);
        
        if (progress_info.transferred > 0 && progress_info.error_result != 0) {
              return (void *)(intptr_t)PKG_ERROR("BGFT_ERROR", progress_info.error_result);
        }

        prog = (uint32_t)(((float)progress_info.transferred / progress_info.length) * 100.f);

        ProgSetMessagewText(prog, "Installing the Homebrew Store");

     if (progress_info.transferred % (4096 * 128) == 0){
         log_for_api("%s, Install_Thread: reading data, %lub / %lub (%%%i) ERR: %i", __FUNCTION__, progress_info.transferred, progress_info.length, prog, progress_info.error_result);
        // log_for_api("ERR: %llu %llu %llu %llu", progress_info.transferred, progress_info.transferredTotal, progress_info.length, progress_info.lengthTotal);
     }

 
    }

    if (progress_info.error_result == 0) {
      if(args->delete_pkg)
           unlink(args->path);
    }
    else {
        log_for_api("Installation of %s has failed with code 0x%x", args->title_id, progress_info.error_result);
    }

    log_for_api("Finalizing Memory...");
    sceMsgDialogTerminate();
    free(args->title_id);
    free(args->path);
    free(args->fname);
    free(args);
    bgft_fini();

    return NULL;
}

uint32_t pkginstall(const char *fullpath, const char* filename)
{
    char title_id[16];
    int  is_app, ret = -1;
    int  task_id = -1;

    if( if_exists(fullpath) )
    {
      if (sceAppInst_done) {
          log_for_api("Initializing AppInstUtil...");

          if (!app_inst_util_init())
              return PKG_ERROR("AppInstUtil", ret);
      }
        
        log_for_api("Initializing BGFT...");
        if (!bgft_init()) {
            return PKG_ERROR("BGFT_initialization", ret);
        }

        ret = sceAppInstUtilGetTitleIdFromPkg(fullpath, title_id, &is_app);
        if (ret) 
            return PKG_ERROR("sceAppInstUtilGetTitleIdFromPkg", ret);


        struct bgft_download_param_ex download_params;
        memset(&download_params, 0, sizeof(download_params));
        download_params.param.entitlement_type = 5;
        download_params.param.id = "";
        download_params.param.content_url = fullpath;
        download_params.param.content_name = "Homebrew Store";
        download_params.param.icon_path = "/update/fakepic.png";
        download_params.param.playgo_scenario_id = "0";
        download_params.param.option = BGFT_TASK_OPTION_DISABLE_CDN_QUERY_PARAM;

        download_params.slot = 0;

    retry:
        log_for_api("%s 1", __FUNCTION__);
        ret = sceBgftServiceIntDownloadRegisterTaskByStorageEx(&download_params, &task_id);
        if(ret == 0x80990088 || ret == 0x80990015)
        {
            ret = sceAppInstUtilAppUnInstall(&title_id[0]);
            if(ret != 0)
                return PKG_ERROR("sceAppInstUtilAppUnInstall", ret);

            goto retry;
        }
        else if(ret) 
            return PKG_ERROR("sceBgftServiceIntDownloadRegisterTaskByStorageEx", ret);
        

        log_for_api("Task ID(s): 0x%08X", task_id);

        ret = sceBgftServiceDownloadStartTask(task_id);
        if(ret) 
            return PKG_ERROR("sceBgftDownloadStartTask", ret);
    }
    else//bgft_download_get_task_progress
        return PKG_ERROR("no file at", ret);


    struct install_args* args = (struct install_args*)malloc(sizeof(struct install_args));
    args->title_id = strdup(title_id);
    args->task_id = task_id;
   // args->size = size;
    args->path = strdup(fullpath);
    args->fname = strdup(filename);
    args->is_thread = false;
    args->delete_pkg = true;
    install_prog((void*)args);

    log_for_api( "%s(%s) done.", __FUNCTION__, fullpath);

    return 0;
}