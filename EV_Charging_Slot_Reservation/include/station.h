#ifndef STATION_H
#define STATION_H
void list_stations(void);
int  choose_station(void);             /* returns index in stations[] or -1 */
int  find_station_by_id(int id);
void add_station(void);
void edit_station(void);
#endif
