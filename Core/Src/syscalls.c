#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

/* These are the low-level OS hooks that newlib's libc expects to find.
 * On bare metal there is no OS, so we provide minimal stubs. None of
 * these will actually be called at runtime: FreeRTOS owns the heap
 * (pvPortMalloc / heap_4), so newlib's malloc — and therefore _sbrk —
 * is never invoked. The stubs exist only to satisfy the linker. */

extern uint32_t _ebss;

void *_sbrk(int incr)
{
    static uint8_t *heap_end = NULL;
    if (heap_end == NULL)
        heap_end = (uint8_t *)&_ebss;

    uint8_t *prev = heap_end;
    heap_end += incr;
    return (void *)prev;
}

int  _write(int fd, char *ptr, int len)       { (void)fd; (void)ptr; return len; }
int  _read(int fd, char *ptr, int len)        { (void)fd; (void)ptr; (void)len; return 0; }
int  _close(int fd)                           { (void)fd; return -1; }
int  _lseek(int fd, int ptr, int dir)         { (void)fd; (void)ptr; (void)dir; return 0; }
int  _fstat(int fd, struct stat *st)          { (void)fd; st->st_mode = S_IFCHR; return 0; }
int  _isatty(int fd)                          { (void)fd; return 1; }
int  _getpid(void)                            { return 1; }
int  _kill(int pid, int sig)                  { (void)pid; (void)sig; errno = EINVAL; return -1; }
void _exit(int status)                        { (void)status; for (;;); }
