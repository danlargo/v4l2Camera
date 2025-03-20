SUBDIRS := source v4l2cam walkMP4 decodeMP4 wrapH264

.PHONY: all clean $(SUBDIRS)

# Default target
all: $(SUBDIRS)

# Invoke make in each subdirectory
$(SUBDIRS):
	$(MAKE) -C $@

# Clean target
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done