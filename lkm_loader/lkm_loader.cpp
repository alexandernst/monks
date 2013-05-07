#include "lkm_loader.h"

// -1 = error
//  0 = not loaded
//  1 = loaded
int LKM_Loader::check(){
    ctx = kmod_new(NULL, &null_config);
    if(ctx == NULL){
        ret = -1;
    }else{
        err = kmod_module_new_from_loaded(ctx, &list);
        if(err < 0){
            ret = -1;
        }else{
            ret = 0;
            kmod_list_foreach(itr, list){
                mod = kmod_module_get_module(itr);
                if(strcmp(kmod_module_get_name(mod), PROCMON_NAME) == 0)
                    ret = 1;
                kmod_module_unref(mod);
            }
            kmod_module_unref_list(list);
        }
    }

    kmod_unref(ctx);
    return ret;
}

// -1 = error
//  0 = loaded OK
//  1 = loaded KO
int LKM_Loader::load(){
    ctx = kmod_new(NULL, &null_config);
    if(ctx == NULL){
        ret = -1;
    }else{
        err = kmod_module_new_from_path(ctx, PROCMON_PATH, &mod);
        if(err != 0){
            ret = -1;
        }else{
            err = kmod_module_insert_module(mod, 0, NULL);
            ret = err != 0 ? 1 : 0;
        }
    }

    kmod_unref(ctx);
    return ret;
}

// -1 = error
//  0 = unloaded OK
//  1 = unloaded KO
int LKM_Loader::unload(){
    ctx = kmod_new(NULL, &null_config);
    if(ctx == NULL){
        ret = -1;
    }else{
        err = kmod_module_new_from_name(ctx, PROCMON_NAME, &mod);
        if(err != 0){
            ret = -1;
        }else{
            err = kmod_module_remove_module(mod, 0);
            ret = err != 0 ? 1 : 0;
        }
    }

    kmod_unref(ctx);
    return ret;
}
