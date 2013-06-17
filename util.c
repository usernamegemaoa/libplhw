#include "util.h"
#include <assert.h>
#include <unistd.h>

#define LOG_TAG "util"
#include "log.h"

int wait_cmd(void *ctx, int cmd, int set_value, int get_value,
	     unsigned poll_us, unsigned timeout,
	     cmd_func_t read, cmd_func_t get, cmd_set_func_t set)
{
	int stat;
	int cmd_res;

	assert(read != NULL);
	assert(get != NULL);
	assert(set != NULL);

	stat = set(ctx, cmd, set_value);

	if (stat) {
		LOG("failed to set cmd %i to %i", cmd, set_value);
		return -1;
	}

	do {
		usleep(poll_us);
		stat = read(ctx, cmd);

		if (stat) {
			LOG("failed to read cmd");
			return -1;
		}

		cmd_res = get(ctx, cmd);

		if (timeout > poll_us) {
			timeout -= poll_us;
		} else if (cmd_res != get_value) {
			LOG("timeout while waiting for cmd %i to be %i",
			    cmd, get_value);
			return -1;
		}
	} while (cmd_res != get_value);

	return 0;
}
