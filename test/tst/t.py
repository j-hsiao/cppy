from tst import testmodule

print('arguments:', (1,2,(3, 4), "hello"))
testmodule.basic_test(1,2,(3, 4), "hello")

print('pass' if testmodule.call_test(1, 2) == 3 else 'fail')
