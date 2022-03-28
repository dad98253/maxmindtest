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

int checkentry(char * ip_address, MMDB_s mmdb, MMDB_entry_data_list_s *entry_data_list, int whichentry, char* whichiso);
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
    char *defaultfilter = "CN";
    char *defaultformat = "1";
    char *defaultinterface = "eth0";
    char *defaultblacklist = "blacklist";
    char *filename = defaultfilename;
    char *countries_file = defaultcountries_file;
    char *blocks_file = defaultblocks_file;
    char *filter = defaultfilter;
    char *format = defaultformat;
    char *interface = defaultinterface;
    int iformat = 0;
    long long unsigned int numadds = 0;
    CountryDataStructure ** tempCountryDatap = NULL;
    struct csv_parser p;
    int i;
    char c;
    char tempstr[10000];
    char * pch = NULL;
    int exit_code = 0;
    int filterid = 0;
    char * fileredCountryName = NULL;
    int tempint = 0;
    int iret = 0;
    FILE *blocksfp = NULL;
    FILE *countriesfp = NULL;

    if ( argc > 1 ) filename = argv[1];
    if ( argc > 2 ) countries_file = argv[2];
    if ( argc > 3 ) blocks_file = argv[3];
    if ( argc > 4 ) filter = argv[4];
    if ( argc > 5 ) format = argv[5];
    if ( argc > 6 ) interface = argv[6];
    if ( !strcmp(filename,"*") ) filename = defaultfilename;
    if ( !strcmp(countries_file,"*") ) countries_file = defaultcountries_file;
    if ( !strcmp(blocks_file,"*") ) blocks_file = defaultblocks_file;
    if ( !strcmp(filter,"*") ) filter = defaultfilter;
    if ( !strcmp(format,"*") ) format = defaultformat;
    sscanf(format,"%i",&iformat);
    if ( iformat == 5 ) {
    	if ( !strcmp(interface,"*") || argc == 6 ) interface = defaultblacklist;
    } else {
    	if ( !strcmp(interface,"*") ) interface = defaultinterface;
    }


#ifdef DEBUG
    for (i=0;i<argc;i++) {
    	fprintf(stderr, " %s", argv[i]);
    }
    fprintf(stderr,"\n opening maxmind db file \"%s\"\n filter format is %i\n",filename,iformat);
