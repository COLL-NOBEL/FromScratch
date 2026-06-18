#include <sys/stat.h>
#include <string.h>

#if defined(_WIN32)
extern "C" int stat64i32(const char* fileName, struct _stat64i32* outStat) {
    if (outStat == nullptr) {
        return -1;
    }

    struct _stat64 fullStat;
    const int result = _stat64(fileName, &fullStat);
    if (result == -1) {
        memset(outStat, 0, sizeof(struct _stat64i32));
        return -1;
    }

    outStat->st_dev = fullStat.st_dev;
    outStat->st_ino = fullStat.st_ino;
    outStat->st_mode = fullStat.st_mode;
    outStat->st_nlink = fullStat.st_nlink;
    outStat->st_uid = fullStat.st_uid;
    outStat->st_gid = fullStat.st_gid;
    outStat->st_rdev = fullStat.st_rdev;
    outStat->st_size = static_cast<_off_t>(fullStat.st_size);
    outStat->st_atime = fullStat.st_atime;
    outStat->st_mtime = fullStat.st_mtime;
    outStat->st_ctime = fullStat.st_ctime;

    return result;
}
#endif
