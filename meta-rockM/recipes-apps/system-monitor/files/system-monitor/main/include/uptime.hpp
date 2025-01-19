#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>

int64_t get_uptime()
{
	struct sysinfo s_info;
    int _error = sysinfo(&s_info);
    return s_info.uptime;
}
