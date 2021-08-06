// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012-2019, The Linux Foundation. All rights reserved.
 * Copyright (C) 2021 Diep Quynh <remilia.1505@gmail.com>.
 */

#include "sched.h"
#include "tune.h"

/*
 * Scheduler boost is a mechanism to temporarily place tasks on CPUs
 * with higher capacity than those where a task would have normally
 * ended up with their load characteristics. Any entity enabling
 * boost is responsible for disabling it as well.
 */

unsigned int sysctl_sched_boost; /* To/from userspace */
unsigned int sched_boost_val;	 /* currently activated boost value */

static DEFINE_MUTEX(boost_mutex);

/*
 * Scheduler boost type and boost policy might at first seem unrelated,
 * however, there exists a connection between them that will allow us
 * to use them interchangeably during placement decisions. We'll explain
 * the connection here in one possible way so that the implications are
 * clear when looking at placement policies.
 */
static bool verify_boost_params(int type)
{
	return type >= RESTRAINED_BOOST_DISABLE && type <= RESTRAINED_BOOST;
}

static void sched_no_boost_nop(void)
{
}

static void sched_full_throttle_boost_enter(void)
{
	disable_energy_aware(true);
}

static void sched_full_throttle_boost_exit(void)
{
	disable_energy_aware(false);
}

static void sched_conservative_boost_enter(void)
{
	struct schedtune *st = schedtune_get("top-app");

	if (unlikely(!st))
		return;

	st->boost_override = 1;
}

static void sched_conservative_boost_exit(void)
{
	struct schedtune *st = schedtune_get("top-app");

	if (unlikely(!st))
		return;

	st->boost_override = 0;
}

static void sched_restrained_boost_enter(void)
{
	struct schedtune *st = schedtune_get("top-app");

	if (unlikely(!st))
		return;

	st->boost_override = 30;
}

static void sched_restrained_boost_exit(void)
{
	struct schedtune *st = schedtune_get("top-app");

	if (unlikely(!st))
		return;

	st->boost_override = 0;
}

struct sched_boost_data {
	int refcount;
	void (*enter)(void);
	void (*exit)(void);
};

static struct sched_boost_data sched_boosts[] = {
	[NO_BOOST] = {
		.refcount = 0,
		.enter = sched_no_boost_nop,
		.exit = sched_no_boost_nop,
	},
	[FULL_THROTTLE_BOOST] = {
		.refcount = 0,
		.enter = sched_full_throttle_boost_enter,
		.exit = sched_full_throttle_boost_exit,
	},
	[CONSERVATIVE_BOOST] = {
		.refcount = 0,
		.enter = sched_conservative_boost_enter,
		.exit = sched_conservative_boost_exit,
	},
	[RESTRAINED_BOOST] = {
		.refcount = 0,
		.enter = sched_restrained_boost_enter,
		.exit = sched_restrained_boost_exit,
	},
};

#define SCHED_BOOST_START FULL_THROTTLE_BOOST
#define SCHED_BOOST_END (RESTRAINED_BOOST + 1)

static int sched_effective_boost(void)
{
	int i;

	/*
	 * The boosts are sorted in descending order by
	 * priority.
	 */
	for (i = SCHED_BOOST_START; i < SCHED_BOOST_END; i++) {
		if (sched_boosts[i].refcount >= 1)
			return i;
	}

	return NO_BOOST;
}

static void sched_boost_disable(int type)
{
	struct sched_boost_data *sb = &sched_boosts[type];
	int next_boost;

	if (sb->refcount <= 0)
		return;

	sb->refcount--;

	if (sb->refcount)
		return;

	/*
	 * This boost's refcount becomes zero, so it must
	 * be disabled. Disable it first and then apply
	 * the next boost.
	 */
	sb->exit();

	next_boost = sched_effective_boost();
	sched_boosts[next_boost].enter();
}

static void sched_boost_enable(int type)
{
	struct sched_boost_data *sb = &sched_boosts[type];
	int next_boost, prev_boost = sched_boost_val;

	sb->refcount++;

	if (sb->refcount != 1)
		return;

	/*
	 * This boost enable request did not come before.
	 * Take this new request and find the next boost
	 * by aggregating all the enabled boosts. If there
	 * is a change, disable the previous boost and enable
	 * the next boost.
	 */

	next_boost = sched_effective_boost();
	if (next_boost == prev_boost)
		return;

	sched_boosts[prev_boost].exit();
	sched_boosts[next_boost].enter();
}

static void sched_boost_disable_all(void)
{
	int i;

	for (i = SCHED_BOOST_START; i < SCHED_BOOST_END; i++) {
		if (sched_boosts[i].refcount > 0) {
			sched_boosts[i].exit();
			sched_boosts[i].refcount = 0;
		}
	}
}

static void _sched_set_boost(int type)
{
	if (type == 0)
		sched_boost_disable_all();
	else if (type > 0)
		sched_boost_enable(type);
	else
		sched_boost_disable(-type);

	/*
	 * sysctl_sched_boost holds the boost request from
	 * user space which could be different from the
	 * effectively enabled boost. Update the effective
	 * boost here.
	 */

	sched_boost_val = sched_effective_boost();
	sysctl_sched_boost = sched_boost_val;
}

int sched_set_boost(int type)
{
	int ret = 0;

	mutex_lock(&boost_mutex);
	if (verify_boost_params(type))
		_sched_set_boost(type);
	else
		ret = -EINVAL;
	mutex_unlock(&boost_mutex);
	return ret;
}

int sched_boost_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos)
{
	int ret;
	unsigned int *data = (unsigned int *)table->data;

	mutex_lock(&boost_mutex);

	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		goto done;

	if (verify_boost_params(*data))
		_sched_set_boost(*data);
	else
		ret = -EINVAL;

done:
	mutex_unlock(&boost_mutex);
	return ret;
}
