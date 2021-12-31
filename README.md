# libhotel

hotel's canonical library for PROS. the only things in it are extremely stupid experiments.

## features

- [some kind of PID controller](include/hotel/pid.hpp)
- [coroutine generator class](include/hotel/coro/generator.hpp)
- more coming soon? don't hold your breath!

## usage

at the time of writing, this header-only library depends on at least one feature i haven't yet merged into purduesigbots/pros.
namely, that the code in this library only compiles with c++20, and (again, at the time of writing) there is no way for 
users to easily change the standard their PROS projects compile with. yes, you can change it in common.mk, but this gets
rewritten any time you upgrade the kernel because it's considered a "system" file.
i'm not sure why we wrote the Makefile this way because it's dead simple to fix. in any case, if your project's Makefile
doesn't have

```makefile
C_STANDARD=
CXX_STANDARD=
```

around line 18, then you have to check common.mk for

```makefile
ifeq ($(C_STANDARD),)
	C_STANDARD:=gnu11
endif
ifeq ($(CXX_STANDARD),)
	CXX_STANDARD:=gnu++17
endif
```

around line 10 and

```makefile
CFLAGS=$(MFLAGS) $(CPPFLAGS) $(WARNFLAGS) $(GCCFLAGS) --std=$(C_STANDARD)
CXXFLAGS=$(MFLAGS) $(CPPFLAGS) $(WARNFLAGS) $(GCCFLAGS) --std=$(CXX_STANDARD)
```

around line 34. if your common.mk doesn't have those, you can go ahead and add them (replacing lines 34 and 35 with the
above), but be warned that you'll have to redo the changes every time you upgrade the kernel until i get around to making
a pull request to add those lines to the default template.

if you do have those lines in common.mk but not the ones in Makefile, then you can just add the above lines to Makefile.
in any case, once you've modified your common.mk (or not) go back to the Makefile and set

```makefile
EXTRA_CXXFLAGS=-fcoroutines

CXX_STANDARD=gnu++20
```

and you should be good to start including my garbage library headers in your code.

## something else to note

at the time of writing, clang doesn't really have support for coroutines. this means that your code will compile
(provided you followed the above instructions) because we use a version of g++, but if you were to look at the code in
[generator.hpp](include/hotel/coro/generator.hpp) you would find it riddled with errors because we (and basically
everyone else these days) use clang for static analysis. in theory it might work if you replaced all instances of
`-fcoroutines` with `-fcoroutines-ts` in your `compile_commands.json` (clang only supports the technical specification
and not the full standard at the time of writing) but i haven't gotten this to work at all and it's also tedious to do
every time the CDB is regenerated.