#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define PROBEPATH "/sys/bus/w1/devices"  //Location of the probes
#define MAXPROBES 5			 //Max number of probes
#define PROBENAMELEN 80			 //Max length of the probe
#define BUFSIZE 256			 //Typical charbuffer

char probepath[MAXPROBES][PROBENAMELEN]; 
char probename[MAXPROBES][PROBENAMELEN]; 
char alias[MAXPROBES][BUFSIZE];
FILE *probefd;
int numOfSensor;

int findprobes(void)
{
	struct dirent *pDirent;
	DIR *pDir;
	int count;

	count = 0;

	pDir = opendir(PROBEPATH);
	if(pDir == NULL) {
		printf("Cannot open directory '%s'\n", PROBEPATH);
		return 0;
	}

	while((pDirent = readdir(pDir)) != NULL)
	{
		if(pDirent->d_name[0] == '2' && pDirent->d_name[1] == '8' && pDirent->d_name[2] == '-')
		{
			snprintf(probepath[count], PROBENAMELEN-1, "%s/%s/w1_slave", PROBEPATH, pDirent->d_name);
			snprintf(probename[count], PROBENAMELEN-1, "%s", pDirent->d_name);

			printf("Found DS18820 compatible probe named '%s':\nDevice file '%s'\n", probename[count], probepath[count]);
			count++;
		}
	}

	closedir(pDir);

	return count;
}

int main()
{
	int i;
	double temperature;
	char *temp;
	time_t now;
	struct tm *t;
	char buf[BUFSIZE];

	numOfSensor = findprobes(); // 몇개의 센서가 연결되어 있는지 확인하고 그 위치를 저장한다.
	if(numOfSensor == 0)
	{
		printf("Error : No DS18B20 compatible probes located.\n");
		exit(-1);
	}

	while(1)
	{
		for(i=0; i<numOfSensor; i++) //발견된 센서수 만큼 반복
		{
			probefd = fopen(probepath[i], "r"); // probepath에 저장된 위치로부터 센서 값을 읽어 올 수 있

			if(probefd == NULL)
			{
				printf("Error : Unable to open '%s': %s\n", probepath[i], strerror(errno));
				exit(-1);
			}

			fgets(buf, sizeof(buf)-1, probefd);
			memset(buf, 0, sizeof(buf));

			fgets(buf, sizeof(buf)-1, probefd);
			temp = strtok(buf, "t=");
			temp = strtok(NULL, "t=");
			temperature = atof(temp)/1000;

			now = time(NULL);
			t = localtime(&now);

			printf("%s", probename[i]);
			printf("\t%04d-%02d-%02d\t%02d:%02d:%02d\t", t->tm_year + 1900, t->tm_mon+1, t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec );
			printf("%2.3f\n",temperature);

			fclose(probefd);
		}
	}

	return 0;
}
