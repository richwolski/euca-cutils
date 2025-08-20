#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "simple_input.h"
char Primary[1024];
char Secondary[1024];

#define ARGS "x:y:"

char *Usage = "time_join -x primary-file, -y secondary-file\n";

//
// joins the secondary record that occurs soonest after primary
// first field is assumed to be time stamp 
// data assumed to be in timestamp order
//
// multiple secondaries between primaries are skipped


int main(int argc, char **argv)
{
	int c;
	int fc_p;
	int fc_s;
	int err;
	int err_p;
	void *prim;
	void *sec;
	double *v_p;
	double *v_s;
	int i;

	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'x':
				strncpy(Primary,optarg,sizeof(Primary));
				break;
			case 'y':
				strncpy(Secondary,optarg,sizeof(Secondary));
				break;
			default:
				fprintf(stderr,
					"unrecognized command %c\n",(char)c);
				fprintf(stderr,"usage: %s",Usage);
				exit(1);
		}
	}

	fc_p = GetFieldCount(Primary);
	if(fc_p <= 0) {
		fprintf(stderr,"could not get field count from %s\n",Primary);
		exit(1);
	}
	fc_s = GetFieldCount(Secondary);
	if(fc_s <= 0) {
		fprintf(stderr,"could not get field count from %s\n",Secondary);
		exit(1);
	}

	err = InitDataSet(&prim,fc_p);
	if(err < 0) {
		fprintf(stderr,"could not init from %s\n",Primary);
		exit(1);
	}
	err = InitDataSet(&sec,fc_s);
	if(err < 0) {
		fprintf(stderr,"could not init from %s\n",Secondary);
		exit(1);
	}

	v_p = (double *)malloc(fc_p*sizeof(double));
	if(v_p == NULL) {
		exit(1);
	}

	v_s = (double *)malloc(fc_s*sizeof(double));
	if(v_s == NULL) {
		exit(1);
	}

	err_p = ReadData(prim,fc_p,v_p);
	if(err < 0) {
		fprintf(stderr,"could not read first entry from %s\n",Primary);
		exit(1);
	}
	err = ReadData(sec,fc_s,v_s);
	if(err < 0) {
		fprintf(stderr,"could not read first entry from %s\n",Secondary);
		exit(1);
	}

	while(err_p > 0) {
		// find the secondary immediately after the primary
		while(v_s[0] < v_p[0]) {
			err = ReadData(sec,fc_s,v_s);
			if(err < 0) {
				break;
			}
		}
		if(err < 0) {
			break;
		}
		printf("%10.0f ",v_p[0]);
		for(i=1; i < fc_p; i++) {
			printf("%f ",v_p[i]);
		}
		for(i=1; i < (fc_s-1); i++) {
			printf("%f ",v_s[i]);
		}
		printf("%f\n",v_s[fc_s-1]);
		fflush(stdout);
		// advance primary until it is past the current secondary
		err_p = ReadData(prim,fc_p,v_p);
		if(err_p < 0) {
			break;
		}
		while(v_p[0] < v_s[0]) {
			err_p = ReadData(prim,fc_p,v_p);
			if(err_p < 0) {
				break;
			}
		}
	}

	free(v_p);
	free(v_s);
	exit(1);
}

