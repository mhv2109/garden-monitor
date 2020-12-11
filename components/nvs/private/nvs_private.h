#ifndef NVS_PRIVATE_H
#define NVS_PRIVATE_H

char *read_string(const char*, const char*);
void set_string(const char*, const char*, char*);
int8_t read_int8(const char*, const char*);
void set_int8(const char*, const char*, int8_t);

#endif