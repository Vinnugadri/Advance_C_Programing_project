/* admin.c - Administration and Reports module */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "admin.h"
#include "station.h"
#include "reservation.h"
#include "user.h"
#include "utils.h"
#include "storage.h"

int admin_login(void)
{
    char name[40], pw[40];
    int tries;

    print_header("ADMINISTRATOR LOGIN");
    read_line("Username : ", name, sizeof name);
    for (tries = 0; tries < 3; tries++) {
        read_password("Password : ", pw, sizeof pw);
        if (strcmp(name, "admin") == 0 && hash_password(pw) == admin_hash) {
            printf("\nWelcome, Administrator.\n");
            return 1;
        }
        printf("  Invalid credentials. (%d attempt(s) left)\n", 2 - tries);
    }
    return 0;
}

static void view_all_bookings(void)
{
    int i;
    print_header("ALL BOOKINGS");
    if (n_res == 0) { printf("No bookings recorded.\n"); return; }
    for (i = 0; i < n_res; i++) {
        int ui = find_user_by_id(reservations[i].user_id);
        printf("[User %d: %s] ", reservations[i].user_id, ui >= 0 ? users[ui].name : "?");
        print_booking_row(&reservations[i]);
    }
}

static void view_users(void)
{
    int i;
    print_header("REGISTERED USERS");
    printf("%-5s %-22s %-12s %s\n", "ID", "Name", "Phone", "Vehicle");
    for (i = 0; i < n_users; i++)
        printf("%-5d %-22s %-12s %s\n", users[i].id, users[i].name,
               users[i].phone, users[i].vehicle);
    if (n_users == 0) printf("No users registered.\n");
}

static void view_slots(void)
{
    char date[DATE_LEN];
    int idx;
    print_header("VIEW SLOTS");
    idx = choose_station();
    if (idx < 0) return;
    read_date("Date (YYYY-MM-DD) : ", date);
    show_availability(idx, date);
}

/* Simple summary report */
static void summary(void)
{
    int i, booked = 0, cancelled = 0, s;
    long hours = 0;

    print_header("SUMMARY REPORT");
    for (i = 0; i < n_res; i++) {
        if (reservations[i].status == STATUS_BOOKED) {
            booked++;
            hours += reservations[i].end_hour - reservations[i].start_hour;
        } else cancelled++;
    }
    printf("Registered users   : %d\n", n_users);
    printf("Charging stations  : %d\n", n_stations);
    printf("Total bookings     : %d\n", n_res);
    printf("  Active           : %d\n", booked);
    printf("  Cancelled        : %d\n", cancelled);
    printf("Active booked hours: %ld\n", hours);

    printf("\nActive bookings per station:\n");
    for (s = 0; s < n_stations; s++) {
        int cnt = 0;
        for (i = 0; i < n_res; i++)
            if (reservations[i].station_id == stations[s].id &&
                reservations[i].status == STATUS_BOOKED) cnt++;
        printf("  %-22s : %d\n", stations[s].name, cnt);
    }
}

static void admin_change_password(void)
{
    char p1[40], p2[40];
    print_header("CHANGE ADMIN PASSWORD");
    read_password("New password (min 4 chr): ", p1, sizeof p1);
    if (strlen(p1) < 4) { printf("Password too short.\n"); return; }
    read_password("Confirm new password: ", p2, sizeof p2);
    if (strcmp(p1, p2) != 0) { printf("Passwords do not match.\n"); return; }
    admin_hash = hash_password(p1);
    save_all();
    printf("Admin password changed.\n");
}

void admin_menu(void)
{
    int ch;
    do {
        print_header("ADMINISTRATOR MENU");
        printf(" 1. View all stations\n"
               " 2. Add station\n"
               " 3. Edit station\n"
               " 4. View slot availability\n"
               " 5. View all bookings\n"
               " 6. Remove (cancel) a booking\n"
               " 7. View registered users\n"
               " 8. Summary report\n"
               " 9. Change admin password\n"
               " 0. Logout\n");
        ch = read_int("Choice: ", 0, 9);
        switch (ch) {
            case 1: list_stations();        break;
            case 2: add_station();          break;
            case 3: edit_station();         break;
            case 4: view_slots();           break;
            case 5: view_all_bookings();    break;
            case 6: cancel_booking(0);      break;
            case 7: view_users();           break;
            case 8: summary();              break;
            case 9: admin_change_password();break;
            default: break;
        }
        if (ch != 0) pause_screen();
    } while (ch != 0);
    printf("Administrator logged out.\n");
}
