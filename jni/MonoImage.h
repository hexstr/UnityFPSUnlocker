typedef int    gint;
typedef gint   gboolean;
typedef unsigned int    guint;
typedef unsigned char guint8;
typedef unsigned short guint16;
typedef signed short gint16;
typedef unsigned int guint32;
typedef unsigned long guint64;
typedef unsigned char   guchar;
typedef void* gpointer;
typedef uint8_t mono_byte;
#define MONO_PUBLIC_KEY_TOKEN_LENGTH 17

struct MonoRefCount {
    guint32 ref;
    void (*destructor) (gpointer data);
};

struct MonoImageStorage {
    MonoRefCount ref;

    /* key used for lookups.  owned by this image storage. */
    char *key;

    /* If the raw data was allocated from a source such as mmap, the allocator may store resource tracking information here. */
    void *raw_data_handle;
    char *raw_data;
    guint32 raw_data_len;
    guint8 raw_buffer_used : 1;
    guint8 raw_data_allocated : 1;
    guint8 fileio_used : 1;
};

enum MonoImageOpenStatus {
    MONO_IMAGE_OK,
    MONO_IMAGE_ERROR_ERRNO,
    MONO_IMAGE_MISSING_ASSEMBLYREF,
    MONO_IMAGE_IMAGE_INVALID
};

struct MonoCOFFHeader {
    guint16  coff_machine;
    guint16  coff_sections;
    guint32  coff_time;
    guint32  coff_symptr;
    guint32  coff_symcount;
    guint16  coff_opt_header_size;
    guint16  coff_attributes;
};

struct MonoPEHeader {
    guint16 pe_magic;
    guchar  pe_major;
    guchar  pe_minor;
    guint32 pe_code_size;
    guint32 pe_data_size;
    guint32 pe_uninit_data_size;
    guint32 pe_rva_entry_point;
    guint32 pe_rva_code_base;
    guint32 pe_rva_data_base;
};

struct MonoPEHeaderNT {
	guint32 pe_image_base;		/* must be 0x400000 */
	guint32 pe_section_align;       /* must be 8192 */
	guint32 pe_file_alignment;      /* must be 512 or 4096 */
	guint16 pe_os_major;            /* must be 4 */
	guint16 pe_os_minor;            /* must be 0 */
	guint16 pe_user_major;
	guint16 pe_user_minor;
	guint16 pe_subsys_major;
	guint16 pe_subsys_minor;
	guint32 pe_reserved_1;
	guint32 pe_image_size;
	guint32 pe_header_size;
	guint32 pe_checksum;
	guint16 pe_subsys_required;
	guint16 pe_dll_flags;
	guint32 pe_stack_reserve;
	guint32 pe_stack_commit;
	guint32 pe_heap_reserve;
	guint32 pe_heap_commit;
	guint32 pe_loader_flags;
	guint32 pe_data_dir_count;
};

struct MonoPEDirEntry {
	guint32 rva;
	guint32 size;
};

struct MonoPEDatadir {
    MonoPEDirEntry pe_export_table;
    MonoPEDirEntry pe_import_table;
    MonoPEDirEntry pe_resource_table;
    MonoPEDirEntry pe_exception_table;
    MonoPEDirEntry pe_certificate_table;
    MonoPEDirEntry pe_reloc_table;
    MonoPEDirEntry pe_debug;
    MonoPEDirEntry pe_copyright;
    MonoPEDirEntry pe_global_ptr;
    MonoPEDirEntry pe_tls_table;
    MonoPEDirEntry pe_load_config_table;
    MonoPEDirEntry pe_bound_import;
    MonoPEDirEntry pe_iat;
    MonoPEDirEntry pe_delay_import_desc;
    MonoPEDirEntry pe_cli_header;
    MonoPEDirEntry pe_reserved;
};

struct MonoDotNetHeader {
    char            pesig[4];
    MonoCOFFHeader  coff;
    MonoPEHeader    pe;
    MonoPEHeaderNT  nt;
    MonoPEDatadir   datadir;
};

struct MonoSectionTable {
    char    st_name[8];
    guint32 st_virtual_size;
    guint32 st_virtual_address;
    guint32 st_raw_data_size;
    guint32 st_raw_data_ptr;
    guint32 st_reloc_ptr;
    guint32 st_lineno_ptr;
    guint16 st_reloc_count;
    guint16 st_line_count;

