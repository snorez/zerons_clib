#include "../include/clib_print.h"

static LIST_HEAD(print_head);
static lock_t print_head_lock;
static int use_std = 1;

void mt_print_init(void)
{
	mutex_lock(&print_head_lock);
	if (!use_std)
		return;
	initscr();
	clear();
	refresh();
	use_std = 0;
	mutex_unlock(&print_head_lock);
}

/* this is called by thread */
void mt_print_add(void)
{
	mutex_lock(&print_head_lock);
	struct clib_print *t;
	list_for_each_entry(t, &print_head, sibling) {
		if (pthread_equal(t->threadid, pthread_self()))
			goto unlock;
	}
	t = malloc(sizeof(*t));
	if (!t) {
		err_dbg(0, err_fmt("malloc err"));
		goto unlock;
	}
	memset(t, 0, sizeof(*t));
	t->threadid = pthread_self();
	list_add_tail(&t->sibling, &print_head);

unlock:
	mutex_unlock(&print_head_lock);
	return;
}

void mt_print_del(void)
{
	mutex_lock(&print_head_lock);
	struct clib_print *t, *next;
	list_for_each_entry_safe(t, next, &print_head, sibling) {
		if (pthread_equal(t->threadid, pthread_self())) {
			list_del(&t->sibling);
			free(t);
			goto unlock;
		}
	}

unlock:
	mutex_unlock(&print_head_lock);
}

void mt_print(pthread_t id, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int found = 0;

	mutex_lock(&print_head_lock);
	struct clib_print *t;

	if (!use_std) {
		clear();
		move(0, 0);
	}

	list_for_each_entry(t, &print_head, sibling) {
		if (pthread_equal(t->threadid, id)) {
			found = 1;
			memset(t->data, 0, sizeof(t->data));
			vsnprintf(t->data+strlen(t->data),
				 sizeof(t->data)-strlen(t->data)-2, fmt, ap);

			if (use_std) {
				fprintf(stdout, "%s", t->data);
				fflush(stdout);
				break;
			}
		}
		if (!use_std) {
			addstr(t->data);
			refresh();
			int line, col;
			getyx(curscr, line, col);
			if (line >= (LINES-1)) {
				move(LINES-1, 0);
				for (int i = 0; i < col; i++)
					addch('.');
				refresh();
				break;
			}
			/* do not assume output contain a \n at last */
			if (col)
				move(line+1, 0);
		}
	}

	if ((!found) && use_std) {
		vfprintf(stdout, fmt, ap);
		fflush(stdout);
	}

	mutex_unlock(&print_head_lock);
	va_end(ap);
	return;
}

void mt_print_fini(void)
{
	mutex_lock(&print_head_lock);
	endwin();
	use_std = 1;
	mutex_unlock(&print_head_lock);
}

static __thread int show_process_byte = 0;
void mt_print_progress(double cur, double total)
{
	if (show_process_byte)
		for (int i = 0; i < show_process_byte; i++)
			fprintf(stdout, "\b");
	show_process_byte = fprintf(stdout, "%.3f%%", cur * 100 / total);
	fflush(stdout);
	if (cur == total)
		show_process_byte = 0;
}