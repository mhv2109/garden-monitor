#include <string.h>

#include "esp_wifi.h"

#include "../private/nvs_private.h"
#include "../include/nvs.h"

// Namespace constants
static const char *WIFI_NS = "wifi";

// Key constants
static const char *WIFI_SSID = "ssid";
static const char *WIFI_PW = "password";
static const char *WIFI_BSSID_SET = "bssid_set";
static const char *WIFI_BSSID = "bssid";

// Config constants
#define SSID_LEN 32
#define PASS_LEN 64
#define BSSID_LEN 6 

static char *get_wifi_ssid(void);
static char *get_wifi_pass(void);
static int8_t get_wifi_bssid_set(void);
static char *get_wifi_bssid(void);
static void set_wifi_ssid(char*);
static void set_wifi_pass(char*);
static void set_wifi_bssid_set(int8_t);
static void set_wifi_bssid(char*);

void
nvs__set_wifi_config(wifi_config_t *config)
{
  char ssid[SSID_LEN+1] = {0};
  memcpy(ssid, config->sta.ssid, sizeof(config->sta.ssid));
  set_wifi_ssid(ssid);

  char pass[PASS_LEN+1] = {0};
  memcpy(pass, config->sta.password, sizeof(config->sta.password));
  set_wifi_pass(pass);

  set_wifi_bssid_set((int8_t) config->sta.bssid_set);
  if (config->sta.bssid_set) {
    char bssid[BSSID_LEN+1] = {0};
    memcpy(bssid, config->sta.bssid, sizeof(config->sta.bssid));
    set_wifi_bssid(bssid);
  }
}

wifi_config_t
nvs__get_wifi_config(void)
{
  wifi_config_t config;
  bzero(&config, sizeof(wifi_config_t));

  char *ssid = get_wifi_ssid();
  if (ssid != NULL) {
    memcpy(config.sta.ssid, ssid, SSID_LEN);
  }
  free(ssid);

  char *pw = get_wifi_pass();
  if (pw != NULL) {
    memcpy(config.sta.password, pw, PASS_LEN);
  }
  free(pw);
  
  int8_t bs = get_wifi_bssid_set();
  if (bs > 0) {
    config.sta.bssid_set = true;
    char *bssid = get_wifi_bssid();
    if (bssid != NULL) {
      memcpy(config.sta.bssid, bssid, BSSID_LEN);
    }
    free(bssid);
  }

  return config;
}

static char*
get_wifi_ssid(void)
{
  return read_string(WIFI_NS, WIFI_SSID); 
}

static char*
get_wifi_pass(void)
{
  return read_string(WIFI_NS, WIFI_PW);
}

static int8_t
get_wifi_bssid_set(void)
{
  return read_int8(WIFI_NS, WIFI_BSSID_SET);
}

static char*
get_wifi_bssid(void)
{
  return read_string(WIFI_NS, WIFI_BSSID);
}

static void
set_wifi_ssid(char *value)
{
  set_string(WIFI_NS, WIFI_SSID, value);
}

static void
set_wifi_pass(char *value)
{
  set_string(WIFI_NS, WIFI_PW, value);
}

static void
set_wifi_bssid_set(int8_t value)
{
  set_int8(WIFI_NS, WIFI_BSSID_SET, value);
}

static void
set_wifi_bssid(char *value)
{
  set_string(WIFI_NS, WIFI_BSSID, value);
}