    #define SECT_FLAGS_HAS_CODE               0x20
    #define SECT_FLAGS_HAS_INITIALIZED_DATA   0x40
    #define SECT_FLAGS_HAS_UNINITIALIZED_DATA 0x80
    #define SECT_FLAGS_MEM_DISCARDABLE        0x02000000
    #define SECT_FLAGS_MEM_NOT_CACHED         0x04000000
    #define SECT_FLAGS_MEM_NOT_PAGED          0x08000000
    #define SECT_FLAGS_MEM_SHARED             0x10000000
    #define SECT_FLAGS_MEM_EXECUTE            0x20000000
    #define SECT_FLAGS_MEM_READ               0x40000000
    #define SECT_FLAGS_MEM_WRITE              0x80000000
    guint32 st_flags;
};

struct MonoCLIHeader {
    guint32        ch_size;
    guint16        ch_runtime_major;
    guint16        ch_runtime_minor;
    MonoPEDirEntry ch_metadata;

    #define CLI_FLAGS_ILONLY         0x01
    #define CLI_FLAGS_32BITREQUIRED  0x02
    #define CLI_FLAGS_STRONGNAMESIGNED 0x8
    #define CLI_FLAGS_TRACKDEBUGDATA 0x00010000
    guint32        ch_flags;

    guint32        ch_entry_point;
    MonoPEDirEntry ch_resources;
    MonoPEDirEntry ch_strong_name;
    MonoPEDirEntry ch_code_manager_table;
    MonoPEDirEntry ch_vtable_fixups;
    MonoPEDirEntry ch_export_address_table_jumps;

    /* The following are zero in the current docs */
    MonoPEDirEntry ch_eeinfo_table;
    MonoPEDirEntry ch_helper_table;
    MonoPEDirEntry ch_dynamic_info;
    MonoPEDirEntry ch_delay_load_info;
    MonoPEDirEntry ch_module_image;
    MonoPEDirEntry ch_external_fixups;
    MonoPEDirEntry ch_ridmap;
    MonoPEDirEntry ch_debug_map;
    MonoPEDirEntry ch_ip_map;
};

struct MonoCLIImageInfo {
    MonoDotNetHeader  cli_header;
    int               cli_section_count;
    MonoSectionTable  *cli_section_tables;
    void            **cli_sections;
    MonoCLIHeader     cli_cli_header;
};

struct MonoStreamHeader {
    const char* data;
    guint32  size;
};

struct MonoTableInfo {
    const char *base;
    guint       rows : 24;
    guint       row_size : 8;
    guint32   size_bitfield;
};

struct MonoAssemblyName {
    const char *name;
    const char *culture;
    const char *hash_value;
    const mono_byte* public_key;
    mono_byte public_key_token[MONO_PUBLIC_KEY_TOKEN_LENGTH];
    uint32_t hash_alg;
    uint32_t hash_len;
    uint32_t flags;
    uint16_t major, minor, build, revision, arch;
};

struct MonoImage {
    int   ref_count;
    void *raw_data_handle;
    char *raw_data;
    guint32 raw_data_len;
    guint8 raw_buffer_used : 1;
    guint8 raw_data_allocated : 1;
    guint8 fileio_used : 1;
    guint8 dynamic : 1;
    guint8 ref_only : 1;
    guint8 uncompressed_metadata : 1;
    guint8 metadata_only : 1;
    guint8 load_from_context : 1;
    guint8 checked_module_cctor : 1;
    guint8 has_module_cctor : 1;
    guint8 idx_string_wide : 1;
    guint8 idx_guid_wide : 1;
    guint8 idx_blob_wide : 1;
    guint8 core_clr_platform_code : 1;
    char *name;
    const char *assembly_name;
    const char *module_name;
    char *version;
    gint16 md_version_major, md_version_minor;
    char *guid;
    MonoCLIImageInfo    *image_info;
    void         *mempool;
    char                *raw_metadata;
    MonoStreamHeader     heap_strings;
    MonoStreamHeader     heap_us;
    MonoStreamHeader     heap_blob;
    MonoStreamHeader     heap_guid;
    MonoStreamHeader     heap_tables;
    MonoStreamHeader     heap_pdb;
    const char          *tables_base;
    guint64 referenced_tables;
    int *referenced_table_rows;
};