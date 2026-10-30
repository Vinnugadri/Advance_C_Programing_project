/* common.h - shared constants, structures and global data */
#ifndef COMMON_H
#define COMMON_H

#define MAX_USERS     200
#define MAX_STATIONS  20
#define MAX_RES       2000

#define NAME_LEN      50
#define PHONE_LEN     11   /* 10 digits + '\0' */
#define VEHICLE_LEN   16
#define LOC_LEN       60
#define TYPE_LEN      20
#define DATE_LEN      11   /* YYYY-MM-DD + '\0' */

#define MAX_CHARGERS  10
#define MAX_DURATION  4    /* longest single booking, in hours */

#define STATUS_CANCELLED 0
#define STATUS_BOOKED    1

#define USERS_FILE    "data/users.txt"
#define STATIONS_FILE "data/stations.txt"
#define RES_FILE      "data/reservations.txt"
#define ADMIN_FILE    "data/admin.txt"

typedef struct {
    int id;
    char name[NAME_LEN];
    char phone[PHONE_LEN];
    char vehicle[VEHICLE_LEN];
    unsigned long pass_hash;
} User;

typedef struct {
    int id;
    char name[NAME_LEN];
    char location[LOC_LEN];
    int chargers;              /* number of chargers (1..MAX_CHARGERS) */
    char type[TYPE_LEN];       /* e.g. "AC Type2", "DC Fast" */
    int open_hour;             /* first bookable hour (0-23)  */
    int close_hour;            /* closing hour (1-24)         */
} Station;

typedef struct {
    int id;                    /* booking id, shown as BK<id> */
    int user_id;
    int station_id;
    int charger;               /* 1..chargers of the station  */
    char date[DATE_LEN];
    int start_hour;
    int end_hour;
    int status;
} Reservation;

/* ---- global data tables (arrays of structures) ---- */
extern User        users[MAX_USERS];
extern Station     stations[MAX_STATIONS];
extern Reservation reservations[MAX_RES];
extern int n_users, n_stations, n_res;
extern int next_user_id, next_station_id, next_res_id;
extern unsigned long admin_hash;

#endif
