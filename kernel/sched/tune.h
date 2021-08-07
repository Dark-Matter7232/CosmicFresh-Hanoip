
/* SchdTune tunables for a group of tasks */
struct schedtune {
	/* SchedTune CGroup subsystem */
	struct cgroup_subsys_state css;

	/* Boost group allocated ID */
	int idx;

	/* Boost value for tasks on that SchedTune CGroup */
	int boost;

#ifdef CONFIG_SCHED_WALT
	/* Toggle ability to override sched boost enabled */
	bool sched_boost_no_override;

	/*
	 * Controls whether a cgroup is eligible for sched boost or not. This
	 * can temporariliy be disabled by the kernel based on the no_override
	 * flag above.
	 */
	bool sched_boost_enabled;

	/*
	 * Controls whether tasks of this cgroup should be colocated with each
	 * other and tasks of other cgroups that have the same flag turned on.
	 */
	bool colocate;

	/* Controls whether further updates are allowed to the colocate flag */
	bool colocate_update_disabled;
#endif /* CONFIG_SCHED_WALT */

	/* Hint to bias scheduling of tasks on that SchedTune CGroup
	 * towards idle CPUs */
	int prefer_idle;
};

#ifdef CONFIG_SCHED_TUNE

#include <linux/reciprocal_div.h>

/*
 * System energy normalization constants
 */
struct target_nrg {
	unsigned long min_power;
	unsigned long max_power;
	struct reciprocal_value rdiv;
};

int schedtune_cpu_boost(int cpu);
int schedtune_task_boost(struct task_struct *tsk);

int schedtune_prefer_idle(struct task_struct *tsk);

void schedtune_enqueue_task(struct task_struct *p, int cpu);
void schedtune_dequeue_task(struct task_struct *p, int cpu);

struct schedtune *schedtune_get(char *st_name);

#else /* CONFIG_SCHED_TUNE */

#define schedtune_cpu_boost(cpu)  0
#define schedtune_task_boost(tsk) 0

#define schedtune_prefer_idle(tsk) 0

#define schedtune_enqueue_task(task, cpu) do { } while (0)
#define schedtune_dequeue_task(task, cpu) do { } while (0)

static inline struct schedtune *schedtune_get(char *st_name) { return NULL; }

#endif /* CONFIG_SCHED_TUNE */
