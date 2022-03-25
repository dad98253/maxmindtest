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

typedef struct CountryData
{
	int			geoname_id;
	char *		locale_code;
	char *		continent_code;
	char *		continent_name;
	char *		country_iso_code;
	char *		country_name;
} CountryDataStructure;
/*
 geoname_id	integer	A unique identifier for the a location as specified by GeoNames. This ID can be used as a key for the Location file.

locale_code	string	The locale that the names in this row are in. This will always correspond to the locale name of the file.

continent_code	string (2)

The continent code for this location. Possible codes are:

    AF - Africa
    AN - Antarctica
    AS - Asia
    EU - Europe
    NA - North America
    OC - Oceania
    SA - South America

continent_name	string	The continent name for this location in the file's locale.

country_iso_code	string (2)	A two-character ISO 3166-1 country code for the country associated with the location.

country_name	string	The country name for this location in the file's locale.
 */
CountryDataStructure *tempCountryData = NULL;
CountryDataStructure **CountryDataBase = NULL;
#define NSIZECOUNTRY 1000

typedef struct BlockData
{
	char *		network;
	int			geoname_id;
	int			registered_country_geoname_id;
	int			represented_country_geoname_id;
	int			is_anonymous_proxy;
	int			is_satellite_provider;
} BlockDataStructure;
/*
 Name	Type							Description
network	IP network as a string			This is the IPv4 or IPv6 network in CIDR format such as "2.21.92.0/29" or "2001:4b0::/80".
										We offer a utility to convert this column to start/end IPs or start/end integers.
										See the conversion utility section for details.

geoname_id	integer						A unique identifier for the network's location as specified by GeoNames. This ID can be used to look
										up the location information in the Location file.

registered_country_geoname_id	integer	The registered country is the country in which the ISP has registered the network.
										This column contains a unique identifier for the network's registered country as
										specified by GeoNames. This ID can be used to look up the location information in
										the Location file.

represented_country_geoname_id	integer	The represented country is the country which is represented by users of the IP address.
										For instance, the country represented by an overseas military base. This column contains
										a unique identifier for the network's registered country as specified by GeoNames. This
										ID can be used to look up the location information in the Location file.

is_anonymous_proxy	boolean				Deprecated. Please see our GeoIP2 Anonymous IP database to determine whether the IP address is
										used by an anonymizing service.

is_satellite_provider	boolean			Deprecated.
 */
BlockDataStructure *tempBlockData = NULL;
BlockDataStructure **BlockDataBase = NULL;
#define NSIZEBLOCKDATA 1000000

static int fieldnum;
static int numcountries = 0;
static int numblocks = 0;
MMDB_s mmdb;
MMDB_entry_data_list_s *entry_data_list = NULL;

int checkentry(char * ip_address, MMDB_s mmdb, MMDB_entry_data_list_s *entry_data_list);
void quickSort(CountryDataStructure *array[], int low, int high);
void cleanupmem();

void cb1 (void *s, size_t i, void *p) {
	int id;
#ifdef DEBUG
	if (fieldnum) putc(',', stdout);
	if ( s ) fprintf(stdout,"%s",(char*)s);
#endif	// DEBUG
	fieldnum++;
	if(s) {
		switch (fieldnum) {
		case 1:
			sscanf(s,"%i",&id);
			tempCountryData->geoname_id = id;
			break;
		case 2:
			tempCountryData->locale_code = (char*)malloc(strlen(s)+1);
			strcpy(tempCountryData->locale_code,s);
			break;
		case 3:
			tempCountryData->continent_code = (char*)malloc(strlen(s)+1);
			strcpy(tempCountryData->continent_code,s);
			break;
		case 4:
			tempCountryData->continent_name = (char*)malloc(strlen(s)+1);
			strcpy(tempCountryData->continent_name,s);
			break;
		case 5:
			tempCountryData->country_iso_code = (char*)malloc(strlen(s)+1);
			strcpy(tempCountryData->country_iso_code,s);
			break;
		case 6:
			tempCountryData->country_name = (char*)malloc(strlen(s)+1);
			strcpy(tempCountryData->country_name,s);
			break;
		default:
			break;
		}
	}
}

void cb2 (int c, void *p) {
#ifdef DEBUG
	putc('\n', stdout);
#endif	// DEBUG
	*(CountryDataBase+numcountries) = tempCountryData;
	tempCountryData = (CountryDataStructure*) calloc(1, sizeof(CountryDataStructure));
	numcountries++;
	fieldnum = 0;
}

void cb3 (void *s, size_t i, void *p) {
  int id;
#ifdef DEBUG
  if (fieldnum) putc(',', stdout);
  if ( s ) fprintf(stdout,"%s",(char*)s);
#endif	// DEBUG
  fieldnum++;
	if(s) {
		switch (fieldnum) {
		case 1:
			tempBlockData->network = (char*)malloc(strlen(s)+1);
			strcpy(tempBlockData->network,s);
			break;
		case 2:
			sscanf(s,"%i",&id);
			tempBlockData->geoname_id = id;
			break;
		case 3:
			sscanf(s,"%i",&id);
			tempBlockData->registered_country_geoname_id = id;
			break;
		case 4:
			sscanf(s,"%i",&id);
			tempBlockData->represented_country_geoname_id = id;
			break;
		case 5:
			sscanf(s,"%i",&id);
			tempBlockData->is_anonymous_proxy = id;
			break;
		case 6:
			sscanf(s,"%i",&id);
			tempBlockData->is_satellite_provider = id;
			break;
		default:
			break;
		}
	}
}

