#ifndef NVS_PRIVATE_H
#define NVS_PRIVATE_H

bool read_string(const char *, const char *, char *);
bool set_string(const char *, const char *, char *);
bool read_int8(const char *, const char *, int8_t *);
bool set_int8(const char *, const char *, int8_t);

#endif