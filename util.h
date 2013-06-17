#ifndef INCLUDE_UTIL_H
#define INCLUDE_UTIL_H 1

typedef int (*cmd_func_t)(void *ctx, int cmd_id);
typedef int (*cmd_set_func_t)(void *ctx, int cmd_id, int on);

extern int wait_cmd(void *ctx, int cmd, int set_value, int get_value,
		    unsigned poll_us, unsigned timeout,
		    cmd_func_t read, cmd_func_t get, cmd_set_func_t set);

#endif /* INCLUDE_UTIL_H */
