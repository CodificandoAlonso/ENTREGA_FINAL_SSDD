//
// Created by hector-pc on 12/04/25.
//

#ifndef CONTROL_DDBB_H
#define CONTROL_DDBB_H

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
int connect_client(char * username, int port_num, int ip_addr);

/**
 * @brief
 */
int disconnect(char *username);


/**
 * @brief
 */
int publish(char *name, char *user, char *path, char *description);

/**
 * @brief
 */
int delete(char *path);

/**
 * @brief
 */
int list_users(char *user);

/**
 * @brief
 */
int list_content(char *user);

#endif //CONTROL_DDBB_H
