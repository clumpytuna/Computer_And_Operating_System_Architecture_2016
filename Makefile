#
# Файл для сборки всего.
#

TASKS= task_rna_coder task_wc_sort short_tasks

all: $(TASKS)
	for TASK in $^; do $(MAKE) -C $$TASK; done

.PHONY: $(TASKS)

clean:
	for TASK in $(TASKS); do $(MAKE) -C $$TASK clean; done




