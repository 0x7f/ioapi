#define WBY_IMPLEMENTATION
#define WBY_STATIC
#include "web.h"
#include "wiringPi.h"
#include "wiringPiI2C.h"

int i2c_get(struct wby_con *con)
{
  int len, device, value;
  char param[100];
  struct wby_header headers[1];

  len = wby_find_query_var(con->request.query_params, "device", param, WBY_LEN(param));
  if (len == -1)
  {
    wby_response_begin(con, 400, 0, NULL, 0);
    wby_response_end(con);
    return 0;
  }
  param[len] = '\0';

  device = strtol(param, NULL, 16);
  value = wiringPiI2CRead(device);

  headers[0].name = "Content-Type";
  headers[0].value = "application/json";

  wby_response_begin(con, 200, -1, headers, WBY_LEN(headers));
  wby_printf(con, "{\"value\":\"0x%X\"}", value);
  wby_response_end(con);
  return 0;
}

int i2c_set(struct wby_con *con)
{
  int len, res, device, value;
  char param[100];

  len = wby_find_query_var(con->request.query_params, "device", param, WBY_LEN(param));
  if (len == -1)
  {
    wby_response_begin(con, 400, 0, NULL, 0);
    wby_response_end(con);
    return 0;
  }
  param[len] = '\0';
  device = strtol(param, NULL, 16);

  len = wby_find_query_var(con->request.query_params, "value", param, WBY_LEN(param));
  if (len == -1)
  {
    wby_response_begin(con, 400, 0, NULL, 0);
    wby_response_end(con);
    return 0;
  }
  param[len] = '\0';
  value = strtol(param, NULL, 16);

  res = wiringPiI2CWrite(device, value);

  wby_response_begin(con, res == 0 ? 204 : 500, 0, NULL, 0);
  wby_response_end(con);
  return 0;
}

int dispatch(struct wby_con *con, void *user_data)
{
  if (strcmp(con->request.uri, "/i2c-get") == 0)
  {
    return i2c_get(con);
  }

  if (strcmp(con->request.uri, "/i2c-set") == 0)
  {
    return i2c_set(con);
  }

  return 1;
}

int main(int argc, const char *argv[])
{
  wiringPiSetup();

  /* setup config */
  struct wby_config config;
  memset(&config, 0, sizeof(config));
  config.address = "0.0.0.0";
  config.port = 8080;
  config.connection_max = 8;
  config.request_buffer_size = 2048;
  config.io_buffer_size = 8192;
  config.dispatch = dispatch;

  /* compute and allocate needed memory and start server */
  struct wby_server server;
  size_t needed_memory;
  wby_init(&server, &config, &needed_memory);
  void *memory = calloc(needed_memory, 1);
  wby_start(&server, memory);
  while (1)
  {
    wby_update(&server);
    usleep(100);
  }
  wby_stop(&server);
  free(memory);
}
