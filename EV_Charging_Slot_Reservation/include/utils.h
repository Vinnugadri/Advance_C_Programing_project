/* utils.h - input handling, validation and helper routines */
#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>

void read_line(const char *prompt, char *buf, size_t n);
void read_text(const char *prompt, char *buf, size_t n);   /* non-empty, no '|' */
int  read_int(const char *prompt, int min, int max);
void read_password(const char *prompt, char *buf, size_t n);
void read_phone(const char *prompt, char *buf);
void read_date(const char *prompt, char *out);             /* valid, not in the past */

int  valid_date(const char *s);
int  valid_phone(const char *s);
void today_str(char *out);
int  current_hour(void);
unsigned long hash_password(const char *s);
void to_upper(char *s);
void pause_screen(void);
void print_header(const char *title);

#endif
