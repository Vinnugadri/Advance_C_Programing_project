# EV Charging Slot Reservation System (Pure C)

**Advanced C Programming – Console-Based Mini Project**
Team: Vishnu Gadri (24BCS11970), Hitesh (24BCS12743)

A menu-driven console application written only in C (C99). Users register, view
charging-slot availability, book and cancel slots; the administrator manages stations
and bookings. All data is stored permanently in text files.

## Build and Run

Linux / macOS:
```
make
./ev_charging
```
Windows (MinGW):
```
gcc -std=c99 -Iinclude src/*.c -o ev_charging.exe
ev_charging.exe
```
Run the program from the project folder so it finds the `data/` directory.

**Default admin login:** username `admin`, password `admin123` (changeable from the admin menu).
Three demo stations are created automatically on first run.

## Project Structure

```
EV_Charging_Slot_Reservation/
├── include/          header files (one per module)
├── src/
│   ├── main.c         main menu and user menu
│   ├── user.c         User Management        – register, login, change password
│   ├── station.c      Station & Slot Mgmt    – list/add/edit stations
│   ├── reservation.c  Reservation + Cancellation – availability, booking, cancel
│   ├── admin.c        Administration & Reports   – all bookings, users, summary
│   ├── storage.c      File Handling          – load/save all records
│   └── utils.c        Input Validation       – safe input, dates, password hiding
├── data/             text files created at run time
├── Makefile
└── README.md
```

## How It Meets the Phase 1 Requirements

| ID  | Requirement                              | Where implemented                      |
|-----|------------------------------------------|----------------------------------------|
| FR1 | Register (name, phone, vehicle, password)| `user.c: register_user`                |
| FR2 | Authenticate users and admin             | `user.c: login_user`, `admin.c: admin_login` |
| FR3 | Show stations and slot availability      | `station.c: list_stations`, `reservation.c: show_availability` |
| FR4 | Reserve a slot for a time period         | `reservation.c: book_slot`             |
| FR5 | Reject overlapping bookings              | `reservation.c: is_available` (start1 < end2 AND start2 < end1) |
| FR6 | Unique booking ID                        | `next_res_id` → `BK1001, BK1002, ...`  |
| FR7 | View / cancel own reservations           | `view_my_bookings`, `cancel_booking`   |
| FR8 | Admin add/edit/view stations, bookings   | `station.c`, `admin.c`                 |
| FR9 | Save and reload records                  | `storage.c: save_all / load_all`       |

Non-functional: input validation on every entry (`utils.c`), hidden password typing,
passwords stored only as hashes, users can only see/cancel their own bookings,
modular small functions.

## C Concepts Used

Structures (`User`, `Station`, `Reservation`), arrays of structures, functions and
header files, `switch`/`if`/loops for menus, string handling (`strcmp`, `strncpy`,
`strtol`, ...), file handling (`fopen`, `fgets`, `fprintf`), `time.h` for today's
date, `termios` / `conio.h` for hidden password input, and a Makefile for building.

## Data Files (`data/`, pipe-separated text)

| File               | Fields                                                     |
|--------------------|------------------------------------------------------------|
| users.txt          | id \| name \| phone \| vehicle \| password-hash             |
| stations.txt       | id \| name \| location \| chargers \| type \| open \| close  |
| reservations.txt   | id \| user \| station \| charger \| date \| start \| end \| status (1 booked, 0 cancelled) |
| admin.txt          | admin password hash                                        |

## Booking Rules

* Time is divided into one-hour slots; a booking lasts 1 to 4 hours.
* Booking must fit inside the station's opening hours and cannot be in the past.
* If a clash occurs, the program suggests other free chargers/times.
* Cancelling marks the booking as cancelled, which frees the slot again.
* Station edits are refused if they would invalidate an active booking.

## Sample Session

1. Choose **2** to register → note your User ID (e.g. 1).
2. Choose **1** to log in → **3** Book a charging slot → pick station, date, charger, start hour, duration.
3. Try booking the same charger/time again → rejected with alternatives.
4. **4** View my bookings, **5** Cancel a booking.
5. Log in as admin → view all bookings, add stations, see the summary report.

## Possible Future Extensions

Payment/billing, modify-booking option, GUI, network access, SQLite instead of text files.
