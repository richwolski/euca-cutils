#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "simple_input.h"
char Primary[1024];
char Secondary[1024];
int Verbose;
int Predecessor;

#define ARGS "x:y:pV"

char *Usage = "time_join -x primary-file\n\
\t-y secondary-file\n\
\t-p <use nearest predecessor>\n\
\t-V <enable verbose mode>\n";

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
	double *last_s;
	int i;

	while((c = getopt(argc,argv,ARGS)) != EOF) {
		switch(c) {
			case 'x':
				strncpy(Primary,optarg,sizeof(Primary));
				break;
			case 'y':
				strncpy(Secondary,optarg,sizeof(Secondary));
				break;
			case 'p':
				Predecessor = 1;
				break;
			case 'V':
				Verbose = 1;
				break;
			default:
				fprintf(stderr,
					"unrecognized command %c\n",(char)c);
				fprintf(stderr,"usage: %s",Usage);
				exit(1);
		}
	}

	if(Primary[0] == 0) {
		fprintf(stderr,"must specify primary file\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
	}
	if(Secondary[0] == 0) {
		fprintf(stderr,"must specify secondary file\n");
		fprintf(stderr,"%s",Usage);
		exit(1);
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

	err_p = LoadDataSet(Primary,prim);
	if(err_p <= 0) {
		fprintf(stderr,"could not load from %s\n",Primary);
		exit(1);
	}

	err = LoadDataSet(Secondary,sec);
	if(err <= 0) {
		fprintf(stderr,"could not load from %s\n",Secondary);
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

	last_s = (double *)malloc(fc_s*sizeof(double));
	if(last_s == NULL) {
		exit(1);
	}

	err_p = ReadData(prim,fc_p,v_p);
	if(err_p <= 0) {
		fprintf(stderr,"could not read first entry from %s\n",Primary);
		exit(1);
	}
	err = ReadData(sec,fc_s,v_s);
	if(err <= 0) {
		fprintf(stderr,"could not read first entry from %s\n",Secondary);
		exit(1);
	}

	while(err_p > 0) {
		// find the secondary immediately after the primary
		while(v_s[0] < v_p[0]) {
			memcpy(last_s,v_s,fc_s*sizeof(double));
			err = ReadData(sec,fc_s,v_s);
			if(err <= 0) {
				break;
			}
		}
		if(err <= 0) {
			break;
		}
		printf("%10.0f ",v_p[0]);
		for(i=1; i < fc_p; i++) {
			printf("%f ",v_p[i]);
		}
		if(Predecessor == 0) {
			for(i=1; i < (fc_s-1); i++) {
				printf("%f ",v_s[i]);
			}
		} else {
			for(i=1; i < (fc_s-1); i++) {
				printf("%f ",last_s[i]);
			}
		}
		if(Verbose == 0) {
			if(Predecessor == 0) {
				printf("%f\n",v_s[fc_s-1]);
			} else {
				printf("%f\n",last_s[fc_s-1]);
			}
		} else {
			if(Predecessor == 0) {
				printf("%f | ", v_s[fc_s-1]);
				printf("ts: %10.0f ",v_s[0]);
				printf("ts_diff: %f\n",
					v_s[0] - v_p[0]);
			} else {
				printf("%f | ", last_s[fc_s-1]);
				printf("ts: %10.0f ",last_s[0]);
				printf("ts_diff: %f\n",
					v_p[0] - last_s[0]);
			}
		}
		fflush(stdout);
		// advance primary until it is past the current secondary
		err_p = ReadData(prim,fc_p,v_p);
		if(err_p <= 0) {
			break;
		}
		while(v_p[0] < v_s[0]) {
			err_p = ReadData(prim,fc_p,v_p);
			if(err_p <= 0) {
				break;
			}
		}
	}

	free(v_p);
	free(v_s);
	free(last_s);
	exit(1);
}

