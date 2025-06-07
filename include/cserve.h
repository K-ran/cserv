#ifndef CSERVE_H
#define CSERVE_H

/**
 * cserve.h
 *
 * Created by Karan Purohit on 10/10/25.
 *
 */

/**
 * @brief Initialize the server
 *
 * @param port Port number to listen on
 * @return 0 on success, negative value on error
 */
int cserve_init(int port, const char *directory);
int cserve_start();
#endif