from tst import testmodule

import sys

tup = ([slice(10,30)],(3.14, 101), "hello")
counts1 = [sys.getrefcount(thing) for thing in tup]
print('arguments:', tup)
testmodule.basic_test(*tup)
counts2 = [sys.getrefcount(thing) for thing in tup]
print('counts match:', counts1 == counts2)


testmodule.tuparg_test(*tup)
counts3 = [sys.getrefcount(thing) for thing in tup]
print('counts match', counts1 == counts2)




print('pass' if testmodule.call_test(1, 2) == 3 else 'fail')
