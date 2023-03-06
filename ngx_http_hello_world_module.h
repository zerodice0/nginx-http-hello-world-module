#ifndef NGX_HTTP_HELLO_WORLD_MODULE_H
#define NGX_HTTP_HELLO_WORLD_MODULE_H

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_flag_t enabled;
		ngx_str_t message;
} ngx_http_hello_world_loc_conf_t;

extern ngx_module_t ngx_http_hello_world_module;

#endif /* NGX_HTTP_HELLO_WORLD_MODULE_H */
