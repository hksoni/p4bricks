#if COND_NANOMSG
#MAYBE_TESTS = tests
#endif

#SUBDIRS = . $(MAYBE_TESTS)

THRIFT_IDL = $(srcdir)/thrift/linker_switch.thrift

noinst_LTLIBRARIES = liblinkerswitch.la

liblinkerswitch_la_SOURCES = linker_switch.cpp linker_switch.h 

liblinkerswitch_la_LIBADD = \
$(top_builddir)/src/bm_sim/libbmsim.la \
$(top_builddir)/src/bf_lpm_trie/libbflpmtrie.la \
$(top_builddir)/src/BMI/libbmi.la \
$(top_builddir)/third_party/jsoncpp/libjson.la \
-lboost_system $(THRIFT_LIB) -lboost_program_options

if COND_THRIFT

linker_switch_thrift_py_files = \
gen-py/lswitch_runtime/constants.py \
gen-py/lswitch_runtime/__init__.py \
gen-py/lswitch_runtime/LinkerSwitch.py \
gen-py/lswitch_runtime/ttypes.py
linker_switch_thrift_files = \
gen-cpp/bm/linker_switch_constants.cpp \
gen-cpp/bm/linker_switch_constants.h \
gen-cpp/bm/LinkerSwitch.cpp \
gen-cpp/bm/LinkerSwitch.h \
gen-cpp/bm/linker_switch_types.cpp \
gen-cpp/bm/linker_switch_types.h

BUILT_SOURCES = $(linker_switch_thrift_files) $(linker_switch_thrift_py_files)

lswitchpydir = $(pythondir)/lswitch_runtime
nodist_lswitchpy_PYTHON = $(linker_switch_thrift_py_files)

python_PYTHON = lswitch_CLI.py

# See
# http://www.gnu.org/software/autoconf/manual/autoconf-2.69/html_node/Installation-Directory-Variables.html
edit = sed \
	-e 's|@pythondir[@]|$(pythondir)|g'

linker_switch_CLI: Makefile
	rm -f $@ $@.tmp
	$(edit) $(srcdir)/$@.in >$@.tmp
	chmod +x $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

linker_switch_CLI: linker_switch_CLI.in

EXTRA_DIST = \
linker_switch_CLI.in \
lswitch_CLI

bin_SCRIPTS = \
linker_switch_CLI

AM_CPPFLAGS += \
-I$(srcdir)/thrift/src/ \
-I$(builddir)/gen-cpp

liblinkerswitch_la_SOURCES += thrift/src/LinkerSwitch_server.cpp

bin_PROGRAMS = simple_linker

simple_linker_LDADD = \
$(top_builddir)/src/bm_runtime/libbmruntime.la \
$(top_builddir)/thrift_src/libruntimestubs.la \
liblinkerswitch.la \
liblinkerswitch_thrift.la

simple_linker_SOURCES = simple_linker.cpp  primitives.cpp

lib_LTLIBRARIES = liblinkerswitch_thrift.la

nodist_liblinkerswitch_thrift_la_SOURCES = \
$(linker_switch_thrift_files)

# See http://www.gnu.org/software/automake/manual/html_node/Multiple-Outputs.html

EXTRA_DIST += $(THRIFT_IDL)

# We copy the generated Python code to srcdir/ if we have permissions (i.e. not
# for 'make distcheck'). This is to ensure we can run the CLI easily. Maybe I
# will think of a better solution later.
thrift_files.ts: $(THRIFT_IDL)
	@rm -f thrift_files.tmp
	@touch thrift_files.tmp
	@mkdir -p $(builddir)/gen-cpp/bm
	$(THRIFT) -out $(builddir)/gen-cpp/bm --gen cpp -r $(THRIFT_IDL)
	$(THRIFT) -o $(builddir) --gen py -r $(THRIFT_IDL)
	if mkdir $(srcdir)/lswitch_runtime.test 2>/dev/null; then \
	  rm -rf $(srcdir)/lswitch_runtime/; \
	  cp -r $(builddir)/gen-py/lswitch_runtime/ $(srcdir)/; \
	  rm -rf $(srcdir)/lswitch_runtime.test; else :; \
        fi
	@mv -f thrift_files.tmp $@

$(BUILT_SOURCES): thrift_files.ts
## Recover from the removal of $@
	@if test -f $@; then :; else \
	  trap 'rm -rf thrift_files.lock thrift_files.ts' 1 2 13 15; \
## mkdir is a portable test-and-set
	if mkdir thrift_files.lock 2>/dev/null; then \
## This code is being executed by the first process.
	  rm -f thrift_files.ts; \
	  $(MAKE) $(AM_MAKEFLAGS) thrift_files.ts; \
	  result=$$?; rm -rf thrift_files.lock; exit $$result; \
	else \
## This code is being executed by the follower processes.
## Wait until the first process is done.
	  while test -d thrift_files.lock; do sleep 1; done; \
## Succeed if and only if the first process succeeded.
	    test -f thrift_files.ts; \
	  fi; \
	fi

nodist_pkginclude_HEADERS = \
gen-cpp/bm/linker_switch_constants.h \
gen-cpp/bm/LinkerSwitch.h \
gen-cpp/bm/linker_switch_types.h

CLEANFILES = $(BUILT_SOURCES) \
thrift_files.ts \
gen-cpp/bm/LinkerSwitch_server.skeleton.cpp \
$(bin_SCRIPTS)

# I used to do a rm on gen-cpp, but it was removing a .deps directory, instead I
# am adding the skeleton file to CLEANFILES
clean-local:
	rm -rf gen-py

endif  # COND_THRIFT
