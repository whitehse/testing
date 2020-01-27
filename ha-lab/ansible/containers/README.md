# Commands

chroot
nsenter
unshare
setcap
getcap
capsh

# Syscalls

````
capabilities(7)

int cap_set_flag(cap_t cap_p, cap_flag_t flag, int ncap,
  const cap_value_t *capsint ncap, cap_flag_value_t value);
int cap_set_proc(cap_t cap_p);
int prctl(int option, unsigned long arg2, unsigned long arg3,
                 unsigned long arg4, unsigned long arg5);
int setns(int fd, int nstype);
scmp_filter_ctx seccomp_init(uint32_t def_action);
int seccomp_rule_add(scmp_filter_ctx ctx, uint32_t action,
                 int syscall, unsigned int arg_cnt, ...);
int unshare(int flags);
````
