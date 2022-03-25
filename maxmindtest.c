/*
 * maxmindtest.c
 *
 *  Created on: Mar 24, 2022
 *      Author: dad


The code is based on an example found at:
https://maxmind.github.io/libmaxminddb/

The original is:
Copyright 2013-2014 MaxMind, Inc.

My changes are:
Copyright 2022 John Kuras
 */


#include <errno.h>
#include <maxminddb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <csv.h>

static int fieldnum;
MMDB_s mmdb;
MMDB_entry_data_list_s *entry_data_list = NULL;

int checkentry(char * ip_address, MMDB_s mmdb, MMDB_entry_data_list_s *entry_data_list);

void cb1 (void *s, size_t i, void *p) {
  if (fieldnum) putc(',', stdout);
  if ( s ) fprintf(stdout,"%s",(char*)s);
  fieldnum++;
}

void cb2 (int c, void *p) {
	fieldnum = 0;
  putc('\n', stdout);
}

void cb3 (void *s, size_t i, void *p) {
  if (fieldnum) putc(',', stdout);
  if ( s ) fprintf(stdout,"%s",(char*)s);
  fieldnum++;
}


int main(int argc, char **argv)
{
    char *defaultfilename = "/var/lib/GeoIP/GeoLite2-Country.mmdb";
    char *defaultcountries_file = "GeoLite2-Country-Locations-en.csv";
    char *defaultblocks_file = "GeoLite2-Country-Blocks-IPv4.csv";
    char *filename = defaultfilename;
    char *countries_file = defaultcountries_file;
    char *blocks_file = defaultblocks_file;
    struct csv_parser p;
    int i;
    char c;
    int exit_code;
    FILE *blocksfp = NULL;
    FILE *countriesfp = NULL;

    if ( argc > 1 ) filename = argv[1];
    if ( argc > 2 ) countries_file = argv[2];
    if ( argc > 3 ) blocks_file = argv[3];
    if ( !strcmp(filename,"*") ) filename = defaultfilename;
    if ( !strcmp(countries_file,"*") ) countries_file = defaultcountries_file;
    if ( !strcmp(blocks_file,"*") ) blocks_file = defaultblocks_file;

    int status = MMDB_open(filename, MMDB_MODE_MMAP, &mmdb);

    if (MMDB_SUCCESS != status) {
        fprintf(stderr, "\n Can't open %s - %s\n",
                filename, MMDB_strerror(status));

        if (MMDB_IO_ERROR == status) {
            fprintf(stderr, " IO error: %s\n", strerror(errno));
        }
        exit(1);
    }
// get country list
    csv_init(&p, CSV_STRICT_FINI | CSV_APPEND_NULL | CSV_EMPTY_IS_NULL);
    if ( (countriesfp = fopen(countries_file, "r")) == NULL ) {
    	 fprintf(stderr, " IO error: %s while trying to open %s\n", strerror(errno), countries_file);
    	 goto end;
    }
    fieldnum = 0;

    while ((i=getc(countriesfp)) != EOF) {
      c = i;
      if (csv_parse(&p, &c, 1, cb1, cb2, NULL) != 1) {
        fprintf(stderr, " Error: %s\n", csv_strerror(csv_error(&p)));
        exit_code = EXIT_FAILURE;
        goto end;
      }
    }
// read blocks
    csv_fini(&p, cb3, cb2, NULL);
    csv_free(&p);

    csv_init(&p, CSV_STRICT_FINI | CSV_APPEND_NULL | CSV_EMPTY_IS_NULL);
    if ( (blocksfp = fopen(blocks_file, "r")) == NULL ) {
    	 fprintf(stderr, " IO error: %s while trying to open %s\n", strerror(errno), blocks_file);
    	 goto end;
    }
    fieldnum = 0;

    while ((i=getc(blocksfp)) != EOF) {
      c = i;
      if (csv_parse(&p, &c, 1, cb3, cb2, NULL) != 1) {
        fprintf(stderr, " Error: %s\n", csv_strerror(csv_error(&p)));
        exit_code = EXIT_FAILURE;
        goto end;
      }
    }

    csv_fini(&p, cb3, cb2, NULL);
    csv_free(&p);

    fprintf(stderr, " done\n");

    end:
        MMDB_free_entry_data_list(entry_data_list);
        MMDB_close(&mmdb);
        if ( countriesfp ) fclose(countriesfp);
        if ( blocksfp ) fclose(blocksfp);
        exit(exit_code);
}



int checkentry(char * ip_address, MMDB_s mmdb, MMDB_entry_data_list_s *entry_data_list) {
	int exit_code = 0;
	int gai_error, mmdb_error;
	MMDB_lookup_result_s result =
	    MMDB_lookup_string(&mmdb, ip_address, &gai_error, &mmdb_error);

	if (0 != gai_error) {
	    fprintf(stderr,
	            "\n  Error from getaddrinfo for %s - %s\n\n",
	            ip_address, gai_strerror(gai_error));
	    exit(2);
	}

	if (MMDB_SUCCESS != mmdb_error) {
		fprintf(stderr,
				"\n  Got an error from libmaxminddb: %s\n\n",
				MMDB_strerror(mmdb_error));
		exit_code = 3;
		goto end;
	}


	if (result.found_entry) {
		int status = MMDB_get_entry_data_list(&result.entry,
											  &entry_data_list);

		if (MMDB_SUCCESS != status) {
			fprintf(
				stderr,
				" Got an error looking up the entry data - %s\n",
				MMDB_strerror(status));
			exit_code = 4;
			goto end;
		}

		if (NULL != entry_data_list) {
			MMDB_dump_entry_data_list(stdout, entry_data_list, 2);
		}
	} else {
		fprintf(
			stderr,
			"\n  No entry for this IP address (%s) was found\n\n",
			ip_address);
		exit_code = 5;
	}

end:
	return(exit_code);
}


