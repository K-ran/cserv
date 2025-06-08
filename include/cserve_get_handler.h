#ifndef CSERVE_GET_HANDLER_H
#define CSERVE_GET_HANDLER_H

/**
 * cserve_get_handler.h
 *
 * Created by Karan Purohit on 10/10/25.
 *
 * GET request handler
 */

#include "cserve_net.h"

cserver_http_res_t *cserve_get_handler(cserver_http_req_t *req, const char *root_dir);

#endif