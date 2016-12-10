#ifndef _LINUX_PID_NS_H
#define _LINUX_PID_NS_H

#include <linux/sched.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/threads.h>
#include <linux/nsproxy.h>
#include <linux/kref.h>

struct pidmap {
       atomic_t nr_free;/*<llj>���ܷ����pid����</llj>*/
       void *page;/*<llj>ָ����pid������ҳ</llj>*/
};

#define PIDMAP_ENTRIES         ((PID_MAX_LIMIT + 8*PAGE_SIZE - 1)/PAGE_SIZE/8)

struct bsd_acct_struct;

struct pid_namespace {
	struct kref kref;
	/*<llj>����pid��λͼ������Ҫ����һ���µ�pidʱֻ�����λͼ��
	 *�ҵ�bitΪ0��λ�ò���1��Ȼ�����ͳ��������nr_free)����ʾ
	 *��pid_namespace��pid�ѷ�������</llj>*/
	struct pidmap pidmap[PIDMAP_ENTRIES];
	int last_pid;/*<llj>����pidmap�ķ��䣬ָ�����һ�������pid��λ��</llj>*/
	/*<llj>ָ�����һ�����̡� �ý��̵������ǵ��ӽ��̽���ʱΪ����ʬ�����տռ䣩��
	*����Ŀǰֻ֧��global namespace������child_reaper��ָ��init_task</llj>*/
	struct task_struct *child_reaper;
	/*<llj>ָ�����pid��slab�ĵ�ַ</llj>*/
	struct kmem_cache *pid_cachep;
	/*<llj>��namespace������һ�㣬 ����������Ȼ��0</llj>*/
	unsigned int level;
	/*<llj>��namespace�ĸ���namespace�� ����һ����NULL</llj>*/
	struct pid_namespace *parent;
#ifdef CONFIG_PROC_FS
	struct vfsmount *proc_mnt;
#endif
#ifdef CONFIG_BSD_PROCESS_ACCT
	struct bsd_acct_struct *bacct;
#endif
	kgid_t pid_gid;
	int hide_pid;
	int reboot;	/* group exit code if this pidns was rebooted */
};

extern struct pid_namespace init_pid_ns;

#ifdef CONFIG_PID_NS
static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	if (ns != &init_pid_ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct pid_namespace *copy_pid_ns(unsigned long flags, struct pid_namespace *ns);
extern void free_pid_ns(struct kref *kref);
extern void zap_pid_ns_processes(struct pid_namespace *pid_ns);
extern int reboot_pid_ns(struct pid_namespace *pid_ns, int cmd);

static inline void put_pid_ns(struct pid_namespace *ns)
{
	if (ns != &init_pid_ns)
		kref_put(&ns->kref, free_pid_ns);
}

#else /* !CONFIG_PID_NS */
#include <linux/err.h>

static inline struct pid_namespace *get_pid_ns(struct pid_namespace *ns)
{
	return ns;
}

static inline struct pid_namespace *
copy_pid_ns(unsigned long flags, struct pid_namespace *ns)
{
	if (flags & CLONE_NEWPID)
		ns = ERR_PTR(-EINVAL);
	return ns;
}

static inline void put_pid_ns(struct pid_namespace *ns)
{
}

static inline void zap_pid_ns_processes(struct pid_namespace *ns)
{
	BUG();
}

static inline int reboot_pid_ns(struct pid_namespace *pid_ns, int cmd)
{
	return 0;
}
#endif /* CONFIG_PID_NS */

extern struct pid_namespace *task_active_pid_ns(struct task_struct *tsk);
void pidhash_init(void);
void pidmap_init(void);

#endif /* _LINUX_PID_NS_H */
