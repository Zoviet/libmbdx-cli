/*
 * Copyright 2015-2022 Leonid Yuriev <leo@yuriev.ru>.
 * Copyright 2017 Ilya Shipitsin <chipitsine@gmail.com>.
 * Copyright 2012-2015 Howard Chu, Symas Corp.
 * Copyright 2024 Zoviet, asustem.ru.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */

#include "mdbx.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void help(void) {
   printf("Usage: mdbx [db_path...] [--help] [--del]  [--put] [--get] [--info] [--del]\n Options:\n--put key value :put key-value pair\n--del key :delete key\n--get key :get key\n--info :db info\n");
}

int main(int argc, char **argv) {
	
	int rc;
	MDBX_env *env = NULL;
	MDBX_dbi dbi = 0;
	MDBX_val key, data;
	MDBX_txn *txn = NULL;
	MDBX_cursor *cursor = NULL;
	char sval[32];

#if UINTPTR_MAX > 0xffffFFFFul || ULONG_MAX > 0xffffFFFFul
	const double scale_factor = 1099511627776.0;
	const char *const scale_unit = "TiB";
#else
	const double scale_factor = 1073741824.0;
	const char *const scale_unit = "GiB";
#endif
	const size_t pagesize_min = mdbx_limits_pgsize_min();
	const size_t pagesize_max = mdbx_limits_pgsize_max();
	const size_t pagesize_default = mdbx_default_pagesize();
	
	if(argc <= 1 || !strcmp(argv[1], "--help")) {
		help();
		return 0;
	}	
	
	rc = mdbx_env_create(&env);
	if (rc != MDBX_SUCCESS) {
		fprintf(stderr, "mdbx_env_create: (%d) %s\n", rc, mdbx_strerror(rc));
		goto bailout;
	}
	
	rc = mdbx_env_open(env, argv[1], MDBX_NOSUBDIR | MDBX_COALESCE | MDBX_LIFORECLAIM, 0664);
	if (rc != MDBX_SUCCESS) {
		fprintf(stderr, "mdbx_env_open: (%d) %s\n", rc, mdbx_strerror(rc));
		goto bailout;
	}
	
	rc = mdbx_txn_begin(env, NULL, 0, &txn);
	if (rc != MDBX_SUCCESS) {
		fprintf(stderr, "mdbx_txn_begin: (%d) %s\n", rc, mdbx_strerror(rc));
		goto bailout;
	}
	
	rc = mdbx_dbi_open(txn, NULL, 0, &dbi);
	if (rc != MDBX_SUCCESS) {
		fprintf(stderr, "mdbx_dbi_open: (%d) %s\n", rc, mdbx_strerror(rc));
		goto bailout;
	}

	if(!strcmp(argv[2], "--put")) {
		if(argc <= 4) {
			printf("'--put' operation requires two parameters: key and value.\n");
			return 0;
		} 
		key.iov_len = sizeof(argv[3]);
		key.iov_base = argv[3];
		data.iov_len = sizeof(argv[4]);
		data.iov_base = argv[4];
		rc = mdbx_put(txn, dbi, &key, &data, 0);
		if (rc != MDBX_SUCCESS) {
			fprintf(stderr, "mdbx_put: (%d) %s\n", rc, mdbx_strerror(rc));
			goto bailout;
		}
		rc = mdbx_txn_commit(txn);
		if (rc) {
			fprintf(stderr, "mdbx_txn_commit: (%d) %s\n", rc, mdbx_strerror(rc));
			goto bailout;
		}	
		goto bailout;
	} else if (!strcmp(argv[2], "--get")) {
		if(argc <= 3) {
			printf("'--get' operation requires key.\n");
			return 0;
		} 
		key.iov_len = sizeof(argv[3]);
		key.iov_base = argv[3];
		MDBX_val v = {0};
		rc = mdbx_get(txn, dbi, &key, &v);		
		if (rc != MDBX_SUCCESS) {
			if (rc == MDBX_NOTFOUND) {
				printf("Key '%s' not found", argv[3]);
				goto bailout;
			}
			fprintf(stderr, "mdbx_put: (%d) %s\n", rc, mdbx_strerror(rc));
			goto bailout;
		}
		printf("%s", (char *) v.iov_base);	
		goto bailout;
	} else if (!strcmp(argv[2], "--del")) {
		if(argc <= 3) {
			printf("'--del' operation requires key.\n");
			return 0;
		} 
		key.iov_len = sizeof(argv[3]);
		key.iov_base = argv[3];
		rc = mdbx_del(txn, dbi, &key, NULL);		
		if (rc != MDBX_SUCCESS) {
			if (rc == MDBX_EINVAL) {
				printf("Key '%s' not found", argv[3]);
				goto bailout;
			}
			fprintf(stderr, "mdbx_put: (%d) %s\n", rc, mdbx_strerror(rc));
			goto bailout;
		}		
		rc = mdbx_txn_commit(txn);
		if (rc) {
			fprintf(stderr, "mdbx_txn_commit: (%d) %s\n", rc, mdbx_strerror(rc));
			goto bailout;
		}	
		goto bailout;
	} else if (!strcmp(argv[2], "--info")) {
		printf("\tWrite transaction size: up to %zu (0x%zX) pages (%f %s for default "
         "%zuK pagesize, %f %s for %zuK pagesize).\n",
         mdbx_limits_txnsize_max(pagesize_min) / pagesize_min,
         mdbx_limits_txnsize_max(pagesize_min) / pagesize_min,
         mdbx_limits_txnsize_max(-1) / scale_factor, scale_unit,
         pagesize_default / 1024,
         mdbx_limits_txnsize_max(pagesize_max) / scale_factor, scale_unit,
         pagesize_max / 1024);
		printf("\tDatabase size: up to %zu pages (%f %s for default %zuK "
         "pagesize, %f %s for %zuK pagesize).\n",
         mdbx_limits_dbsize_max(pagesize_min) / pagesize_min,
         mdbx_limits_dbsize_max(-1) / scale_factor, scale_unit,
         pagesize_default / 1024,
         mdbx_limits_dbsize_max(pagesize_max) / scale_factor, scale_unit,
         pagesize_max / 1024);
	} else {
		printf("Unknown parameter: '%s'. Type %s --help for help.\n", argv[2], argv[0]);
	}
	bailout:
	if (cursor)
		mdbx_cursor_close(cursor);
	if (txn)
		mdbx_txn_abort(txn);
	if (dbi)
		mdbx_dbi_close(env, dbi);
	if (env)
		mdbx_env_close(env);
	return (rc != MDBX_SUCCESS) ? EXIT_FAILURE : EXIT_SUCCESS;
 }
