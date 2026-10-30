/* reservation.c - Reservation, Cancellation and booking-view module */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "reservation.h"
#include "station.h"
#include "user.h"
#include "utils.h"
#include "storage.h"

/* FR5 - returns 1 if the charger is free for [start,end) on that date.
   Two bookings overlap when start1 < end2 AND start2 < end1. */
int is_available(int station_id, int charger, const char *date, int start, int end)
{
    int i;
    for (i = 0; i < n_res; i++) {
        const Reservation *r = &reservations[i];
        if (r->status != STATUS_BOOKED) continue;
        if (r->station_id != station_id || r->charger != charger) continue;
        if (strcmp(r->date, date) != 0) continue;
        if (start < r->end_hour && r->start_hour < end)
            return 0;
    }
    return 1;
}

/* FR3 - grid of hours x chargers for one station and date */
void show_availability(int st_idx, const char *date)
{
    const Station *s = &stations[st_idx];
    int h, c;

    printf("\nAvailability at %s on %s   ( . = free   X = booked )\n", s->name, date);
    printf("Hour     ");
    for (c = 1; c <= s->chargers; c++) printf("Chg%-2d ", c);
    printf("\n---------");
    for (c = 1; c <= s->chargers; c++) printf("------");
    printf("\n");
    for (h = s->open_hour; h < s->close_hour; h++) {
        printf("%02d-%02d    ", h, h + 1);
        for (c = 1; c <= s->chargers; c++)
            printf("  %c   ", is_available(s->id, c, date, h, h + 1) ? '.' : 'X');
        printf("\n");
    }
}

void print_booking_row(const Reservation *r)
{
    int si = find_station_by_id(r->station_id);
    printf("BK%-6d %-20s Chg %-2d %s  %02d:00-%02d:00  %s\n",
           r->id, si >= 0 ? stations[si].name : "(removed)", r->charger, r->date,
           r->start_hour, r->end_hour,
           r->status == STATUS_BOOKED ? "BOOKED" : "CANCELLED");
}

/* Suggest other free options when the first choice is taken. */
static void suggest_alternatives(int st_idx, const char *date, int charger,
                                 int start, int dur)
{
    const Station *s = &stations[st_idx];
    int c, h, found = 0;

    printf("\nSuggested alternatives:\n");
    for (c = 1; c <= s->chargers; c++) {
        if (c != charger && is_available(s->id, c, date, start, start + dur)) {
            printf("  - Charger %d is free at %02d:00-%02d:00\n", c, start, start + dur);
            found = 1;
        }
    }
    for (h = s->open_hour; h + dur <= s->close_hour; h++) {
        if (h != start && is_available(s->id, charger, date, h, h + dur)) {
            printf("  - Charger %d is free at %02d:00-%02d:00\n", charger, h, h + dur);
            if (++found >= 8) break;
        }
    }
    if (!found) printf("  (none found - try another date or station)\n");
}

/* FR4, FR6 - book a slot */
void book_slot(int user_idx)
{
    Reservation r;
    int st_idx, dur;
    const Station *s;
    char today[DATE_LEN];

    print_header("BOOK A CHARGING SLOT");
    if (n_res >= MAX_RES) { printf("Reservation storage is full.\n"); return; }

    st_idx = choose_station();
    if (st_idx < 0) return;
    s = &stations[st_idx];

    read_date("Date (YYYY-MM-DD) : ", r.date);
    show_availability(st_idx, r.date);

    r.charger    = read_int("\nCharger number     : ", 1, s->chargers);
    r.start_hour = read_int("Start hour (0-23)  : ", 0, 23);
    dur          = read_int("Duration in hours (1-4): ", 1, MAX_DURATION);
    r.end_hour   = r.start_hour + dur;

    today_str(today);
    if (r.start_hour < s->open_hour || r.end_hour > s->close_hour) {
        printf("\nBooking refused: station is open %02d:00-%02d:00 only.\n",
               s->open_hour, s->close_hour);
        return;
    }
    if (strcmp(r.date, today) == 0 && r.start_hour <= current_hour()) {
        printf("\nBooking refused: that start time has already passed today.\n");
        return;
    }
    if (!is_available(s->id, r.charger, r.date, r.start_hour, r.end_hour)) {
        printf("\nSorry, that slot overlaps an existing booking.\n");
        suggest_alternatives(st_idx, r.date, r.charger, r.start_hour, dur);
        return;
    }

    r.id         = next_res_id++;
    r.user_id    = users[user_idx].id;
    r.station_id = s->id;
    r.status     = STATUS_BOOKED;
    reservations[n_res++] = r;
    save_all();

    printf("\n*** BOOKING CONFIRMED ***\n");
    printf("Booking ID : BK%d\n", r.id);
    printf("Station    : %s, Charger %d (%s)\n", s->name, r.charger, s->type);
    printf("Date/Time  : %s, %02d:00 - %02d:00\n", r.date, r.start_hour, r.end_hour);
}

/* FR7 - list this user's bookings */
void view_my_bookings(int user_id)
{
    int i, count = 0;
    print_header("MY BOOKINGS");
    for (i = 0; i < n_res; i++) {
        if (reservations[i].user_id == user_id) {
            print_booking_row(&reservations[i]);
            count++;
        }
    }
    if (count == 0) printf("You have no bookings yet.\n");
}

/* FR7 - cancel a booking (user_id 0 = administrator, any booking) */
void cancel_booking(int user_id)
{
    int i, count = 0, id;

    print_header(user_id ? "CANCEL MY BOOKING" : "REMOVE A BOOKING");
    for (i = 0; i < n_res; i++) {
        if (reservations[i].status == STATUS_BOOKED &&
            (user_id == 0 || reservations[i].user_id == user_id)) {
            print_booking_row(&reservations[i]);
            count++;
        }
    }
    if (count == 0) { printf("No active bookings to cancel.\n"); return; }

    id = read_int("\nEnter booking number to cancel (e.g. 1001 for BK1001, 0 = back): ", 0, 999999);
    if (id == 0) return;

    for (i = 0; i < n_res; i++) {
        if (reservations[i].id == id && reservations[i].status == STATUS_BOOKED &&
            (user_id == 0 || reservations[i].user_id == user_id)) {
            reservations[i].status = STATUS_CANCELLED;
            save_all();
            printf("Booking BK%d cancelled. The slot is available again.\n", id);
            return;
        }
    }
    printf("No active booking found with that number.\n");
}
