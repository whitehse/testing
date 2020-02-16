#!/usr/bin/python3

#from struct import *
import struct
import lmdb

#print (struct.calcsize('!hhl'))

key = struct.pack('!HH', 1, 1)
value = struct.pack('!12s', 'Hello World!'.encode('ascii'))

# Open (and create if necessary) our database environment. Must specify
# max_dbs=... since we're opening subdbs.
env = lmdb.open('./test.lmdb', max_dbs=10)

test_db = env.open_db(b'test')

#with env.begin(write=True) as txn:
#    txn.put(key, value, db=test_db)

for i in range(1000000):
    with env.begin(write=False) as txn:
        new_value = txn.get(key, db=test_db)
        string = struct.unpack('!12s', new_value)
        #print (string)

# Iterate each DB to show the keys are sorted:
#with env.begin() as txn:
#    for name, db in ('test', home_db):
#        print('DB:', name)
#        for key, value in txn.cursor(db=db):
#            print('  ', key, value)
#        print()

#
# Now let's update some phone numbers. We can specify the default subdb when
# starting the transaction, rather than pass it in every time:
#with env.begin(write=True, db=home_db) as txn:
#    print('Updating number for dentist')
#    txn.put(b'dentist', b'099991231')
#
#    print('Deleting number for hospital')
#    txn.delete(b'hospital')
#    print()
#
#    print('Home DB is now:')
#    for key, value in txn.cursor():
#        print('  ', key, value)
#    print()
#
#
# Now let's look up a number in the business DB
#with env.begin(db=business_db) as txn:
#    print('Boss telephone number:', txn.get(b'boss'))
#    print()
#
#
# We got fired, time to delete all keys from the business DB.
#with env.begin(write=True) as txn:
#    print('Deleting all numbers from business DB:')
#    txn.drop(business_db, delete=False)
#
#    print('Adding number for recruiter to business DB')
#    txn.put(b'recruiter', b'04123125324', db=business_db)
#
#    print('Business DB is now:')
#    for key, value in txn.cursor(db=business_db):
#        print('  ', key, value)
#    print()
