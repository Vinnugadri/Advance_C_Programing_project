/* utils.c - input handling, validation and helper routines */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "common.h"
#include "utils.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

/* Read one line safely; strips newline and discards any overflow. */
void read_line(const char *prompt, char *buf, size_t n)
{
    size_t len;
    int c;

    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, (int)n, stdin) == NULL) {
        printf("\nInput closed. Exiting.\n");
        exit(0);
    }
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        while ((c = getchar()) != '\n' && c != EOF)
            ;                                   /* flush the rest of a long line */
    }
    /* trim leading spaces */
    {
        char *p = buf;
        while (*p == ' ' || *p == '\t') p++;
        if (p != buf) memmove(buf, p, strlen(p) + 1);
    }
    /* trim trailing spaces */
    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == ' ' || buf[len - 1] == '\t' || buf[len - 1] == '\r'))
        buf[--len] = '\0';
}

/* Read a non-empty text field that does not contain the '|' file separator. */
void read_text(const char *prompt, char *buf, size_t n)
{
    for (;;) {
        read_line(prompt, buf, n);
        if (buf[0] == '\0')
            printf("  Input cannot be empty. Try again.\n");
        else if (strchr(buf, '|') != NULL)
            printf("  The character '|' is not allowed. Try again.\n");
        else
            return;
    }
}

/* Read an integer within [min, max]; repeats until valid. */
int read_int(const char *prompt, int min, int max)
{
    char line[32], *end;
    long v;

    for (;;) {
        read_line(prompt, line, sizeof line);
        if (line[0] != '\0') {
            v = strtol(line, &end, 10);
            if (*end == '\0' && v >= min && v <= max)
                return (int)v;
        }
        printf("  Please enter a number between %d and %d.\n", min, max);
    }
}

/* Read a password without echoing it to the screen. */
void read_password(const char *prompt, char *buf, size_t n)
{
    size_t i = 0;
    printf("%s", prompt);
    fflush(stdout);

#ifdef _WIN32
    {
        int c;
        while ((c = _getch()) != '\r' && c != '\n') {
            if (c == '\b') { if (i > 0) i--; }
            else if (i + 1 < n) buf[i++] = (char)c;
        }
        buf[i] = '\0';
        printf("\n");
    }
#else
    {
        struct termios oldt, newt;
        int c, have_tty = isatty(STDIN_FILENO);
        if (have_tty) {
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(tcflag_t)ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        }
        while ((c = getchar()) != '\n' && c != EOF)
            if (i + 1 < n) buf[i++] = (char)c;
        buf[i] = '\0';
        if (have_tty) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            printf("\n");
        }
        if (c == EOF && i == 0) { printf("\nInput closed. Exiting.\n"); exit(0); }
    }
#endif
}

int valid_phone(const char *s)
{
    int i;
    if (strlen(s) != 10) return 0;
    for (i = 0; i < 10; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

void read_phone(const char *prompt, char *buf)
{
    char tmp[32];
    for (;;) {
        read_line(prompt, tmp, sizeof tmp);
        if (valid_phone(tmp)) { strcpy(buf, tmp); return; }
        printf("  Phone number must be exactly 10 digits.\n");
    }
}

/* Validate a date in YYYY-MM-DD format (including leap years). */
int valid_date(const char *s)
{
    int y, m, d, i;
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxd;

    if (strlen(s) != 10 || s[4] != '-' || s[7] != '-') return 0;
    for (i = 0; i < 10; i++)
        if (i != 4 && i != 7 && !isdigit((unsigned char)s[i])) return 0;
    if (sscanf(s, "%d-%d-%d", &y, &m, &d) != 3) return 0;
    if (y < 2000 || y > 2100 || m < 1 || m > 12) return 0;
    maxd = dim[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) maxd = 29;
    return d >= 1 && d <= maxd;
}

void today_str(char *out)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(out, DATE_LEN, "%Y-%m-%d", tm);
}

int current_hour(void)
{
    time_t t = time(NULL);
    return localtime(&t)->tm_hour;
}

/* Read a valid date that is today or later. */
void read_date(const char *prompt, char *out)
{
    char today[DATE_LEN], tmp[32];
    today_str(today);
    for (;;) {
        read_line(prompt, tmp, sizeof tmp);
        if (!valid_date(tmp))
            printf("  Invalid date. Use YYYY-MM-DD (e.g. %s).\n", today);
        else if (strcmp(tmp, today) < 0)
            printf("  That date is in the past.\n");
        else { strcpy(out, tmp); return; }
    }
}

/* djb2 string hash - passwords are never stored as plain text. */
unsigned long hash_password(const char *s)
{
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*s++) != 0)
        h = ((h << 5) + h) + (unsigned long)c;
    return h;
}

void to_upper(char *s)
{
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

void pause_screen(void)
{
    char tmp[8];
    read_line("\nPress Enter to continue...", tmp, sizeof tmp);
}

void print_header(const char *title)
{
    printf("\n==============================================\n");
    printf("  %s\n", title);
    printf("==============================================\n");
}
