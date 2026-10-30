/* station.c - Station and Slot Management module */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "station.h"
#include "utils.h"
#include "storage.h"

int find_station_by_id(int id)
{
    int i;
    for (i = 0; i < n_stations; i++)
        if (stations[i].id == id) return i;
    return -1;
}

void list_stations(void)
{
    int i;
    printf("\n%-4s %-22s %-28s %-9s %-8s %s\n",
           "ID", "Station", "Location", "Type", "Chargers", "Hours");
    printf("---------------------------------------------------------------------------------\n");
    if (n_stations == 0) printf("(no stations available)\n");
    for (i = 0; i < n_stations; i++)
        printf("%-4d %-22s %-28s %-9s %-8d %02d:00-%02d:00\n",
               stations[i].id, stations[i].name, stations[i].location,
               stations[i].type, stations[i].chargers,
               stations[i].open_hour, stations[i].close_hour);
}

/* Ask the user to pick a station; returns its index or -1 if none exist. */
int choose_station(void)
{
    int id, idx;
    if (n_stations == 0) { printf("No stations available.\n"); return -1; }
    list_stations();
    for (;;) {
        id = read_int("\nEnter Station ID: ", 1, 999999);
        idx = find_station_by_id(id);
        if (idx >= 0) return idx;
        printf("  No station with that ID.\n");
    }
}

static void read_hours(int *open, int *close)
{
    for (;;) {
        *open  = read_int("Opening hour (0-23)  : ", 0, 23);
        *close = read_int("Closing hour (1-24)  : ", 1, 24);
        if (*close > *open) return;
        printf("  Closing hour must be after opening hour.\n");
    }
}

static void read_type(char *type)
{
    int c = read_int("Charger type: 1) AC Type2  2) DC Fast  3) DC Ultra : ", 1, 3);
    strcpy(type, c == 1 ? "AC Type2" : c == 2 ? "DC Fast" : "DC Ultra");
}

/* FR8 - add a new station */
void add_station(void)
{
    Station s;
    print_header("ADD STATION");
    if (n_stations >= MAX_STATIONS) { printf("Station limit reached.\n"); return; }

    read_text("Station name   : ", s.name, NAME_LEN);
    read_text("Location       : ", s.location, LOC_LEN);
    s.chargers = read_int("No. of chargers (1-10): ", 1, MAX_CHARGERS);
    read_type(s.type);
    read_hours(&s.open_hour, &s.close_hour);

    s.id = next_station_id++;
    stations[n_stations++] = s;
    save_all();
    printf("Station added with ID %d.\n", s.id);
}

/* Is any active booking at this station outside the new rules? */
static int conflicts_with_bookings(int st_id, int new_chargers, int open, int close)
{
    int i;
    for (i = 0; i < n_res; i++) {
        Reservation *r = &reservations[i];
        if (r->station_id != st_id || r->status != STATUS_BOOKED) continue;
        if (r->charger > new_chargers || r->start_hour < open || r->end_hour > close)
            return 1;
    }
    return 0;
}

/* FR8 - edit an existing station */
void edit_station(void)
{
    Station t;
    int idx;

    print_header("EDIT STATION");
    idx = choose_station();
    if (idx < 0) return;
    t = stations[idx];

    printf("\nEditing '%s'. Re-enter all details.\n", t.name);
    read_text("Station name   : ", t.name, NAME_LEN);
    read_text("Location       : ", t.location, LOC_LEN);
    t.chargers = read_int("No. of chargers (1-10): ", 1, MAX_CHARGERS);
    read_type(t.type);
    read_hours(&t.open_hour, &t.close_hour);

    if (conflicts_with_bookings(t.id, t.chargers, t.open_hour, t.close_hour)) {
        printf("Change refused: existing active bookings would fall outside the\n"
               "new charger count or opening hours. Cancel them first.\n");
        return;
    }
    stations[idx] = t;
    save_all();
    printf("Station updated.\n");
}
