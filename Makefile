
CC=gcc


sources:=$(wildcard *.c) $(wildcard *.cpp)

objects:=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(sources)))


dependence:=$(objects:.o=.d)

ssa: $(objects)
	$(CC) $(CPPFLAGS) $^ -o $@
#	@./$@	

%.o: %.c
	$(CC) $(CPPFLAGS) -c $< -o $@


%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

define gen_dep
set -e; rm -f $@; \
$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
rm -f $@.$$$$
endef


%.d: %.c
	$(gen_dep)

%.d: %.cpp
	$(gen_dep)


.PHONY: clean
clean:	
	rm -f all $(objects) $(dependence) ssa

echo:
	@echo sources=$(sources)
	@echo objects=$(objects)
	@echo dependence=$(dependence)
	@echo CPPFLAGS=$(CPPFLAGS)
