#include <string.h>

#include "esp_wifi.h"

#include "../include/nvs.h"
#include "../private/nvs_private.h"

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

static bool get_wifi_ssid(char *);
static bool get_wifi_pass(char *);
static bool get_wifi_bssid_set(int8_t *);
static bool get_wifi_bssid(char *);
static bool set_wifi_ssid(char *);
static bool set_wifi_pass(char *);
static bool set_wifi_bssid_set(int8_t);
static bool set_wifi_bssid(char *);

bool nvs__set_wifi_config(wifi_config_t *config) {
  char ssid[SSID_LEN + 1] = {0};
  memcpy(ssid, config->sta.ssid, sizeof(config->sta.ssid));
  if (!set_wifi_ssid(ssid))
    return false;

  char pass[PASS_LEN + 1] = {0};
  memcpy(pass, config->sta.password, sizeof(config->sta.password));
  if (!set_wifi_pass(pass))
    return false;

  if (!set_wifi_bssid_set((int8_t)config->sta.bssid_set))
    return false;

  if (config->sta.bssid_set) {
    char bssid[BSSID_LEN + 1] = {0};
    memcpy(bssid, config->sta.bssid, sizeof(config->sta.bssid));
    if (!set_wifi_bssid(bssid))
      return false;
  }

  return true;
}

bool nvs__get_wifi_config(wifi_config_t *config) {
  bzero(config, sizeof(wifi_config_t));

  char ssid[SSID_LEN + 1];
  if (get_wifi_ssid(ssid))
    memcpy(config->sta.ssid, ssid, SSID_LEN);
  else
    return false;

  char pw[PASS_LEN + 1];
  if (get_wifi_pass(pw))
    memcpy(config->sta.password, pw, PASS_LEN);
  else
    return false;

  int8_t bs;
  if (get_wifi_bssid_set(&bs)) {
    if (bs > 0) {
      config->sta.bssid_set = true;

      char bssid[BSSID_LEN + 1];
      if (get_wifi_bssid(bssid))
        memcpy(config->sta.bssid, bssid, BSSID_LEN);
      else
        return false;
    }
  } else {
    return false;
  }

  return true;
}

static bool get_wifi_ssid(char *ssid) {
  return read_string(WIFI_NS, WIFI_SSID, ssid);
}

static bool get_wifi_pass(char *pass) {
  return read_string(WIFI_NS, WIFI_PW, pass);
}

static bool get_wifi_bssid_set(int8_t *bssid_set) {
  return read_int8(WIFI_NS, WIFI_BSSID_SET, bssid_set);
}

static bool get_wifi_bssid(char *bssid) {
  return read_string(WIFI_NS, WIFI_BSSID, bssid);
}

static bool set_wifi_ssid(char *value) {
  return set_string(WIFI_NS, WIFI_SSID, value);
}

static bool set_wifi_pass(char *value) {
  return set_string(WIFI_NS, WIFI_PW, value);
}

static bool set_wifi_bssid_set(int8_t value) {
  return set_int8(WIFI_NS, WIFI_BSSID_SET, value);
}

static bool set_wifi_bssid(char *value) {
  return set_string(WIFI_NS, WIFI_BSSID, value);
}