#ifndef BROWSER_H
#define BROWSER_H

typedef struct {
    char displayname[64];
    int  dircheck;
    char filename[256];
} entries;

typedef struct {
    int offset_x;
    int offset_y;
    u8 display;
    u8 interlace;
    u8 filter;
    u8 sprite_limit;
    char elfpath[1024];
    char savepath[1024];
    //char skinpath[1024];
    u16 PlayerInput[2][10];
    int autofire_pattern;
} vars;

typedef struct {
    u64 frame;
    u64 textcolor;
    u64 highlight;
    u64 bgColor1;
    u64 bgColor2;
    u64 bgColor3;
    u64 bgColor4;
    char bgTexture[1024];
    char bgMenu[1024];
} skin;

// Prototypes
char* browseup(char *path);
int RomBrowserInput(int files_too, int inside_menu);
int listdir(char *path, entries *FileEntry, int files_too);
int listcdvd(const char *path, entries *FileEntry);
int listpfs(char *path, entries *FileEntry, int files_too);
int listpartitions(entries *FileEntry);
char *partname(char *d, const char *hdd_path);
void unmountPartition(int pfs_number);
int mountPartition(char *name);
char* Browser(int files_too, int menu_id, int filtered);

#endif
