#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "lmdb.h"

int main(int argc,char * argv[])
{
	int rc;
	MDB_env *env;
	MDB_dbi dbi;
	MDB_val key, data;
	MDB_txn *txn;
	MDB_cursor *cursor;
	int index = 0;
	int val;

	srand(time(NULL));

	rc = mdb_env_create(&env);
	rc = mdb_env_open(env, "./boguscoin_db", 0, 0664);

	key.mv_size = sizeof(index);
	data.mv_size = sizeof(val);

	for (int j=0; j<=10; j++) {
		index++;
		key.mv_data = &index;
		val = rand();
		data.mv_data = &val;
		rc = mdb_txn_begin(env, NULL, 0, &txn);
		rc = mdb_open(txn, NULL, 0, &dbi);
		rc = mdb_put(txn, dbi, &key, &data, 0);
		rc = mdb_txn_commit(txn);
		if (rc) {
			fprintf(stderr, "mdb_txn_commit: (%d) %s\n", rc, mdb_strerror(rc));
			goto leave;
		}
		printf ("Inserted key=%d, and value=%d\n", index, val);
		sleep (1);
	}

leave:
	mdb_close(env, dbi);
	mdb_env_close(env);
	return 0;
}
