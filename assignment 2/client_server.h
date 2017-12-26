/*
 * client_server.h
 *
 *  Created on: Oct 26, 2016
 *      Author: Nathan J Thomas
 */

#define FREE 0
#define BUSY 1
#define TICK 1000

extern void *server(void *arg);
extern void *clock1(void *tick);
extern void *client(void *arg);
