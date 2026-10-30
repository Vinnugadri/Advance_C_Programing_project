/* user.c - User Management module: register, login, change password */
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "user.h"
#include "utils.h"
#include "storage.h"

int find_user_by_id(int id)
{
    int i;
    for (i = 0; i < n_users; i++)
        if (users[i].id == id) return i;
    return -1;
}

static int vehicle_taken(const char *veh)
{
    int i;
    for (i = 0; i < n_users; i++)
        if (strcmp(users[i].vehicle, veh) == 0) return 1;
    return 0;
}

static int phone_taken(const char *ph)
{
    int i;
    for (i = 0; i < n_users; i++)
        if (strcmp(users[i].phone, ph) == 0) return 1;
    return 0;
}

/* FR1 - register a new user */
void register_user(void)
{
    User u;
    char p1[40], p2[40];

    print_header("USER REGISTRATION");
    if (n_users >= MAX_USERS) {
        printf("User limit reached. Cannot register more users.\n");
        return;
    }

    read_text("Full name            : ", u.name, NAME_LEN);

    for (;;) {
        read_phone("Phone (10 digits)    : ", u.phone);
        if (!phone_taken(u.phone)) break;
        printf("  This phone number is already registered.\n");
    }
    for (;;) {
        read_text("Vehicle number       : ", u.vehicle, VEHICLE_LEN);
        to_upper(u.vehicle);
        if (!vehicle_taken(u.vehicle)) break;
        printf("  This vehicle number is already registered.\n");
    }
    for (;;) {
        read_password("Password (min 4 chr): ", p1, sizeof p1);
        if (strlen(p1) < 4) { printf("  Password too short.\n"); continue; }
        read_password("Confirm password    : ", p2, sizeof p2);
        if (strcmp(p1, p2) != 0) { printf("  Passwords do not match.\n"); continue; }
        break;
    }

    u.id = next_user_id++;
    u.pass_hash = hash_password(p1);
    users[n_users++] = u;
    save_all();
    printf("\nRegistration successful! Your User ID is %d. Use it to log in.\n", u.id);
}

/* FR2 - authenticate a user. Returns index in users[] or -1. */
int login_user(void)
{
    char pw[40];
    int id, idx, tries;

    print_header("USER LOGIN");
    id = read_int("User ID  : ", 1, 999999);
    idx = find_user_by_id(id);

    for (tries = 0; tries < 3; tries++) {
        read_password("Password : ", pw, sizeof pw);
        if (idx >= 0 && users[idx].pass_hash == hash_password(pw)) {
            printf("\nWelcome, %s!\n", users[idx].name);
            return idx;
        }
        printf("  Invalid User ID or password. (%d attempt(s) left)\n", 2 - tries);
    }
    return -1;
}

void change_password(int idx)
{
    char old[40], p1[40], p2[40];

    print_header("CHANGE PASSWORD");
    read_password("Current password : ", old, sizeof old);
    if (users[idx].pass_hash != hash_password(old)) {
        printf("Incorrect current password.\n");
        return;
    }
    read_password("New password (min 4 chr): ", p1, sizeof p1);
    if (strlen(p1) < 4) { printf("Password too short.\n"); return; }
    read_password("Confirm new password: ", p2, sizeof p2);
    if (strcmp(p1, p2) != 0) { printf("Passwords do not match.\n"); return; }
    users[idx].pass_hash = hash_password(p1);
    save_all();
    printf("Password changed successfully.\n");
}
