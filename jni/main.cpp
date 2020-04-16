#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <dlfcn.h>
#include "main.h"
#include "elfio.hpp"

static char package_name[256];
const char* package = "com.bilibili.fatego";
static int enable_hook;

int inject (long start_addr) {
	/*replace elf header
	7F 45 4C 46 01 01 42 79 45 74 68 65 72 65 61 6C 03 00 28 00 01
	*/
	//for (int i = 0; i < 21; ++i) {
	//	((unsigned char*)start_addr)[i] = header[i];
	//}
	unsigned char* ptr = (unsigned char*)start_addr;
	/* Load library and replace .dynsym section and .dynstr section */
	ELFIO::elfio reader;
	if (!reader.load("/data/data/com.bilibili.fatego/lib/libmono.so")) {
		LOGD("load libmono.so failed");
		return -1;
	}
	
    /* SHT_STRTAB */
	//ELFIO::section* psec = reader.sections[2];//x86
	ELFIO::section* psec = reader.sections[3];
	if (psec->get_type() == SHT_STRTAB) {
		char* dynstrSec = (char*)malloc(psec->get_size());
		if (dynstrSec) {
			memcpy(dynstrSec, ptr + psec->get_offset(), psec->get_size());
			dynstrSec[psec->get_size()] = '\0';
			psec->set_data(dynstrSec, psec->get_size());
		}
		else {
			LOGD( "SHT_STRTAB malloc failed");
            return -1;
		}
	}
	/* SHT_DYNSYM */
	//psec = reader.sections[1];//x86
	psec = reader.sections[2];
	if (psec->get_type() == SHT_DYNSYM) {
		char* dynsymSec = (char*)malloc(psec->get_size());
		if (dynsymSec) {
			memcpy(dynsymSec, ptr + psec->get_offset(), psec->get_size());
			dynsymSec[psec->get_size()] = '\0';
			psec->set_data(dynsymSec, psec->get_size());
		}
		else {
			LOGD("SHT_DYNSYM malloc failed");
            return -1;
		}
	}
    
	/* Define function address */
	long mono_get_root_domain_addr = start_addr;
	long mono_thread_attach_addr = start_addr;
	long mono_class_from_name_addr = start_addr;
	long mono_object_new_addr = start_addr;
	long mono_runtime_object_init_addr = start_addr;
    
	/* Search symbol */
	const ELFIO::symbol_section_accessor symbols(reader, psec);
	std::string   name;
	ELFIO::Elf64_Addr    value;
	ELFIO::Elf_Xword     size;
	unsigned char bind;
	unsigned char type;
	ELFIO::Elf_Half      section_index;
	unsigned char other;
	int methodcount = 0;
	for (int i = 0; i < 800/*symbols.get_symbols_num()*/; ++i) {
		if (methodcount == 5) {
			break;
		}
		symbols.get_symbol(i, name, value, size, bind,
			type, section_index, other);
		if (!name.compare("mono_get_root_domain")) {
			LOGD("mono_get_root_domain: %d address: %02llX", i, value);
			mono_get_root_domain_addr += value;
			methodcount++;
		}
		if (!name.compare("mono_thread_attach")) {
			LOGD("mono_thread_attach: %d address: %02llX", i, value);
			mono_thread_attach_addr += value;
			methodcount++;
		}
		//if (!name.compare("mono_image_open_from_data_with_name")) {
		//	LOGD("mono_image_open_from_data_with_name: %d address: %02llX", i, value);
		//	mono_image_open_from_data_with_name_addr += value;
		//}
		//if (!name.compare("mono_assembly_load_from_full")) {
		//	LOGD("mono_assembly_load_from_full: %d address: %02llX", i, value);
		//	mono_assembly_load_from_full_addr += value;
		//}
		if (!name.compare("mono_class_from_name")) {
			LOGD("mono_class_from_name: %d address: %02llX", i, value);
			mono_class_from_name_addr += value;
			methodcount++;
		}
		if (!name.compare("mono_object_new")) {
			LOGD("mono_object_new: %d address: %02llX", i, value);
			mono_object_new_addr += value;
			methodcount++;
		}
		if (!name.compare("mono_runtime_object_init")) {
			LOGD("mono_runtime_object_init: %d address: %02llX", i, value);
			mono_runtime_object_init_addr += value;
			methodcount++;
		}
	}
    
	if (mono_get_root_domain_addr && mono_thread_attach_addr /* && mono_image_open_from_data_with_name_addr mono_image_open_from_data_addr &&
		mono_assembly_load_from_full_addr*/ && mono_class_from_name_addr && mono_object_new_addr &&
		mono_runtime_object_init_addr) {
		/*
		Fix function header
		00 48 2D E9                 STMFD           SP!, {R11,LR}
        04 B0 8D E2                 ADD             R11, SP, #4
		*/
		//unsigned char* ptr = (unsigned char*)mono_class_from_name_addr;
		//ptr[0] = 0;
		//ptr[1] = 0x48;
		//ptr[2] = 0x2D;
		//ptr[3] = 0xE9;
		//ptr[4] = 4;
		//ptr[5] = 0xB0;
		//ptr[6] = 0x8D;
		//ptr[7] = 0xE2;

		void* handle = dlopen("/system/lib/libmono.so", 0);
		if (!handle) {
			LOGD("dlopen failed: %s", dlerror());
			return -1;
		}
		mono_image_open_from_data_t mono_image_open_from_data = (mono_image_open_from_data_t)dlsym(handle, "mono_image_open_from_data");
		LOGD("mono_image_open_from_data: %p", mono_image_open_from_data);
		mono_assembly_load_from_full_t mono_assembly_load_from_full = (mono_assembly_load_from_full_t)dlsym(handle, "mono_assembly_load_from_full");
		LOGD("mono_assembly_load_from_full: %p", mono_assembly_load_from_full);

		mono_get_root_domain_t mono_get_root_domain = (mono_get_root_domain_t)(mono_get_root_domain_addr);
		mono_thread_attach_t mono_thread_attach = (mono_thread_attach_t)(mono_thread_attach_addr);
		mono_class_from_name_t mono_class_from_name = (mono_class_from_name_t)(mono_class_from_name_addr);
		mono_object_new_t mono_object_new = (mono_object_new_t)(mono_object_new_addr);
		mono_runtime_object_init_t mono_runtime_object_init = (mono_runtime_object_init_t)(mono_runtime_object_init_addr);

        /* Read configuration */
        std::ifstream cfg ("/data/local/tmp/unlocker");
        if (!cfg.is_open()) {
            LOGD("failed to open configuration file");
        } else {
            std::string val;
            getline(cfg, val);
            int rate = std::stoi(val);
            LOGD("read cfg: %02X", rate);
            raw[0x265] = rate;
        }

		mono_thread_attach(mono_get_root_domain());
		MonoImageOpenStatus status;
		int D_size = sizeof(raw) / sizeof(raw[0]);
		void* image = mono_image_open_from_data((char*)raw, D_size, 1, &status);
		mono_assembly_load_from_full(image, "FPSUnlocker", &status, 0);
		void* pClass = mono_class_from_name(image, "UnityEngine", "init");
		void* method = mono_object_new(mono_get_root_domain(), pClass);
		mono_runtime_object_init(method);
		LOGD("Plugin has been loaded");
		return 0;
	} else {
		LOGD("Could not find function");
		return -2;
	}
}

