#include <jni.h>
#include <string>

static char package_name[256];
const char* package = "com.aniplex.fategrandorder";
int is_app_need_hook(JNIEnv *env, jstring jAppDataDir) {
    if (jAppDataDir) {
        const char *appDataDir = env->GetStringUTFChars(jAppDataDir, nullptr);
        int user = 0;
        if (sscanf(appDataDir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
            if (sscanf(appDataDir, "/data/%*[^/]/%s", package_name) != 1) {
                package_name[0] = '\0';
                return false;
            }
        }
        env->ReleaseStringUTFChars(jAppDataDir, appDataDir);
    } else {
        return false;
    }
    if (strcmp(package, package_name) == 0)
    		return 1;
    return 0;
}

long get_module_base () {
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];
    snprintf (filename, sizeof(filename), "/proc/self/maps");
    fp = fopen (filename, "r");
    if (fp != NULL) {
        while (fgets (line, sizeof(line), fp)) {
            if (strstr (line, "libmono.so")) {
                pch = strtok (line, "-");
                addr = strtoul (pch, NULL, 16);
                if (addr == 0x8000)
                    addr = 0;
                break;
            }
        }
        fclose (fp);
    }
    return (long)addr;
}