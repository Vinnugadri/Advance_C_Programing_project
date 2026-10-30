#ifndef USER_H
#define USER_H
void register_user(void);
int  login_user(void);                 /* returns index in users[] or -1 */
void change_password(int idx);
int  find_user_by_id(int id);          /* index or -1 */
#endif
