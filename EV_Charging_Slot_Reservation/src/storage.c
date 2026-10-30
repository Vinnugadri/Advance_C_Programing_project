/* storage.c - permanent storage using text files (FR9) */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "storage.h"
#include "utils.h"

User        users[MAX_USERS];
Station     stations[MAX_STATIONS];
Reservation reservations[MAX_RES];
int n_users = 0, n_stations = 0, n_res = 0;
int next_user_id = 1, next_station_id = 1, next_res_id = 1001;
unsigned long admin_hash = 0;

#define DEFAULT_ADMIN_PASSWORD "admin123"
#define MAX_FIELDS 10

/* Split a line in place on '|'; returns number of fields. */
static int split_fields(char *line, char *f[], int max)
{
    int n = 0;
    char *p = line;
    size_t len = strlen(line);

    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';
    if (len == 0) return 0;

    f[n++] = p;
    while (*p && n < max) {
        if (*p == '|') { *p = '\0'; f[n++] = p + 1; }
        p++;
    }
    return n;
}

static void copy_field(char *dst, size_t n, const char *src)
{
    strncpy(dst, src, n - 1);
    dst[n - 1] = '\0';
}

static void load_users(void)
{
    FILE *fp = fopen(USERS_FILE, "r");
    char line[256], *f[MAX_FIELDS];
    if (!fp) return;
    while (n_users < MAX_USERS && fgets(line, sizeof line, fp)) {
        if (split_fields(line, f, MAX_FIELDS) != 5) continue;
        User *u = &users[n_users];
        u->id = atoi(f[0]);
        copy_field(u->name, NAME_LEN, f[1]);
        copy_field(u->phone, PHONE_LEN, f[2]);
        copy_field(u->vehicle, VEHICLE_LEN, f[3]);
        u->pass_hash = strtoul(f[4], NULL, 10);
        if (u->id >= next_user_id) next_user_id = u->id + 1;
        n_users++;
    }
    fclose(fp);
}

static void load_stations(void)
{
    FILE *fp = fopen(STATIONS_FILE, "r");
    char line[300], *f[MAX_FIELDS];
    if (!fp) return;
    while (n_stations < MAX_STATIONS && fgets(line, sizeof line, fp)) {
        if (split_fields(line, f, MAX_FIELDS) != 7) continue;
        Station *s = &stations[n_stations];
        s->id = atoi(f[0]);
        copy_field(s->name, NAME_LEN, f[1]);
        copy_field(s->location, LOC_LEN, f[2]);
        s->chargers = atoi(f[3]);
        copy_field(s->type, TYPE_LEN, f[4]);
        s->open_hour = atoi(f[5]);
        s->close_hour = atoi(f[6]);
        if (s->id >= next_station_id) next_station_id = s->id + 1;
        n_stations++;
    }
    fclose(fp);
}

static void load_reservations(void)
{
    FILE *fp = fopen(RES_FILE, "r");
    char line[300], *f[MAX_FIELDS];
    if (!fp) return;
    while (n_res < MAX_RES && fgets(line, sizeof line, fp)) {
        if (split_fields(line, f, MAX_FIELDS) != 8) continue;
        Reservation *r = &reservations[n_res];
        r->id = atoi(f[0]);
        r->user_id = atoi(f[1]);
        r->station_id = atoi(f[2]);
        r->charger = atoi(f[3]);
        copy_field(r->date, DATE_LEN, f[4]);
        r->start_hour = atoi(f[5]);
        r->end_hour = atoi(f[6]);
        r->status = atoi(f[7]);
        if (r->id >= next_res_id) next_res_id = r->id + 1;
        n_res++;
    }
    fclose(fp);
}

static void load_admin(void)
{
    FILE *fp = fopen(ADMIN_FILE, "r");
    char line[64];
    if (fp && fgets(line, sizeof line, fp))
        admin_hash = strtoul(line, NULL, 10);
    else
        admin_hash = hash_password(DEFAULT_ADMIN_PASSWORD);
    if (fp) fclose(fp);
}

/* On the very first run, create a few demo stations. */
static void seed_stations(void)
{
    static const struct { const char *name, *loc, *type; int ch, open, close; } d[] = {
        {"GreenCharge Central", "City Centre Mall, Block A", "DC Fast",  3, 6, 22},
        {"VoltPoint Highway",   "NH-5 Highway Plaza",        "AC Type2", 4, 0, 24},
        {"EcoPlug Campus",      "University Main Gate",      "AC Type2", 2, 8, 20}
    };
    int i;
    for (i = 0; i < 3; i++) {
        Station *s = &stations[n_stations++];
        s->id = next_station_id++;
        copy_field(s->name, NAME_LEN, d[i].name);
        copy_field(s->location, LOC_LEN, d[i].loc);
        copy_field(s->type, TYPE_LEN, d[i].type);
        s->chargers = d[i].ch;
        s->open_hour = d[i].open;
        s->close_hour = d[i].close;
    }
}

void save_all(void)
{
    FILE *fp;
    int i;

    if ((fp = fopen(USERS_FILE, "w")) != NULL) {
        for (i = 0; i < n_users; i++)
            fprintf(fp, "%d|%s|%s|%s|%lu\n", users[i].id, users[i].name,
                    users[i].phone, users[i].vehicle, users[i].pass_hash);
        fclose(fp);
    } else printf("Warning: could not write %s\n", USERS_FILE);

    if ((fp = fopen(STATIONS_FILE, "w")) != NULL) {
        for (i = 0; i < n_stations; i++)
            fprintf(fp, "%d|%s|%s|%d|%s|%d|%d\n", stations[i].id, stations[i].name,
                    stations[i].location, stations[i].chargers, stations[i].type,
                    stations[i].open_hour, stations[i].close_hour);
        fclose(fp);
    } else printf("Warning: could not write %s\n", STATIONS_FILE);

    if ((fp = fopen(RES_FILE, "w")) != NULL) {
        for (i = 0; i < n_res; i++)
            fprintf(fp, "%d|%d|%d|%d|%s|%d|%d|%d\n", reservations[i].id,
                    reservations[i].user_id, reservations[i].station_id,
                    reservations[i].charger, reservations[i].date,
                    reservations[i].start_hour, reservations[i].end_hour,
                    reservations[i].status);
        fclose(fp);
    } else printf("Warning: could not write %s\n", RES_FILE);

    if ((fp = fopen(ADMIN_FILE, "w")) != NULL) {
        fprintf(fp, "%lu\n", admin_hash);
        fclose(fp);
    }
}

void load_all(void)
{
    load_users();
    load_stations();
    load_reservations();
    load_admin();
    if (n_stations == 0) {
        seed_stations();
        save_all();
    }
}
