#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <dpsim/MNASolverDynInterface.h>

int main(int argc, char* argv[]){

    struct dpsim_mna_plugin {
        void (*log)(const char *);
        int (*init)(struct dpsim_csr_matrix*);
        int (*lu_decomp)(struct dpsim_csr_matrix*);
        int (*solve)(double *, double*);
        void (*cleanup)(void);
    };

    struct dpsim_mna_plugin *mPlugin;

    void* handle = dlopen("juliaplugin.so",RTLD_NOW);
    
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    // Call function directly
    void (*example_cleanup)(void);
    *(void **) (&example_cleanup) = dlsym(handle, "example_cleanup");

    (*example_cleanup)();

    // Call function from function pointer struct
    struct dpsim_mna_plugin* (*get_mna_plugin)(const char *);
    get_mna_plugin = (struct dpsim_mna_plugin* (*)(const char *)) dlsym(handle, "get_mna_plugin");

    if (get_mna_plugin == NULL) {
        printf("error reading symbol from library");
    }

    if (( mPlugin = get_mna_plugin("juliaplugin")) == NULL){
        printf("error getting plugin class");
    }

    printf("jumped out of library");

    mPlugin->cleanup();


    int ret = dlclose(handle);
    
    return ret;
}