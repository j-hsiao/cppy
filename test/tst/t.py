from tst import testmodule

import sys

tup = ([slice(10,30)],(3.14, 101), "hello")
counts1 = [sys.getrefcount(thing) for thing in tup]
print('arguments:', tup)
testmodule.basic_test(*tup)
counts2 = [sys.getrefcount(thing) for thing in tup]
if counts1 != counts2:
    print('-------------------------')
    print('ERROR: refcounts do not match.')
    print('-------------------------')



print('call test:', 'pass' if testmodule.call_test(1, 2) == 3 else 'fail')
print('call test2:', 'pass' if testmodule.call_test2((lambda x, y: x+y)) == 3 else 'fail')

print('tup creation:', 'pass' if testmodule.tupassign_test() == (1,'hello world!',1.5,3.25,5, ('hello', 42)) else 'fail')
print(testmodule.tupassign_test())

