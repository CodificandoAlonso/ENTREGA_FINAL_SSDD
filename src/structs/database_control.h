//
// Created by hector-pc on 12/04/25.
//

#ifndef CONTROL_DDBB_H
#define CONTROL_DDBB_H
#include <bits/types.h>

/**
 * @brief
 */
int register_user(char *username);

/**
 * @brief
 */
int unregister_user(char *username);

/**
 *
 */
int connect_client(char * username, __uint32_t port_num, char *ip_addr);

/**
 * @brief
 */
int disconnect(char *username);


/**
 * @brief
 */
int publish(char *user, char *path, char *description);

/**
 * @brief
 */
int delete(char *path, char *username);

/**
 * @brief
 */
int list_users(char *username, char users[2048][256], char ips[2048][256], int ports[2048], int *len);

/**
 * @brief
 */
int list_content(char *user, char *user_content, char users[2048][256], int *len);

#endif //CONTROL_DDBB_H
