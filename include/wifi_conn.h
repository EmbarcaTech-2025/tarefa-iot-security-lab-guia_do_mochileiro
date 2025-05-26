#ifndef WIFI_CONN_H
#define WIFI_CONN_H
void connect_to_wifi(const char *ssid, const char *password);
int wifi_comm_is_connected();
#endif