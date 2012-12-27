# Copyright Edd Dawson 2012
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

project('dbg')


# Variables
flags = cfgvar('dbg.flags', 'flags', description='list of abstract build flags (see doozer docs)')
platform = cfgvar('dbg.platform', description='"windows" or "osx"')
optimize_out = cfgvar('dbg.optimize_out', default=False, description='True if the library shouldn\'t be built e.g. for release builds where asserts are dummy macros')

if platform == 'windows':
    # Ideally, if symbol lookup is needed, you should get the debugging tools 
    # redistributable package so that a recent build of dbghelp.dll can be 
    # installed alongside your application's executable, as older versions have
    # some bugs. See http://www.gamedev.net/topic/630292-help-with-dbghelpimagehlp/
    #
    # However, if you only use dbg's symbol lookup in local/debug builds the path
    # to dbghelp.dll can be hardcoded in to the dbg static library.
    # To do this, set dbg.dbghelp_dll_path configuration variable to point to it.

    dbghelp_dll_path = cfgvar(
        'dbg.dbghelp_dll_path',
        default=None,
        description='Location of a recent dbghelp.dll to be hardcoded (optional)'
    )

    if dbghelp_dll_path:
        extra_windows_defines = ['DBG_RECENT_DBGHELP_DLL="'+dbghelp_dll_path.replace('\\', '/')+'"']
    else:
        extra_windows_defines = []


test_o_matic_root = cfgvar('test_o_matic.root', description='the path to test_o_matic')
fungo_root = cfgvar('fungo.root', description='the path to fungo')


# Dependencies
fungoproj = subproject(fungo_root/'make.py')
tomproj = subproject(test_o_matic_root/'make.py')



# For tests and demos, create a different set of flags that ensures debug information
# and symbols are maintained in the usual way for the toolchain.
demo_flags = flags[:]
if 'debuggable' not in demo_flags: demo_flags.append('debuggable')
if 'strip-symbols' in demo_flags: demo_flags.remove('strip-symbols')
if 'debug-info-internal' in demo_flags: demo_flags.remove('debug-info-internal')
if 'debug-info-external' in demo_flags: demo_flags.remove('debug-info-external')


def keep_frame_pointers(kit, opt):
    if 'mscl' in kit.installed():
        opt.cppflags += ['/Oy-']
    elif 'gpp' in kit.installed():
        opt.cppflags -= ['-fomit-frame-pointers']

@target
def full_staticlib(kit):
    fungo = fungoproj.staticlib(kit)

    opt = kit.cpp.opt(*flags)
    opt.includes += [here/'include', here/'src', here/'src'/platform] + fungo.includes
    opt.sources += here/'src/*.cpp' + here/'src'/platform/'*.cpp'

    if platform == 'windows':
        if 'mscl' in kit.installed():
            opt.cppflags -= ['/Za'] # <windows.h> :/
            opt.sources += here/'src/windows/msvc/*.cpp'

        elif 'gpp' in kit.installed():
            opt.sources += here/'src/windows/mingw/*.cpp'

            for f in opt.cppflags:
                if f.startswith('-march='): break
            else:
                opt.cppflags.append('-march=pentium4')

            if kit.cpp.compiler.version[0:2] == (4,2):
                # GCC 4.2 emits unnecessary anonymous namespace warnings: 
                # http://gcc.gnu.org/bugzilla/show_bug.cgi?id=29365
                opt.cppflags -= ['-Werror']

        opt.defines += extra_windows_defines

    keep_frame_pointers(kit, opt)

    return properties(
        libs = fungo.libs + [kit.cpp.lib('dbg', opt)],
        includes = [here/'include'] + fungo.includes,
        syslibs = fungo.syslibs
    )

@target
def staticlib(kit):
    if optimize_out:
        # Still need fungo in this case for DBG_THROW().
        fungo = fungoproj.staticlib(kit)

        return properties(
            libs = fungo.libs,
            includes = [here/'include'] + fungo.includes,
            syslibs = fungo.syslibs
        )

    else:
        return full_staticlib(kit)

def make_example(kit, name):
    dbg = full_staticlib(kit)

    opt = kit.cpp.opt(*demo_flags)
    opt.includes += [here/'examples'] + dbg.includes
    opt.sources += [here/'examples'/(name+'.cpp')]
    opt.libs += dbg.libs
    opt.syslibs += dbg.syslibs

    keep_frame_pointers(kit, opt)

    return process([kit.cpp.exe(name, opt)])

@target
def traceback_example(kit): return make_example(kit, 'traceback')

@target
def terminate_example(kit): return make_example(kit, 'terminate')

@target
def assert_example(kit): return make_example(kit, 'assert')

@target
def note_example(kit): return make_example(kit, 'note')

@target
def unusual_example(kit): return make_example(kit, 'unusual')

@target
def throw_example(kit): return make_example(kit, 'throw')

@target
def examples(kit):
    return [
        traceback_example(kit),
        terminate_example(kit),
        assert_example(kit),
        note_example(kit),
        unusual_example(kit),
        throw_example(kit),
    ]

@target
def test_objects(kit):
    dbg = full_staticlib(kit)
    tom = tomproj.staticlib(kit)

    opt = kit.cpp.opt(*demo_flags)
    opt.sources += here/'tests/*.cpp'
    opt.sources.erase(here/'tests/main.cpp')
    opt.includes += dbg.includes + tom.includes

    keep_frame_pointers(kit, opt)

    if 'mscl' in kit.installed():
        opt.cppflags -= ['/Za'] # <windows.h> :/

    if platform == 'windows':
        opt.defines += extra_windows_defines

    return properties(
        objs = kit.cpp.objs(opt),
        libs = dbg.libs + tom.libs,
        syslibs = dbg.syslibs + tom.syslibs
    )

@target
def testlibs(kit):
    testobjs = test_objects(kit)
    tom = tomproj.staticlib(kit)

    opt = kit.cpp.opt(*demo_flags)
    opt.objects += testobjs.objs
    opt.libs += testobjs.libs
    opt.syslibs += testobjs.syslibs

    return [kit.cpp.shlib('dbg_unittests', opt)]
    
@target
def tests(kit):
    testobjs = test_objects(kit)
    tom = tomproj.staticlib(kit)

    opt = kit.cpp.opt(*demo_flags)
    opt.includes += tom.includes
    opt.sources += [here/'tests/main.cpp']
    opt.objects += testobjs.objs
    opt.libs += testobjs.libs
    opt.syslibs += testobjs.syslibs

    return [process(kit.cpp.exe('dbg_unittests', opt))]

@target
def run_tests(kit):
    for proc in tests(kit):
        proc.test()

@target
def default(kit):
    run_tests(kit)
    examples(kit)