void cb4 (int c, void *p) {
#ifdef DEBUG
	putc('\n', stdout);
#endif	// DEBUG
	*(BlockDataBase+numblocks) = tempBlockData;
	tempBlockData = (BlockDataStructure*) calloc(1, sizeof(BlockDataStructure));
	numblocks++;
	fieldnum = 0;
}

// qsort/bsearch compare callback function
int qscompare(const void* array, const void* pivot){
	return ( ((CountryDataStructure*)array)->geoname_id - ((CountryDataStructure*)pivot)->geoname_id );
//	if(((CountryDataStructure*)array)->geoname_id >= ((CountryDataStructure*)pivot)->geoname_id) return (1);
//	return(0);
}
int bscompare(const void* array, const void* pivot){
	CountryDataStructure** temp;
	temp = (CountryDataStructure**)pivot;
	return ( ((CountryDataStructure*)array)->geoname_id - (*temp)->geoname_id );
//	if(((CountryDataStructure*)array)->geoname_id >= ((CountryDataStructure*)pivot)->geoname_id) return (1);
//	return(0);
}


int main(int argc, char **argv)
{
    char *defaultfilename = "/var/lib/GeoIP/GeoLite2-Country.mmdb";
    char *defaultcountries_file = "GeoLite2-Country-Locations-en.csv";
    char *defaultblocks_file = "GeoLite2-Country-Blocks-IPv4.csv";
    char *filename = defaultfilename;
    char *countries_file = defaultcountries_file;
    char *blocks_file = defaultblocks_file;
    CountryDataStructure ** tempCountryDatap = NULL;
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
    CountryDataBase = (CountryDataStructure**) calloc(NSIZECOUNTRY, sizeof(CountryDataStructure*));
    tempCountryData = (CountryDataStructure*) calloc(1, sizeof(CountryDataStructure));
    BlockDataBase = (BlockDataStructure**) calloc(NSIZEBLOCKDATA, sizeof(BlockDataStructure*));
    tempBlockData = (BlockDataStructure*) calloc(1, sizeof(BlockDataStructure));
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
// check what was saved
#ifdef DEBUG
    fprintf(stdout,"---------------------------------\n");
    if (numcountries) {
    	qsort((void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),qscompare);
    	for (int i=1;i<numcountries;i++) {
    		tempCountryData = *(CountryDataBase+i);
    		if (tempCountryData->geoname_id < 1 ) {
    			fprintf(stdout,"bad data at country index = %i\n",i);
    		} else {
    			fprintf(stdout,"%i,",tempCountryData->geoname_id);
    		}
    		if ( tempCountryData->locale_code ) {
    			fprintf(stdout,"%s,",tempCountryData->locale_code);
    		} else {
    			fprintf(stdout,",");
    		}
    		if ( tempCountryData->continent_code ) {
    			fprintf(stdout,"%s,",tempCountryData->continent_code);
    		} else {
    			fprintf(stdout,",");
    		}
    		if ( tempCountryData->continent_name ) {
    			fprintf(stdout,"%s,",tempCountryData->continent_name);
    		} else {
    			fprintf(stdout,",");
    		}
    		if ( tempCountryData->country_iso_code ) {
    			fprintf(stdout,"%s,",tempCountryData->country_iso_code);
    		} else {
    			fprintf(stdout,",");
    		}
    		if ( tempCountryData->country_name ) {
    			fprintf(stdout,"%s",tempCountryData->country_name);
    		}
    		fprintf(stdout,"\n");
    	}
    }
    fprintf(stdout,"---------------------------------\n");
    // void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *))
    // find 4032283 (Tonga)
    int indx = 4032283;
    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&indx, (void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
    if( tempCountryData != NULL ) {
       printf("Found geoname_id = %i\n", (*tempCountryDatap)->geoname_id);
    } else {
       printf("geoname_id = %i could not be found\n", indx);
    }
    fprintf(stdout,"---------------------------------\n");
#endif	// DEBUG
    csv_init(&p, CSV_STRICT_FINI | CSV_APPEND_NULL | CSV_EMPTY_IS_NULL);
    if ( (blocksfp = fopen(blocks_file, "r")) == NULL ) {
    	 fprintf(stderr, " IO error: %s while trying to open %s\n", strerror(errno), blocks_file);
    	 goto end;
    }
    fieldnum = 0;

    while ((i=getc(blocksfp)) != EOF) {
      c = i;
      if (csv_parse(&p, &c, 1, cb3, cb4, NULL) != 1) {
        fprintf(stderr, " Error: %s\n", csv_strerror(csv_error(&p)));
        exit_code = EXIT_FAILURE;
        goto end;
      }
    }

    csv_fini(&p, cb3, cb4, NULL);
    csv_free(&p);

#ifdef DEBUG
    fprintf(stdout,"---------------------------------\n");
    // void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *))
    // find 4032283

    if (numblocks) {
    	for (int i=1;i<numblocks;i++) {
    		tempBlockData = *(BlockDataBase+i);
    		if ( tempBlockData->network ) {
    			fprintf(stdout,"%s,",tempBlockData->network);
    		} else {
    			fprintf(stdout,",");
    		}
    		// geoname_id
    		if (tempBlockData->geoname_id < 1 ) {
    			//fprintf(stdout,"bad data at block index = %i, geoname_id = %i\n",i,tempBlockData->geoname_id);
    		} else {
    		    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->geoname_id), (void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
    		    if( tempCountryData != NULL ) {
    		    	fprintf(stdout,"%i",tempBlockData->geoname_id);
    		    	if ((*tempCountryDatap)->country_iso_code) {
    		    		fprintf(stdout,"[%s]",(*tempCountryDatap)->country_iso_code);
    		    	} else {
    		    		fprintf(stdout,"[%s]",(*tempCountryDatap)->continent_name);
    		    	}
    		    } else {
    		       printf("geoname_id = %i could not be found\n", tempBlockData->geoname_id);
    		    }
    		}
	    	fprintf(stdout,",");
    		// registered_country_geoname_id
    		if (tempBlockData->registered_country_geoname_id < 1 ) {
    			//fprintf(stdout,"bad data at block index = %i, geoname_id = %i\n",i,tempBlockData->geoname_id);
    		} else {
    		    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->registered_country_geoname_id), (void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
    		    if( tempCountryData != NULL ) {
    		    	fprintf(stdout,"%i",tempBlockData->registered_country_geoname_id);
    		    	if ((*tempCountryDatap)->country_iso_code) {
    		    		fprintf(stdout,"[%s]",(*tempCountryDatap)->country_iso_code);
    		    	} else {
    		    		fprintf(stdout,"[%s]",(*tempCountryDatap)->continent_name);
    		    	}
    		    } else {
    		       printf("registered_country_geoname_id = %i could not be found\n", tempBlockData->registered_country_geoname_id);
    		    }
    		}
	    	fprintf(stdout,",");
    		// represented_country_geoname_id
    		if (tempBlockData->represented_country_geoname_id < 1 ) {
    			//fprintf(stdout,"bad data at block index = %i, geoname_id = %i\n",i,tempBlockData->geoname_id);
    		} else {
    		    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->represented_country_geoname_id), (void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
    		    if( tempCountryData != NULL ) {
    		    	fprintf(stdout,"%i",tempBlockData->represented_country_geoname_id);
    		    	if ((*tempCountryDatap)->country_iso_code) {
    		    		fprintf(stdout,"[%s]",(*tempCountryDatap)->country_iso_code);
    		    	} else {
    		    		fprintf(stdout,"[%s]",(*tempCountryDatap)->continent_name);
    		    	}
    		    } else {
    		       printf("represented_country_geoname_id = %i could not be found\n", tempBlockData->represented_country_geoname_id);
    		    }
    		}
	    	fprintf(stdout,",");
    		//	is_anonymous_proxy;
    		if ( tempBlockData->is_anonymous_proxy ) {
    			fprintf(stdout,"1,");
    		} else {
    			fprintf(stdout,"0,");
    		}
    		//	is_satellite_provider;
    		if ( tempBlockData->is_satellite_provider ) {
    			fprintf(stdout,"1");
    		} else {
    			fprintf(stdout,"0");
    		}

    		fprintf(stdout,"\n");
    	}
    }
#endif	// DEBUG
    fprintf(stderr, " done\n");

    end:
        MMDB_free_entry_data_list(entry_data_list);
        MMDB_close(&mmdb);
        if ( countriesfp ) fclose(countriesfp);
        if ( blocksfp ) fclose(blocksfp);
        cleanupmem();
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

void cleanupmem() {
	int i;
	if (numcountries) {
		for (i=0;i<numcountries;i++) {
			tempCountryData = *(CountryDataBase+i);
			if (tempCountryData) {
				if (tempCountryData->locale_code) free (tempCountryData->locale_code);
				if (tempCountryData->continent_code) free (tempCountryData->continent_code);
				if (tempCountryData->continent_name) free (tempCountryData->continent_name);
				if (tempCountryData->country_iso_code) free (tempCountryData->country_iso_code);
				if (tempCountryData->country_name) free (tempCountryData->country_name);
				free(tempCountryData);
			}
		}
		free(CountryDataBase);
		CountryDataBase = NULL;
		tempCountryData = NULL;
	}
	if (numblocks) {
		for (i=0;i<numblocks;i++) {
			tempBlockData = *(BlockDataBase+i);
			if (tempBlockData) {
				if (tempBlockData->network) free (tempBlockData->network);
				free(tempBlockData);
			}
		}
		free(BlockDataBase);
		BlockDataBase = NULL;
		tempBlockData = NULL;
	}
}

