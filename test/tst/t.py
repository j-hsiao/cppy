from tst import testmodule

import sys

class MyThing(object):
    def __init__(self):
        self.name = "MyThing"
        self.value = 32

    def __repr__(self):
        return self.name
    def __str__(self):
        return self.name

def check(func, *args):
    """Call func with args.

    Return True if an error happened.  Otherwise, False.
    """
    print('------------------------------')
    print(func.__name__)
    print('------------------------------')
    print('  args:', args)
    tupcount1 = sys.getrefcount(args)
    counts1 = list(map(sys.getrefcount, args))
    errored = not func(*args)
    counts2 = list(map(sys.getrefcount, args))
    tupcount2 = sys.getrefcount(args)
    if errored:
        print('  !!FAILED!! Return value was False.')
    if counts1 != counts2:
        # Some things like str, repr, getattr or something
        # somehow seem to change the None refcount, even
        # within only python so I don't think the changes
        # in sys.refcount(None) indicate leaked/incorrectly
        # removed references or anything...
        for idx, (c1, c2) in enumerate(zip(counts1,counts2)):
            if c1 != c2 and args[idx] is not None:
                print('  -------------------------')
                print('  ERROR: arg refcounts do not match.')
                print('  -------------------------')
                print('  before', counts1)
                print('  after ', counts2)
                errored = True
                break
        else:
            print('Only None refcount changed weird.')
    if tupcount2 != tupcount1:
        print('  -------------------------')
        print('  ERROR: argtup refcounts do not match.')
        print('  -------------------------')
        print('  before', tupcount1)
        print('  after ', tupcount2)
        errored = True
    return errored

def argsonly(*args):
    return args

def kwargsonly(**kwargs):
    return kwargs

def argskwargs(*args, **kwargs):
    return args, kwargs

if (
    check(testmodule.test_object, [slice(10,30)], (3.14, 101), "hello", None, MyThing())
    or check(testmodule.test_str, 'abc123def456')
    or check(testmodule.test_tuple, [slice(10,30)], (3.14, 101), "hello", None)
    or check(testmodule.test_convert, 1,2,3,'a', 'b', 'c')
    or check(testmodule.test_list, 'abc', 123)
    or check(testmodule.test_dict, dict(a=1, b=2))
    or check(testmodule.test_call, argsonly, kwargsonly, argskwargs)
):
    sys.exit(1)
