from tst import testmodule

import sys

class MyThing(object):
    def __init__(self):
        self.name = "MyThing"
        self.value = 32

    def __repr__(self):
        return 'MyThing(32)'
    def __str__(self):
        return 'MyThing(32)'

# very weird occurrence, it seems like
# calling str/repr causes None to get decrefed
# but if call it, then afterwards, None does not
# change again...
str(MyThing())
repr(MyThing())

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
        print('  -------------------------')
        print('  ERROR: arg refcounts do not match.')
        print('  -------------------------')
        print('  before', counts1)
        print('  after ', counts2)
        errored = True
    if tupcount2 != tupcount1:
        print('  -------------------------')
        print('  ERROR: argtup refcounts do not match.')
        print('  -------------------------')
        print('  before', tupcount1)
        print('  after ', tupcount2)
        errored = True
    return errored

if (
    check(testmodule.test_object, [slice(10,30)], (3.14, 101), "hello", None, MyThing())
    or check(testmodule.test_tuple, [slice(10,30)], (3.14, 101), "hello", None)
    or check(testmodule.test_convert, 1,2,3,'a', 'b', 'c')
    or check(testmodule.test_list, 'abc', 123)
    or check(testmodule.test_dict, dict(a=1, b=2))
):
    sys.exit(1)