void* thereisnothing(void *args) {
    LOGD("***** new thread: [%d] *****", gettid());
    sleep(10);
    LOGD("***** begin *****");
    int count = 0;
    while (true) {
		long start_addr = getLibAddr();
        if (start_addr) {
			LOGD("start_addr %02lX", start_addr);
            inject (start_addr);
            break;
        }
        else {
            count++;
            sleep (1);
            if (count > 20) {
                LOGD ("**** find module base address failed ****");
                break;
            }
        }
    }
    LOGD ("***** finish *****");
    return 0;
}

//__attribute__((constructor))
//void init() {
//	int temp;
//	pthread_t ntid;
//	if ((temp = pthread_create(&ntid, nullptr, thereisnothing, nullptr))) {
//		LOGD("can't create thread");
//	}
//}

/*riru api*/
extern "C" {
    __attribute__((visibility("default")))
        void nativeForkAndSpecializePre(
            JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtime_flags,
            jobjectArray *rlimits, jint *_mount_external, jstring *se_info, jstring *se_name,
            jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
            jstring *instructionSet, jstring *appDataDir, jstring *packageName,
            jobjectArray *packagesForUID, jstring *sandboxId) {
        // packageName, packagesForUID, sandboxId exists from Android Q
        enable_hook = is_app_need_hook(env, *appDataDir);
    }
    __attribute__((visibility("default")))
        int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
        if (res == 0 && enable_hook) {
            int temp;
            pthread_t ntid;
            if ((temp = pthread_create(&ntid, NULL, thereisnothing, NULL))) {
                LOGD("can't create thread");
            }
        }
        return !enable_hook;
    }
}

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

long getLibAddr() {
	std::ifstream in(R"(/proc/self/maps)");
	std::string line;
	std::string::size_type idx = 0;
	if (in) {
		while (getline(in, line)) {
			idx = line.find("libmono.so");
			if (idx != std::string::npos) {
				return std::stoul(line.substr(0, 8), 0, 16);
			}
		}
	}
	return 0;
}