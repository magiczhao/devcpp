#include "svr_config.h"
#include <libtcc.h>

#define MAX_CONFIG_FILE_SIZE    (1024 * 1024)
typedef void (*load_callback)(struct frame_config*);
//TODO the buffer can not
int load_config(const char* filename, struct frame_config* config)
{
    int ret = -1;
    TCCState* tcc = NULL;
    char* config_content = NULL;
    void* tcc_buffer = NULL;
    FILE* fp = fopen(filename, "r");
    if(!fp){
        goto fini;
    }
    config_content = (char*)malloc(MAX_CONFIG_FILE_SIZE);
    if(!config_content){
        goto fini;
    }
    
    //add config structure definitions here
    //the call back function name is frame_load_config
    int pos = snprintf(config_content, MAX_CONFIG_FILE_SIZE, "%s\nvoid frame_load_config(struct frame_config* config)", frame_config_def_string);
    //implement of frame_load_config stored in the config file
    int bytes_read = fread(config_content + pos, 1, MAX_CONFIG_FILE_SIZE - pos, fp);
    pos += bytes_read;
    //printf("cont:%s\n", config_content);

    tcc = tcc_new();
    if(!tcc){
        goto fini;
    }
    //tcc_add_symbol(tcc, "config", config);
    //compile the string & call frame_load_config
    if(tcc_compile_string(tcc, config_content) == -1){
        goto fini;
    }
    int size = tcc_relocate(tcc, NULL);
    tcc_buffer = malloc(size);
    if(!tcc_buffer){
        goto fini;
    }
    tcc_relocate(tcc, tcc_buffer);
    load_callback user_config = tcc_get_symbol(tcc, "frame_load_config");
    printf("user:%d\n", (int) user_config);
    if(!user_config){
        goto fini;
    }
    //call user defined function
    user_config(config);
    ret = 0;
fini:
    if(fp){
        fclose(fp);
        fp = NULL;
    }
    if(tcc){
        tcc_delete(tcc);
        tcc = NULL;
    }
    free(config_content);
    config_content = NULL;
    free(tcc_buffer);
    tcc_buffer = NULL;
    return ret;
}
