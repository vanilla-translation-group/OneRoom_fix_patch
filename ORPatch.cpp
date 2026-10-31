#include <Windows.h>
#include <psapi.h>
#include <direct.h>
#include <stdio.h>
#include <string>
#include "thirdparty/MinHook.h"
#include "thirdparty/krkrz/tjs.h"
#include "thirdparty/krkrz/StorageIntf.h"
#include "hashset.h"
#include "constants.h"

#define NOTNULL(code) if ((code) == NULL) break

wchar_t *cwd;

hashset_t list = NULL;

wchar_t *myfgetws(wchar_t *str, int n, FILE *fp) {
    wchar_t c;
    for (int i = 0; i < n; i++) {
        if (fread(&c, sizeof(wchar_t), 1, fp) != 1) {
            return NULL;
        }
        *str++ = c;
        if (c == L'\n') {
            break;
        }
    }
    return str;
}

typedef HMODULE (WINAPI *LOADLIBRARYW)(LPCWSTR);

LOADLIBRARYW fpLoadLibraryW = NULL;

HMODULE WINAPI DetourLoadLibraryW(LPCWSTR lpLibFileName) {
    HMODULE lpBase = fpLoadLibraryW(lpLibFileName);
    if (wcsstr(lpLibFileName, L"\\krkr_") != NULL) {
        MODULEINFO modInfo;
        DWORD flProtect;
        GetModuleInformation(GetCurrentProcess(), lpBase, &modInfo, sizeof(modInfo));
        VirtualProtect(lpBase, modInfo.SizeOfImage, PAGE_EXECUTE_READWRITE, &flProtect);
        if (memcmp((unsigned char *)((int)lpBase + relative), pdata, 6) == 0) {
            memcpy((unsigned char *)((int)lpBase + relative + 4), pdata, 2);
        }
    }
    return lpBase;
}

typedef tTJSBinaryStream* (FASTCALL *TVPCREATESTREAM)(ttstr*, tjs_uint32);

TVPCREATESTREAM fpTVPCreateStream = NULL;

tTJSBinaryStream* FASTCALL DetourTVPCreateStream(ttstr* name, tjs_uint32 flags) {
    wchar_t *url = (wchar_t*)(name->c_str()), *file;
    OutputDebugStringW(url);
    if (wcsncmp(url, L"arc://", 6) == 0) {
        file = url + 8;
    }
    else if (wcsstr(url, L"://") == NULL) {
        file = url;
    }
    else {
        goto not_modified;
    }
    if (hashset_is_member(list, file)) {
        wchar_t str[1024], path[20];
        TJS_snprintf(str, sizeof(str) - 1, L"override\\%s", file);
        if (_waccess(str, 0) == 0) {
            wcscpy(path, L"override/");
        }
        else {
            wcscpy(path, L"override.xp3>");
        }
        TJS_snprintf(str, sizeof(str) - 1, L"file://./%s/%s%s", cwd, path, file);
        ((void (*)(ttstr*, const tjs_char*))fixed)(name, str);
    }
not_modified:
    return fpTVPCreateStream(name, flags);
}

int Init() {
    if ((cwd = _wgetcwd(NULL, 512)) == NULL) {
        return 1;
    }

    if (cwd[1] == L':') {
        cwd[1] = cwd[0];
        cwd += 1;
    }

    for (int i = 0; i < wcslen(cwd); i++) {
        if (cwd[i] == L'\\')
            cwd[i] = L'/';
        if (cwd[i] >= 'A' && cwd[i] <= 'Z')
            cwd[i] += 32;
    }

    FILE* fp;

    if ((fp = fopen("filelist.dat", "rb")) == NULL) {
        return 1;
    }

    wchar_t file[256];
    int flag;

    while ((flag = fgetc(fp)) != EOF) {
        for (int i = 0; i < 8; i++) {
            if (flag & 1) {
                NOTNULL(myfgetws(file, sizeof(file) / 2, fp));
            }
            else {
                char tfile[256];
                NOTNULL(fgets(tfile, sizeof(tfile), fp));
                mbstowcs(file, tfile, sizeof(tfile));
            }
            file[wcscspn(file, L"\n")] = 0;
            hashset_add(list, file);
            flag >>= 1;
        }
    }

    if (MH_Initialize() != MH_OK) {
        return 1;
    }

    if (MH_CreateHook((LPVOID*)&LoadLibraryW, (LPVOID*)&DetourLoadLibraryW, (LPVOID*)&fpLoadLibraryW) != MH_OK) {
        return 1;
    }

    if (MH_EnableHook((LPVOID*)&LoadLibraryW) != MH_OK) {
        return 1;
    }

    if (MH_CreateHook((LPVOID*)absolute, (LPVOID*)&DetourTVPCreateStream, (LPVOID*)&fpTVPCreateStream) != MH_OK) {
        return 1;
    }

    if (MH_EnableHook((LPVOID*)absolute) != MH_OK) {
        return 1;
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            if (Init())
                MessageBoxW(NULL, L"Failed to initialize patch",
                            L"Error", MB_OK | MB_ICONWARNING);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