#endif
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
    // find filtered geoname_id
    if (numcountries &&  argc > 4) {
    	qsort((void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),qscompare);
    	for (int i=1;i<numcountries;i++) {
    		tempCountryData = *(CountryDataBase+i);
     		if (tempCountryData->geoname_id < 1 ) {
    			fprintf(stdout,"bad data at country index = %i\n",i);
    		} else {
				if ( tempCountryData->country_iso_code ) {
					if(!strcmp(tempCountryData->country_iso_code,filter)) {
						filterid = tempCountryData->geoname_id;
						fprintf(stderr," filtering on %s\n",tempCountryData->country_name);
				    	fileredCountryName = (char*) calloc(1,strlen(tempCountryData->country_name)+1);
				    	strcpy(fileredCountryName,tempCountryData->country_name);
					}
				}
    		}
     	}
    }

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
    		    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->geoname_id),
    		    		(void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
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
    		    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->registered_country_geoname_id),
    		    		(void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
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
    		    tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->represented_country_geoname_id),
    		    		(void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
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
    fprintf(stdout,"---------------------------------\n");
#endif	// DEBUG

    if (numblocks && filterid) {
    	numadds = 0;
    	if ( iformat == 2 || iformat == 5 ) fprintf(stdout,"#!/bin/sh\necho \"Drop all IPv4 traffic from %s...\"\n",fileredCountryName);
    	if ( iformat == 3 ) fprintf(stdout,"### tuple ### deny any from %s access to %s\n",fileredCountryName,interface);
    	for (int i=1;i<numblocks;i++) {
    		tempBlockData = *(BlockDataBase+i);
    		if ( (tempBlockData->geoname_id == filterid) ||
    				(tempBlockData->registered_country_geoname_id == filterid) ||
					(tempBlockData->represented_country_geoname_id == filterid) ) {
    			// count the total number of IPv4 addresses this country has assigned to it (just for kicks)...
    			if ( tempBlockData->network ) {
    				// get size of net - find subnet mask
    				pch = strchr(tempBlockData->network,'/');
    				if (pch) {
    					pch++;
    					sscanf(pch,"%i",&tempint);
    					if((tempint < 0) || (tempint > 32) ) {
    						fprintf(stderr,"bad net mask at %s\n",tempBlockData->network);
    					} else {
    						numadds+= 1<<(32-tempint);
    					}
    				}
    				strcpy(tempstr,tempBlockData->network);
    				pch = strchr(tempstr,'/');
    				*pch = '\000';
    				pch++;
    				iret = 0;
    				if ( tempBlockData->geoname_id == filterid ) iret = checkentry(tempstr, mmdb, entry_data_list, 1, filter);
    				if ( iret ) {
    					fprintf(stderr,"checkentry for geoname_id failed on %s , iret = %i\n", tempstr, iret);
    					exit_code = iret;
    					goto end;
    				}
    				iret = 0;
    				if ( tempBlockData->registered_country_geoname_id == filterid ) iret = checkentry(tempstr, mmdb, entry_data_list, 2, filter);
    				if ( iret ) {
    					fprintf(stderr,"checkentry for registered_country_geoname_id failed on %s , iret = %i\n", tempstr, iret);
    					exit_code = iret;
    					goto end;
    				}
    				iret = 0;
    				if ( tempBlockData->represented_country_geoname_id == filterid ) iret = checkentry(tempstr, mmdb, entry_data_list, 3, filter);
    				if ( iret ) {
    					fprintf(stderr,"checkentry for represented_country_geoname_id failed on %s , iret = %i\n", tempstr, iret);
    					exit_code = iret;
    					goto end;
    				}
    			}
    			if ( iformat == 2 ) {
    				// iptables -A INPUT -s 1.0.1.0/24 -j DROP
					if ( tempBlockData->network ) {
						fprintf(stdout,"iptables -A INPUT -i %s -s %s -j DROP\n",interface,tempBlockData->network);
					}
    			} else if ( iformat == 3 ) {
    				//-A ufw-user-input -i eth0 -s 192.168.1.226/24 -j DROP
					if ( tempBlockData->network ) {
						fprintf(stdout,"-A ufw-user-input -i %s -s %s -j DROP\n",interface,tempBlockData->network);
					}
    			} else if ( iformat == 4 ) {
    				// 192.168.1.226/24
					if ( tempBlockData->network ) {
						fprintf(stdout," %s\n",tempBlockData->network);
					}
    			} else if ( iformat == 5 ) {
    				// ipset add blacklist 192.168.1.226/24
					if ( tempBlockData->network ) {
						fprintf(stdout,"ipset add %s %s\n",interface,tempBlockData->network);
					}
    			} else if ( iformat == 1 ) {
    				// csv format similar to csv input but with iso country codes added
					if ( tempBlockData->network ) {
						fprintf(stdout,"%s,",tempBlockData->network);
					} else {
						fprintf(stdout,",");
					}
					// geoname_id
					if (tempBlockData->geoname_id < 1 ) {
						//fprintf(stdout,"bad data at block index = %i, geoname_id = %i\n",i,tempBlockData->geoname_id);
					} else {
						tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->geoname_id),
								(void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
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
						tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->registered_country_geoname_id),
								(void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
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
						tempCountryDatap = (CountryDataStructure**)bsearch((const void *)&(tempBlockData->represented_country_geoname_id),
								(void*)CountryDataBase, numcountries, sizeof(CountryDataStructure *),bscompare);
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
    	}
    	fprintf(stderr," %llu IPv4 addresses assigned to %s\n",numadds,fileredCountryName);
    }

    fprintf(stderr, " done\n");

    end:
        MMDB_free_entry_data_list(entry_data_list);
        MMDB_close(&mmdb);
        if ( countriesfp ) fclose(countriesfp);
        if ( blocksfp ) fclose(blocksfp);
        cleanupmem();
        if (fileredCountryName) free(fileredCountryName);
        exit(exit_code);
}

static size_t mmdb_strnlen(const char *s, size_t maxlen) {
    size_t len;

    for (len = 0; len < maxlen; len++, s++) {
        if (!*s)
            break;
    }
    return (len);
}

static char *mmdb_strndup(const char *str, size_t n) {
    size_t len;
    char *copy;

    len = mmdb_strnlen(str, n);
    if ((copy = malloc(len + 1)) == NULL)
        return (NULL);
    memcpy(copy, str, len);
    copy[len] = '\0';
    return (copy);
}

int checkentry(char * ip_address, MMDB_s mmdb, MMDB_entry_data_list_s *entry_data_list, int whichentry, char* whichiso) {
	int exit_code = 0;
	int gai_error, mmdb_error;
	MMDB_lookup_result_s result =
	    MMDB_lookup_string(&mmdb, ip_address, &gai_error, &mmdb_error);

	if (0 != gai_error) {
	    fprintf(stderr, "\n  Error from getaddrinfo for %s - %s\n\n",
	            ip_address, gai_strerror(gai_error));
	    exit(2);
	}

	if (MMDB_SUCCESS != mmdb_error) {
		fprintf(stderr, "\n  Got an error from libmaxminddb: %s\n\n",
				MMDB_strerror(mmdb_error));
		exit_code = 3;
		goto end;
	}


	if (result.found_entry) {
		int status = MMDB_get_entry_data_list(&result.entry, &entry_data_list);
		if (MMDB_SUCCESS != status) {
			fprintf( stderr, " Got an error looking up the entry data - %s\n",
				MMDB_strerror(status));
			exit_code = 4;
			goto end;
		}


		MMDB_entry_data_s entry_data;
		switch (whichentry) {
		case 1:
			status = MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL);
			break;
		case 2:
			status = MMDB_get_value(&result.entry, &entry_data, "registered_country", "iso_code", NULL);
			break;
		case 3:
			status = MMDB_get_value(&result.entry, &entry_data, "represented_country", "iso_code", NULL);
			break;
		default:
			status = 999;
			break;
		}
		if (MMDB_SUCCESS != status) {
			fprintf( stderr, " Got an error getting iso code value data - %s\n",
				MMDB_strerror(status));
			exit_code = 6;
			goto end;
		}
		if (entry_data.has_data) {
			if (MMDB_DATA_TYPE_UTF8_STRING != entry_data.type) {
				fprintf( stderr, " iso code value type is not a string - %s\n",
					MMDB_strerror(status));
				exit_code = 8;
				goto end;
			}
			char *key = mmdb_strndup(entry_data.utf8_string, entry_data.data_size);
			if (NULL == key) {
				fprintf( stderr, " Can\'t create new string - out of memory : %s\n",
					MMDB_strerror(status));
				exit_code = 9;
				goto end;
			}
#ifdef DEBUG
			fprintf(stdout, "\"%s\": \n", key);
#endif	// DEBUG
			if ( strcmp(key,whichiso) ) {
				fprintf( stderr, " Database mismatch : looking for %s, found %s\n",whichiso,key);
				exit_code = 10;
				free(key);
				goto end;
			}
			free(key);
		} else {
			fprintf( stderr, " No iso code for this entry - %s\n", MMDB_strerror(status));
			exit_code = 7;
			goto end;
		}

#ifdef DUMP
		if (NULL != entry_data_list) {
			MMDB_dump_entry_data_list(stdout, entry_data_list, 2);
		}
#endif	// DUMP
	} else {
		fprintf( stderr, "\n  No entry for this IP address (%s) was found\n\n", ip_address);
		exit_code = 5;
	}

end:
#ifndef DUMP
		if ( exit_code && (NULL != entry_data_list) ) {
			MMDB_dump_entry_data_list(stderr, entry_data_list, 2);
		}
#endif	// DUMP
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

