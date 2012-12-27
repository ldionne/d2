project('fungo')

# Variables
flags = cfgvar('fungo.flags', 'flags', description='list of abstract build flags (see doozer docs)')
test_o_matic_root = cfgvar('test_o_matic.root', description='the path to test_o_matic')


# Dependencies
test_o_matic = subproject(test_o_matic_root/'make.py')


# Builds a static library for fungo.
@target
def staticlib(kit):
    opt = kit.cpp.opt(*flags)
    opt.sources += here/'src/*.cpp'
    opt.includes += [here/'include']

    return properties(
        libs = [kit.cpp.lib('fungo', opt)],
        includes = [here/'include']
    )

@target
def test_objects(kit):
    fungo = staticlib(kit)
    tom = test_o_matic.staticlib(kit)

    opt = kit.cpp.opt(*flags)
    opt.sources += here/'tests/*.cpp'
    opt.sources.erase(here/'tests/main.cpp')
    opt.includes += fungo.includes + tom.includes

    return properties(
        objs = kit.cpp.objs(opt),
        libs = fungo.libs + tom.libs,
        syslibs = fungo.syslibs + tom.syslibs
    )

@target
def testlibs(kit):
    testobjs = test_objects(kit)
    tom = test_o_matic.staticlib(kit)

    opt = kit.cpp.opt(*flags)
    opt.objects += testobjs.objs
    opt.libs += testobjs.libs
    opt.syslibs += testobjs.syslibs

    return [kit.cpp.shlib('fungo_testlib', opt)]
    
@target
def tests(kit):
    testobjs = test_objects(kit)
    tom = test_o_matic.staticlib(kit)

    opt = kit.cpp.opt(*flags)
    opt.includes += tom.includes
    opt.sources += [here/'tests/main.cpp']
    opt.objects += testobjs.objs
    opt.libs += testobjs.libs
    opt.syslibs += testobjs.syslibs

    return [process(kit.cpp.exe('fungo_tests', opt))]

@target
def run_tests(kit):
    for proc in tests(kit):
        proc.test()


# Targets for fungo's example programs
def make_example(kit, name):
    fungo = staticlib(kit)

    opt = kit.cpp.opt(*flags)

    opt.sources += here/'examples'/name/'*.cpp'
    opt.includes += fungo.includes
    opt.libs += fungo.libs
    opt.syslibs += fungo.syslibs

    return process(kit.cpp.exe(name, opt))

@target
def basic_example(kit): return make_example(kit, 'basic')

@target
def clone_example(kit): return make_example(kit, 'clone')

# Builds all the examples for fungo
@target
def examples(kit):
    return [basic_example(kit), clone_example(kit)]

@target
def default(kit):
    run_tests(kit)
    examples(kit)
