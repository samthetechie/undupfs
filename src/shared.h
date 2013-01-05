#ifndef SHARED_H_
#define SHARED_H_

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

#define ASSERT(cond_) do { if (!(cond_)) die("%s:%d: ASSERT failed: %s\n", \
                                        __FILE__, __LINE__, #cond_); } while(0)

/* util.c */

void die(char *fmt, ...);
double rtc(void);

extern int o_verbose;
extern FILE *f_debug;

void verbose(char *fmt, ...);
void debug(char *fmt, ...);

#endif /* SHARED_H_ */