/* main.c - EV Charging Slot Reservation System
   Entry point and menu navigation (Input Validation and Menu module) */
#include <stdio.h>
#include "common.h"
#include "storage.h"
#include "user.h"
#include "station.h"
#include "reservation.h"
#include "admin.h"
#include "utils.h"

static void user_menu(int idx)
{
    int ch;
    do {
        print_header("USER MENU");
        printf("Logged in as: %s (%s)\n\n", users[idx].name, users[idx].vehicle);
        printf(" 1. View stations\n"
               " 2. View slot availability\n"
               " 3. Book a charging slot\n"
               " 4. View my bookings\n"
               " 5. Cancel a booking\n"
               " 6. Change password\n"
               " 0. Logout\n");
        ch = read_int("Choice: ", 0, 6);
        switch (ch) {
            case 1: list_stations(); break;
            case 2: {
                char date[DATE_LEN];
                int s = choose_station();
                if (s >= 0) {
                    read_date("Date (YYYY-MM-DD) : ", date);
                    show_availability(s, date);
                }
                break;
            }
            case 3: book_slot(idx);                    break;
            case 4: view_my_bookings(users[idx].id);   break;
            case 5: cancel_booking(users[idx].id);     break;
            case 6: change_password(idx);              break;
            default: break;
        }
        if (ch != 0) pause_screen();
    } while (ch != 0);
    printf("Logged out. Thank you!\n");
}

int main(void)
{
    int ch, idx;

    load_all();
    printf("\n  ***********************************************\n"
           "  *   EV CHARGING SLOT RESERVATION SYSTEM       *\n"
           "  *   Console application in C                  *\n"
           "  ***********************************************\n");

    do {
        print_header("MAIN MENU");
        printf(" 1. User login\n"
               " 2. Register as new user\n"
               " 3. Administrator login\n"
               " 0. Exit\n");
        ch = read_int("Choice: ", 0, 3);
        switch (ch) {
            case 1:
                idx = login_user();
                if (idx >= 0) user_menu(idx);
                break;
            case 2: register_user();           break;
            case 3: if (admin_login()) admin_menu(); break;
            default: break;
        }
    } while (ch != 0);

    save_all();
    printf("\nGoodbye! Data saved.\n");
    return 0;
}
