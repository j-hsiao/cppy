from tst import testmodule

import sys
errored = False

tup = ([slice(10,30)],(3.14, 101), "hello", None)
tupcount1 = sys.getrefcount(tup)
print('arguments:', tup)
counts1 = [sys.getrefcount(thing) for thing in tup]
errored = not testmodule.basic_test(*tup)
counts2 = [sys.getrefcount(thing) for thing in tup]
if counts1 != counts2:
    print('-------------------------')
    print('ERROR: arg refcounts do not match.')
    print('-------------------------')
    print('before', counts1)
    print('after ', counts2)
    errored = True
tupcount2 = sys.getrefcount(tup)
if tupcount1 != tupcount2:
    print('-------------------------')
    print('ERROR: argtup refcounts do not match.')
    print('-------------------------')
    print('before', tupcount1)
    print('after ', tupcount2)
    errored = True




# print('call cpp with py tup args:', 'pass' if testmodule.call_test(1, 2) == 3 else 'fail')
# print('call py with cpp args: ', 'pass' if testmodule.call_test2((lambda x, y: x+y)) == 3 else 'fail')

# print('tup creation:', 'pass' if testmodule.tupassign_test() == (1,'hello world!',1.5,3.25,5, ('hello', 42)) else 'fail')
# print(testmodule.tupassign_test())
if errored:
    sys.exit(1)
