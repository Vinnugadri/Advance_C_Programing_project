#ifndef RESERVATION_H
#define RESERVATION_H
#include "common.h"
int  is_available(int station_id, int charger, const char *date, int start, int end);
void show_availability(int st_idx, const char *date);
void book_slot(int user_idx);
void view_my_bookings(int user_id);
void cancel_booking(int user_id);          /* user_id = 0 means admin (any booking) */
void print_booking_row(const Reservation *r);
#endif
