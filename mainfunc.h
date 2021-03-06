/*
FILE
	$Id: mainfunc.h 1570 2010-10-14 23:55:26Z Gary $
PURPOSE
	Included in main.c. For command line interface and html main link.
AUTHOR/LEGAL
	(C) 2001-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPLv2 license applies. See LICENSE file included.
*/

#include "local.h"
char *strptime(const char *s, const char *format, struct tm *tm);//this is missing from time.h find out why.

//data
extern unsigned guJS;//Global used by bind.c
//The below default should work for you, if not please 
//define cBinDir at tConfiguration
char gcBinDir[100]={"/usr/sbin"};
static char cTableList[32][32]={"tAuthorize","tBlock","tClient","tConfiguration","tDeletedResource","tDeletedZone","tGlossary","tHit","tHitMonth","tJob","tLog","tLogMonth","tLogType","tMailServer","tMonth","tMonthHit","tNS","tNSSet","tNSType","tRRType","tRegistrar","tResource","tResourceImport","tServer","tTemplate","tTemplateSet","tTemplateType","tView","tZone","tZoneImport",""};
char cInitTableList[32][32]={ "tAuthorize","tBlock","tClient","tConfiguration","tGlossary","tLogType","tMailServer","tNS","tNSSet","tNSType","tRRType","tRegistrar","tResource","tServer","tTemplate","tTemplateSet","tTemplateType","tView","tZone",""};


//Please clean up ths TOC proto section.
void ExtMainShell(int argc, char *argv[]);
void Initialize(char *cPasswd);
void Backup(char *cPasswd);
void Restore(char *cPasswd, char *cTableName);
void RestoreAll(char *cPasswd);
void mySQLRootConnect(char *cPasswd);
void ImportTemplateFile(char *cTemplate, char *cFile, char *cTemplateSet, char *cTemplateType);
void ExportTemplateFiles(char *cDir, char *cTemplateSet, char *cTemplateType);
void CalledByAlias(int iArgc,char *cArgv[]);
unsigned TextConnectDb(void);
void Tutorial(void);
void NamedConf(void);
void MasterZones(void);
void Admin(void);
void ListZones(void);
void UpdateSchema(void);
void UpdateTables(void);
void ImportFromDb(char *cSourceDbName, char *cTargetDbName, char *cPasswd);
void MonthHitData(void);
void MonthUsageData(unsigned uSimile);
void DayUsageData(unsigned uLogType);
void NextMonthYear(char *cMonth, char *cYear, char *cNextMonth, char *cNextYear);
void ExtracttLog(char *cMonth, char *cYear, char *cPasswd, char *cTablePath);
void CreatetLogTable(char *cTableName);
void ExtracttHit(char *cMonth, char *cYear, char *cPasswd, char *cTablePath);
void CreatetHitTable(char *cTableName);
void ZeroSystem(void);
void Import(void);
void ImportAxfr(void);
void DropImportedZones(void);
void ImportCompanies(void);
void DropCompanies(void);
void ImportUsers(void);
void DropUsers(void);
void ImportBlocks(void);
void DropBlocks(void);
void AssociateCompaniesZones(void);
void ImportRegistrars(void);
void DropRegistrars(void);
void AssociateRegistrarsZones(void);
void MassZoneUpdate(void);
void MassZoneNSUpdate(char *cLabel);
void FixBlockOwnership(void);
void ImportSORRs(void);
void CheckAllZones(void);
char *cPrintNSList(FILE *zfp,char *cuNSSet);
void PrintMXList(FILE *zfp,char *cuMailServer);
void CreateWebZone(char *cDomain, char *cIP, char *cNSSet, char *cMailServer);
void DropZone(char *cDomain, char *cNSSet);
void CompareZones(char *cDNSServer1IP, char *cDNSServer2IP, char *cuOwner);
void PassDirectHtml(char *file);
void GetConfiguration(const char *cName, char *cValue, unsigned uHtml);
void Initialize(char *cPasswd);
void ExportTable(char *cTable, char *cFile);
time_t cDateToUnixTime(char *cDate);
unsigned NewNSSet(char *cLabel,char *cMasterIPs,char *cuOwner);
void NewNS(char *cFQDN,char *cNSType,unsigned uNSSet);

//extern bind.c protos
void CreateMasterFiles(char *cMasterNS, char *cZone,unsigned uModDBFiles,
		unsigned uModStubs,unsigned uDebug);
void CreateSlaveFiles(char *cSlaveNS,char *cZone,char *cMasterIP,unsigned uDebug);
void InstallNamedFiles(char *cIpNum);
void SlaveJobQueue(char *cNSSet, char *cMasterIP);
void MasterJobQueue(char *cNSSet);
void ServerJobQueue(char *cServer);
void ProcessExtJobQueue(char *cServer);
void ProcessVZJobQueue(void);
void ExportRRCSV(char *cCompany,char *cOutFile);


int iExtMainCommands(pentry entries[], int x)
{
	if(!strcmp(gcFunction,"MainTools"))
	{
		if(!strcmp(gcCommand,"Admin"))
		{
			Admin();
		}
		else if(!strcmp(gcCommand,"NamedConf"))
		{
			NamedConf();
		}
		else if(!strcmp(gcCommand,"MasterZones"))
		{
			MasterZones();
		}
		else if(!strcmp(gcCommand,"Tutorial"))
		{
			Tutorial();
		}
		else if(!strcmp(gcCommand,"Zero System"))
		{
			if(guPermLevel>=12)
				ZeroSystem();
			Admin();
		}
	}
	return(0);

}//int iExtMainCommands(pentry entries[], int x)


void DashBoard(const char *cOptionalMsg)
{
	char cConfigBuffer[256]={""};

	//To handle error messages etc.
	if(cOptionalMsg[0] && strcmp(cOptionalMsg,"DashBoard"))
	{
		printf("%s\n",cOptionalMsg);
		return;
	}

        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	time_t luClock;

	OpenFieldSet("Dashboard",100);
	
	GetConfiguration("allzone.stats",cConfigBuffer,1);
	if(cConfigBuffer[0])
		printf("<img src='%s'>\n",cConfigBuffer);

	cConfigBuffer[0]=0;
	GetConfiguration("mrcstatus",cConfigBuffer,1);
	if(cConfigBuffer[0])
	{
		OpenRow("Replication Status","black");
		printf("<img src='%s'>\n",cConfigBuffer);
	}


	OpenRow("Cluster BIND Errors (Last 10)","black");
	sprintf(gcQuery,"SELECT cMessage,GREATEST(uCreatedDate,uModDate),cServer,cLabel,uPermLevel,uTablePK FROM"
			" tLog WHERE uLogType=5 ORDER BY GREATEST(uCreatedDate,uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td colspan=2><a href=iDNS.cgi?gcFunction=tZone&uZone=%s>%s</a> %s (%s times)"
			"</td><td>%s</td></tr>\n",ctime(&luClock),mysqlField[5],mysqlField[3],
							mysqlField[0],mysqlField[4],mysqlField[2]);
	}
	mysql_free_result(mysqlRes);

	OpenRow("System Messages (Last 10)","black");
	sprintf(gcQuery,"SELECT cMessage,GREATEST(uCreatedDate,uModDate),cServer FROM tLog WHERE uLogType=4"
			" ORDER BY GREATEST(uCreatedDate,uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td colspan=2>%s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[2]);
	}
	mysql_free_result(mysqlRes);

	//1-3 backend org admin interfaces
	OpenRow("General Usage (Last 10)","black");
	sprintf(gcQuery,"SELECT tLog.cLabel,GREATEST(tLog.uCreatedDate,tLog.uModDate),tLog.cLogin,tLog.cTableName,"
			"tLog.cHost,tLogType.cLabel FROM tLog,tLogType WHERE tLog.uLogType=tLogType.uLogType AND"
			" tLog.uLogType<=3 ORDER BY GREATEST(tLog.uCreatedDate,tLog.uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td>%s %s</td><td>%s %s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[3],mysqlField[2],mysqlField[5],mysqlField[4]);
	}
	mysql_free_result(mysqlRes);

	//login/logout activity
	OpenRow("Login Activity (Last 10)","black");
	sprintf(gcQuery,"SELECT tLog.cLabel,GREATEST(tLog.uCreatedDate,tLog.uModDate),tLog.cServer,tLog.cHost,"
			"tLogType.cLabel FROM tLog,tLogType WHERE tLog.uLogType=tLogType.uLogType AND"
			" tLog.uLogType>=6 ORDER BY GREATEST(tLog.uCreatedDate,tLog.uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td>%s %s</td><td>%s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[4],mysqlField[2],mysqlField[3]);
	}
	mysql_free_result(mysqlRes);

	OpenRow("Jobs Pending (Last 10)","black");
	sprintf(gcQuery,"SELECT cJob,GREATEST(uCreatedDate,uModDate),cZone,cTargetServer FROM tJob ORDER BY"
			" GREATEST(uCreatedDate,uModDate) DESC LIMIT 10");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	printf("</td></tr>\n");
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sscanf(mysqlField[1],"%lu",&luClock);
		printf("<td></td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n",
			ctime(&luClock),mysqlField[0],mysqlField[2],mysqlField[3]);
	}
	mysql_free_result(mysqlRes);

	CloseFieldSet();

}//void DashBoard(const char *cOptionalMsg)


void ExtMainContent(void)
{
	printf("<p><input type=hidden name=gcFunction value=MainTools>");

	OpenFieldSet("System Information",100);

	OpenRow("Hostname","black");
	printf("<td>%s</td></tr>\n",gcHostname);

	OpenRow("Build Information","black");
	printf("<td>%s</td></tr>\n",gcBuildInfo);

	OpenRow("Status","black");
	printf("<td>%s %s</td></tr>\n",gcRADStatus,REV);

	OpenRow("Application Summary","black");
	printf("<td>DNS/Bind manager for one or 100s of primary and secondary servers.</td></tr>\n");

	if(guPermLevel>9)
	{
		register unsigned int i;
		OpenRow("Table List","black");
		printf("<td>\n");
		for(i=0;cTableList[i][0];i++)
			printf("<a href=iDNS.cgi?gcFunction=%.32s>%.32s</a><br>\n",
				cTableList[i],cTableList[i]);
		printf("</td></tr>\n");

        	OpenRow("Admin Functions","black");
		printf("<td><input type=hidden name=gcFunction value=MainTools>\n");
		printf("<input class=largeButton type=submit name=gcCommand value=Admin></td></tr>\n");
	}

	CloseFieldSet();

}//void ExtMainContent(void)


void RestoreAll(char *cPasswd)
{
	char cISMROOT[256]={""};
	register int i;

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Restoring iDNS data from .txt file in %s/iDNS/data...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	for(i=0;cTableList[i][0];i++)
	{
		sprintf(gcQuery,"LOAD DATA LOCAL INFILE '%s/iDNS/data/%s.txt' REPLACE INTO TABLE %s",
			cISMROOT,cTableList[i],cTableList[i]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		printf("%s\n",cTableList[i]);
	}

	printf("\nDone\n");

}//void RestoreAll(char *cPasswd)


void Restore(char *cPasswd, char *cTableName)
{
	char cISMROOT[256]={""};

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Restoring iDNS data from .txt file in %s/iDNS/data...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"LOAD DATA LOCAL INFILE '%s/iDNS/data/%s.txt' REPLACE INTO TABLE %s",cISMROOT,cTableName,cTableName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("%s\n\nDone\n",cTableName);

}//void Restore(char *cPasswd, char *cTableName)


void Backup(char *cPasswd)
{
	register int i;
	char cISMROOT[256]={""};

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Backing up iDNS data to .txt files in %s/iDNS/data...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	for(i=0;cTableList[i][0];i++)
	{
		char cFileName[300];

		sprintf(cFileName,"%s/iDNS/data/%s.txt"
				,cISMROOT,cTableList[i]);
		unlink(cFileName);

		sprintf(gcQuery,"SELECT * INTO OUTFILE '%s' FROM %s",cFileName,cTableList[i]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		printf("%s\n",cTableList[i]);
	}


	printf("\nDone.\n");

}//void Backup(char *cPasswd)


void Initialize(char *cPasswd)
{
	char cISMROOT[256]={""};
	register int i;

	if(getuid())
	{
		printf("You must be root to run this command\n");
		exit(0);
	}

	if(getenv("ISMROOT")!=NULL)
		sprintf(cISMROOT,"%.255s",getenv("ISMROOT"));
	if(!cISMROOT[0])
	{
		printf("You must set ISMROOT env var first. Ex. export ISMROOT=/home/joe/unxsVZ\n");
		exit(1);
	}
	printf("Creating db and setting permissions, installing data from %s/iDNS...\n\n",cISMROOT);

	//connect as root to master db
	mySQLRootConnect(cPasswd);

	//Create database
	sprintf(gcQuery,"CREATE DATABASE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("0-. %s\n",mysql_error(&gMysql));
		exit(1);
	}

	//Grant access privileges.
	sprintf(gcQuery,"GRANT ALL ON %s.* to %s@localhost IDENTIFIED BY '%s'",
							DBNAME,DBLOGIN,DBPASSWD);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("1-. %s\n",mysql_error(&gMysql));
		exit(1);
	}
	
	//Change to idns db. Then initialize some tables with needed data
	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("2-. %s\n",mysql_error(&gMysql));
		exit(1);
	}
	
	
	//Create tables and install default data
	CreatetAuthorize();
	CreatetClient();
	CreatetZone();
	CreatetResource();
	CreatetRRType();
	CreatetJob();
	CreatetMailServer();
	CreatetNS();
	CreatetNSSet();
	CreatetNSType();
	CreatetServer();
	CreatetConfiguration();
	CreatetTemplate();
	CreatetTemplateSet();
	CreatetTemplateType();
	CreatetBlock();
	CreatetView();
	CreatetLog();
	CreatetLogType();
	CreatetRegistrar();
	CreatetZoneImport();
	CreatetResourceImport();
	CreatetMonth();
	CreatetLogMonth();
	CreatetDeletedZone();
	CreatetDeletedResource();
	CreatetHit();
	CreatetHitMonth();
	CreatetMonthHit();
	CreatetGlossary();

        for(i=0;cInitTableList[i][0];i++)
        {
                sprintf(gcQuery,"LOAD DATA LOCAL INFILE '%s/iDNS/data/%s.txt' REPLACE INTO TABLE %s",
			cISMROOT,cInitTableList[i],cInitTableList[i]);
                mysql_query(&gMysql,gcQuery);
                if(mysql_errno(&gMysql))
                {
                        printf("%s\n",mysql_error(&gMysql));
                        exit(1);
                }
        }

        printf("Done\n");

}//void Initialize(void)


void mySQLRootConnect(char *cPasswd)
{
        mysql_init(&gMysql);
        if (!mysql_real_connect(&gMysql,NULL,"root",cPasswd,"mysql",0,NULL,0))
        {
		printf("Database server unavailable\n");
		exit(1);
        }

}//void mySQLRootConnect(void)


//Import from local file into tTemplate a single record
void ExportTemplateFiles(char *cDir, char *cTemplateSet, char *cTemplateType)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	FILE *fp;
	char cFile[256];
	unsigned uTemplateSet=0;
	unsigned uTemplateType=0;

	printf("\nExportTemplateFiles(): Start\n");

	TextConnectDb();

	//uTemplateSet
	sprintf(gcQuery,"SELECT uTemplateSet FROM tTemplateSet WHERE cLabel='%s'",cTemplateSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateSet);
	mysql_free_result(mysqlRes);

	if(!uTemplateSet)
	{
		printf("Could not find tTemplateSet.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//uTemplateType
	sprintf(gcQuery,"SELECT uTemplateType FROM tTemplateType  WHERE cLabel='%s'",cTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateType);
	mysql_free_result(mysqlRes);

	if(!uTemplateType)
	{
		printf("Could not find tTemplateType.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//Get cTemplate
	sprintf(gcQuery,"SELECT cLabel,cTemplate FROM tTemplate WHERE uTemplateSet=%u AND uTemplateType=%u",
						uTemplateSet,uTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sprintf(cFile,"%.220s/%.32s",cDir,mysqlField[0]);
		if((fp=fopen(cFile,"w"))!=NULL)
		{
			printf("Writing %s\n",cFile);
			fprintf(fp,"%s",mysqlField[1]);
			fclose(fp);
		}
		else
		{
			printf("Could not open: %s\n",cFile);
			exit(1);
		}
	}
	mysql_free_result(mysqlRes);

	printf("\nExportTemplateFiles(): End\n");

}//ExportTemplateFiles()


void ImportTemplateFile(char *cTemplate, char *cFile, char *cTemplateSet, char *cTemplateType)
{
	FILE *fp;
	unsigned uTemplate=0;
	unsigned uTemplateSet=0;
	unsigned uTemplateType=0;
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
	char cBuffer[2048]={""};

	printf("\nImportTemplateFile(): Start\n");

	TextConnectDb();

	sprintf(gcQuery,"USE %s",DBNAME);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	if((fp=fopen(cFile,"r"))==NULL)
	{
		printf("Could not open %s\n",cFile);
		exit(1);
	}

	//uTemplateSet
	sprintf(gcQuery,"SELECT uTemplateSet FROM tTemplateSet WHERE cLabel='%s'",cTemplateSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateSet);
	mysql_free_result(mysqlRes);

	if(!uTemplateSet)
	{
		printf("Could not find tTemplateSet.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//uTemplateType
	sprintf(gcQuery,"SELECT uTemplateType FROM tTemplateType  WHERE cLabel='%s'",cTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplateType);
	mysql_free_result(mysqlRes);

	if(!uTemplateType)
	{
		printf("Could not find tTemplateType.clabel=%s\n",cTemplateSet);
		exit(1);
	}

	//uTemplate
	sprintf(gcQuery,"SELECT uTemplate FROM tTemplate WHERE cLabel='%s' AND uTemplateSet=%u AND uTemplateType=%u",
						cTemplate,uTemplateSet,uTemplateType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        if((mysqlField=mysql_fetch_row(mysqlRes)))
        	sscanf(mysqlField[0],"%u",&uTemplate);
	mysql_free_result(mysqlRes);


	if(uTemplate)
	{
		printf("Updating tTemplate for %s %s %s\n",cTemplate,cTemplateSet,cTemplateType);
		sprintf(cBuffer,"UPDATE tTemplate SET uModBy=1,uModDate=UNIX_TIMESTAMP(NOW()),cTemplate='',"
				"uTemplateSet=%u,uTemplateType=%u,uModBy=1 WHERE uTemplate=%u",
								uTemplateSet,uTemplateType,uTemplate);
		mysql_query(&gMysql,cBuffer);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n%.254s\n",mysql_error(&gMysql),cBuffer);
			exit(1);
		}
	}
	else
	{
		printf("Inserting new tTemplate for %s\n",cTemplate);
		sprintf(cBuffer,"INSERT INTO tTemplate SET uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),"
				"cLabel='%s',uTemplateSet=%u,uTemplateType=%u",cTemplate,uTemplateSet,uTemplateType);
		mysql_query(&gMysql,cBuffer);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n%.254s\n",mysql_error(&gMysql),cBuffer);
			exit(1);
		}
		uTemplate=mysql_insert_id(&gMysql);
	}

	while(fgets(gcQuery,1024,fp)!=NULL)
	{
		sprintf(cBuffer,"UPDATE tTemplate SET cTemplate=CONCAT(cTemplate,'%s') WHERE uTemplate=%u",
				TextAreaSave(gcQuery),uTemplate);
		mysql_query(&gMysql,cBuffer);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n%.254s\n",mysql_error(&gMysql),cBuffer);
			exit(1);
		}
	}
	fclose(fp);

	printf("\nDone\n");

}//void ImportTemplateFile()


void CalledByAlias(int iArgc,char *cArgv[])
{
	if(strstr(cArgv[0],"iDNSRSS.xml"))
	{
		MYSQL_RES *res;
		MYSQL_ROW field;
		char cRSSDate[200];
		char cLinkStart[200]={""};
           	time_t tTime,luClock;
           	struct tm *tmTime;
		char *cHTTP="http";

		time(&luClock);
           	tTime=time(NULL);
           	tmTime=gmtime(&tTime);
           	strftime(cRSSDate,sizeof(cRSSDate),"%a, %d %b %Y %T GMT",tmTime);

		if(getenv("HTTP_HOST")!=NULL)
			sprintf(gcHost,"%.99s",getenv("HTTP_HOST"));
		if(getenv("HTTPS")!=NULL)
			cHTTP="https";

		//This is the standard place. With much parsing nonsense we can
		//	do better by using env vars
		sprintf(cLinkStart,"%s://%s/cgi-bin/iDNS.cgi",cHTTP,gcHost);

		printf("Content-type: text/xml\n\n");

		//Open xml
		printf("<?xml version='1.0' encoding='UTF-8'?>\n");
		printf("<rss version='2.0'>\n");
		printf("<channel>\n");
		printf("<title>iDNS RSS tJob Errors</title>\n");
		printf("<link>http://openisp.net/iDNS</link>\n");
		printf("<description>iDNS tJob Errors</description>\n");
		printf("<lastBuildDate>%.199s</lastBuildDate>\n",cRSSDate);
		printf("<generator>iDNS RSS Generator</generator>\n");
		printf("<docs>http://blogs.law.harvard.edu/tech/rss</docs>\n");
		printf("<ttl>120</ttl>\n");

		//Loop for each tJob error
		//Connect to local mySQL
		TextConnectDb();
		sprintf(gcQuery,"SELECT uJob,cServer,cJobName,uUser,cJobData FROM tJob WHERE uJobStatus=4");
		//4 is tJobStatus Done Error(s)
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			printf("\n<item>\n");
			printf("<title>iDNS.tJob.uJob=%s</title>\n",field[0]);
			printf("<link>%s?gcFunction=tJob&amp;uJob=%s</link>\n",cLinkStart,
							field[0]);
			printf("<description>cJobName=%s Server=%s uUser=%s\ncJobData=(%s)</description>\n",
				field[2],field[1],field[3],field[4]);
			printf("<guid isPermaLink='false'>%s-%lu</guid>\n",field[0],luClock);
			printf("<pubDate>%.199s</pubDate>\n",cRSSDate);
			printf("</item>\n");

		}
       		mysql_free_result(res);

		//Close xml
		printf("\n</channel>\n");
		printf("</rss>\n");
		mysql_close(&gMysql);
		exit(0);
	}
	else
	{
		printf("Content-type: text/plain\n\n");
		printf("Called as unsupported alias %s\n",cArgv[0]);
		exit(0);
	}

}//void CalledByAlias(int iArgc,char *cArgv[])


void PrintUsage(char *arg0)
{
	printf("iDNS %s CLI Menu (C) 2001-2010 Unixservice, LLC.\n",RELEASE);
	printf("Database Ops:\n");
	printf("\tInitialize <mysql root passwd>\n");
	printf("\tBackup|RestoreAll [<mysql root passwd>]\n");
	printf("\tRestore <Restore table name> [<mysql root passwd>]\n");
	printf("Crontab Ops:\n");
	printf("\tProcessJobQueue <fqdn master ns>\n");
	printf("\tProcessServerJobQueue <fqdn server>\n");
	printf("\tProcessExtJobQueue <fqdn server as in mysqlISP>\n");
	printf("\tProcessVZJobQueue\n");
	printf("\tjobqueue master <fqdn master ns> (equivalent to ProcessJobQueue)\n");
	printf("\tjobqueue slave <fqdn slave ns> <master ip>\n");
	printf("\tMonthHitData\n");
	printf("\tMonthUsageData <uSimile>\n");
	printf("\tDayUsageData <uLogType>\n");
	printf("Special Admin Ops:\n");
	printf("\tDebugMasterFile <fqdn master ns> <cZone>\n");
	printf("\tDebugSlaveFile <fqdn slave ns> <cZone> <master ip>\n");
	printf("\tallfiles master|slave <fqdn ns> <master ip>\n");
	printf("\tinstallbind <listen ipnum>\n");
	printf("\texport <table> <filename>\n");
	printf("\tListZones\n");
	printf("\tPrintNSList <cuNSSet>\n");
	printf("\tPrintMXList <cuMailServer>\n");
	printf("\tCreateWebZone <cDomain> <cIP> [<cNSSet group label> <cMailServer group label>]\n");
	printf("\tDropZone <cDomain> [<cNSSet group label>]\n");
	printf("\tMassZoneUpdate\n");
	printf("\tMassZoneNSUpdate <cNSSet group label>\n");
	printf("\tUpdateSchema\n");
	printf("\tUpdateTables\n");
	printf("\tImportFromDb <source mysql db> <target mysql db> <mysql root passwd>\n");
	printf("\tExtracttLog <Mon> <Year> <mysql root passwd> <path to mysql table>\n");
	printf("\tExtracttHit <Mon> <Year> <mysql root passwd> <path to mysql table>\n");
	printf("\tExample args for Extracts: Apr 2007 passwd /var/lib/mysql/idns\n");
	printf("Special Import/Export Ops (Caution):\n");
	printf("\tImportTemplateFile <tTemplate.cLabel> <filespec> <tTemplateSet.cLabel> <tTemplateType.cLabel>\n");
	printf("\tExportTemplateFiles <dir> <tTemplateSet.cLabel> <tTemplateType.cLabel>\n");
	printf("\tImportZones\n");
	printf("\tImportAxfrZones\n");
	printf("\tDropImportedZones\n");
	printf("\tImportCompanies\n");
	printf("\tDropCompanies\n");
	printf("\tImportUsers\n");
	printf("\tDropUsers\n");
	printf("\tImportBlocks\n");
	printf("\tDropBlocks\n");
	printf("\tAssociateCompaniesZones\n");
	printf("\tImportRegistrars\n");
	printf("\tDropRegistrars\n");
	printf("\tAssociateRegistrarsZones\n");
	printf("\tCompareZones <DNS server1 IP> <DNS server2 IP> [<uOwner>]\n");
	printf("\tImportSORRs\n");
	printf("\tExportRRCSV <company> [out file] <mysql root passwd>\n");
	printf("\tFixBlockOwnership\n");
	exit(0);

}//void PrintUsage(char *arg0)


void ExtMainShell(int argc, char *argv[])
{
	char cCmdLineACL[256]={""};

	//Pre ACL
	if(argc==3 && !strcmp(argv[1],"Initialize"))
	{
		Initialize(argv[2]);
		exit(0);
	}

	if(TextConnectDb())
	{
		GetConfiguration("cCmdLineACL",cCmdLineACL,0);
		if(cCmdLineACL[0])
		{
			sprintf(gcQuery,";%u;",getuid());
			if(!strstr(cCmdLineACL,gcQuery))
			{
				printf("Sorry! You must have permission to use this command.\n");
				exit(1);
			}
		}		
		else
		{
			if(getuid())
			{
				printf("Sorry! must be root to run iDNS.cgi command line.\n");
				exit(1);
			}
		}
	}
	else
	{
		if(getuid())
		{
			printf("Sorry! must be root to run iDNS.cgi command line.\n");
			exit(1);
		}
	}
	mysql_close(&gMysql);


	if(argc==2)
	{
		if(!strcmp(argv[1],"Backup"))
		{
                	Backup("");
			exit(0);
		}
		else if(!strcmp(argv[1],"ListZones"))
		{
                	ListZones();
			exit(0);
		}
        	else if(!strcmp(argv[1],"RestoreAll"))
		{
                	RestoreAll("");
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportZones"))
		{
			TextConnectDb();
			Import();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportAxfrZones"))
		{
			TextConnectDb();
			ImportAxfr();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropImportedZones"))
		{
			TextConnectDb();
			DropImportedZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportCompanies"))
		{
			TextConnectDb();
			ImportCompanies();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropCompanies"))
		{
			TextConnectDb();
			DropCompanies();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportUsers"))
		{
			TextConnectDb();
			ImportUsers();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropUsers"))
		{
			TextConnectDb();
			DropUsers();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportBlocks"))
		{
			TextConnectDb();
			ImportBlocks();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropBlocks"))
		{
			TextConnectDb();
			DropBlocks();
			exit(0);
		}
		else if(!strcmp(argv[1],"AssociateCompaniesZones"))
		{
			TextConnectDb();
			AssociateCompaniesZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportRegistrars"))
		{
			TextConnectDb();
			ImportRegistrars();
			exit(0);
		}
		else if(!strcmp(argv[1],"DropRegistrars"))
		{
			TextConnectDb();
			DropRegistrars();
			exit(0);
		}
		else if(!strcmp(argv[1],"AssociateRegistrarsZones"))
		{
			TextConnectDb();
			AssociateRegistrarsZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"MassZoneUpdate"))
		{
			TextConnectDb();
			MassZoneUpdate();
			exit(0);
		}
		else if(!strcmp(argv[1],"UpdateSchema"))
		{
			TextConnectDb();
			UpdateSchema();
			exit(0);
		}
		else if(!strcmp(argv[1],"UpdateTables"))
		{
			TextConnectDb();
			UpdateTables();
			exit(0);
		}
		else if(!strcmp(argv[1],"MonthHitData"))
		{
			TextConnectDb();
                	MonthHitData();
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportSORRs"))
		{
			TextConnectDb();
			ImportSORRs();
			exit(0);
		}
		else if(!strcmp(argv[1],"CheckAllZones"))
		{
			TextConnectDb();
			CheckAllZones();
			exit(0);
		}
		else if(!strcmp(argv[1],"FixBlockOwnership"))
		{
			TextConnectDb();
			FixBlockOwnership();
			exit(0);
		}
		else if(!strcmp(argv[1],"ProcessVZJobQueue"))
		{
			ProcessVZJobQueue();
			exit(0);
		}
	}
	else if(argc==3)
	{
		if(!strcmp(argv[1],"ProcessJobQueue"))
		{
			MasterJobQueue(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ProcessServerJobQueue"))
		{
			ServerJobQueue(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ProcessExtJobQueue"))
		{
			ProcessExtJobQueue(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"installbind"))
		{
			InstallNamedFiles(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"Initialize"))
		{
			Initialize(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"Backup"))
		{
                	Backup(argv[2]);
			exit(0);
		}
        	else if(!strcmp(argv[1],"RestoreAll"))
		{
                	RestoreAll(argv[2]);
			exit(0);
		}
        	else if(!strcmp(argv[1],"Restore"))
		{
                	Restore(argv[2],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"PrintNSList"))
		{
			TextConnectDb();
                	cPrintNSList(stdout,argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"PrintMXList"))
		{
			TextConnectDb();
                	PrintMXList(stdout,argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"DropZone"))
		{
			TextConnectDb();
			DropZone(argv[2],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"MassZoneNSUpdate"))
		{
			TextConnectDb();
			MassZoneNSUpdate(argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"DayUsageData"))
		{
			unsigned uLogType=0;

			TextConnectDb();
			sscanf(argv[2],"%u",&uLogType);
			if(uLogType)
			{
                		DayUsageData(uLogType);
				exit(0);
			}
		}
		else if(!strcmp(argv[1],"MonthUsageData"))
		{
			unsigned uSimile=0;
			TextConnectDb();
			sscanf(argv[2],"%u",&uSimile);
                	MonthUsageData(uSimile);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	else if(argc==4)
	{
		if(!strcmp(argv[1],"jobqueue"))
		{
			if(!strcmp(argv[2],"master"))
				MasterJobQueue(argv[3]);
		}
		else if(!strcmp(argv[1],"export"))
		{
			ExportTable(argv[2],argv[3]);
		}
        	else if(!strcmp(argv[1],"Restore"))
		{
                	Restore(argv[3],argv[2]);
			exit(0);
		}
		else if(!strcmp(argv[1],"CreateWebZone"))
		{
			TextConnectDb();
			CreateWebZone(argv[2],argv[3],"","");
			exit(0);
		}
		else if(!strcmp(argv[1],"DropZone"))
		{
			TextConnectDb();
			DropZone(argv[2],argv[3]);
			exit(0);
		}
		else if(!strcmp(argv[1],"DebugMasterFile"))
		{
			TextConnectDb();
			CreateMasterFiles(argv[2],argv[3],1,1,1);
			exit(0);
		}
		else if(!strcmp(argv[1],"CompareZones"))
		{
			TextConnectDb();
			CompareZones(argv[2],argv[3],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"ExportRRCSV"))
		{
			mySQLRootConnect(argv[3]);
			ExportRRCSV(argv[2],"");
		}
		PrintUsage(argv[0]);
	}
	else if(argc==5)
	{
		if(!strcmp(argv[1],"allfiles"))
		{
			if(!strcmp(argv[2],"slave"))
			{
				TextConnectDb();
				CreateSlaveFiles(argv[3],"",argv[4],0);//All
				exit(0);
			}
			else if(!strcmp(argv[2],"master"))
			{
				TextConnectDb();
				//All+DBFiles+Stubs
				CreateMasterFiles(argv[3],"",1,1,0);
				exit(0);
			}
			//Note this is missing the optional port and path that bind.c uses
			system("/usr/sbin/rndc -c /etc/unxsbind-rndc.conf reload");
			exit(0);
		}
		else if(!strcmp(argv[1],"jobqueue"))
		{
			if(!strcmp(argv[2],"slave"))
			{
				SlaveJobQueue(argv[3],argv[4]);
				exit(0);
			}
		}
		else if(!strcmp(argv[1],"CreateWebZone"))
		{
			TextConnectDb();
			CreateWebZone(argv[2],argv[3],argv[4],"");
			exit(0);
		}
		else if(!strcmp(argv[1],"DebugSlaveFile"))
		{
			TextConnectDb();
			CreateSlaveFiles(argv[2],argv[3],argv[4],1);
			exit(0);
		}
		else if(!strcmp(argv[1],"CompareZones"))
		{
			TextConnectDb();
			//Optional uOwner to limit zones to compare
			CompareZones(argv[2],argv[3],argv[4]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportFromDb"))
		{
			ImportFromDb(argv[2],argv[3],argv[4]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExportRRCSV"))
		{
			mySQLRootConnect(argv[4]);
			ExportRRCSV(argv[2],argv[3]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExportTemplateFiles"))
		{
                	ExportTemplateFiles(argv[2],argv[3],argv[4]);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	else if(argc==6)
	{
		if(!strcmp(argv[1],"CreateWebZone"))
		{
			TextConnectDb();
			CreateWebZone(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExtracttLog"))
		{
                	ExtracttLog(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ExtracttHit"))
		{
                	ExtracttHit(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		else if(!strcmp(argv[1],"ImportTemplateFile"))
		{
                	ImportTemplateFile(argv[2],argv[3],argv[4],argv[5]);
			exit(0);
		}
		PrintUsage(argv[0]);
	}
	PrintUsage(argv[0]);

}//void ExtMainShell(int argc, char *argv[])


void Tutorial(void)
{
	Header_ism3("Tutorial",0);
	printf("</center><blockquote>");
	PassDirectHtml("idns.tutorial.txt");
	printf("</blockquote>");
	Footer_ism3();

}//void Tutorial(void)


void NamedConf(void)
{
	Header_ism3("NamedConf()",0);
	printf("</center><pre><blockquote>");
	PassDirectHtml("/usr/local/idns/named.conf");
	printf("</blockquote></pre>");
	Footer_ism3();

}//void NamedConf(void)


void MasterZones(void)
{
	Header_ism3("MasterZones()()",0);
	printf("</center><pre><blockquote>");
	PassDirectHtml("/usr/local/idns/named.d/master.zones");
	printf("</blockquote></pre>");
	Footer_ism3();

}//void MasterZones(void)


void Admin(void)
{
	Header_ism3("Admin",0);
	printf("<table width=900><tr><td>");
	printf("<form action=iDNS.cgi method=post>");
	
	printf("<input type=hidden name=gcFunction value=MainTools>");

	if(guPermLevel>7)
	{
		printf("<input title=\"View this system's named.conf file\" class=largeButton type=submit"
			" name=gcCommand value=NamedConf><br>");
		printf("<input title=\"View this system's master.zones file\" class=largeButton type=submit"
			" name=gcCommand value=MasterZones><br>");
		printf("<input title='Tutorial' class=largeButton type=submit name=gcCommand value=Tutorial>");
		if(guPermLevel>11 && guLoginClient==1 )
			printf("<p><input title='DANGER truncates all tables, then installs distro init data from"
			" /usr/local/share/iDNS/data'"
			" class=lwarnButton type=submit name=gcCommand value='Zero System'>");
	}

	printf("</td></tr></table></form>");
	Footer_ism3();

}//void Admin(void)

//List zones for a given uNSSet
void ListZones(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	TextConnectDb();

	sprintf(gcQuery,"SELECT cZone,cLabel FROM tZone,tNSSet WHERE tZone.uNSSet=tNSSet.uNSSet");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res))) 
	{
		//printf("%s %s\n",field[0],field[1]);
		printf("%s\n",field[0]);
	}
	mysql_free_result(res);
	exit(0);

}//void ListZones(void)


void CompareZones(char *cDNSServer1IP, char *cDNSServer2IP, char *cuOwner)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cZone[255]={""};

	if(cuOwner[0])
		sprintf(gcQuery,"SELECT tRRType.cLabel,tResource.cName,tZone.cZone FROM tResource,tRRType,tZone"
				" WHERE tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType AND"
				" tZone.uOwner=%s ORDER BY tZone.cZone",cuOwner);
	else
		sprintf(gcQuery,"SELECT tRRType.cLabel,tResource.cName,tZone.cZone FROM tResource,tRRType,tZone"
				" WHERE tResource.uZone=tZone.uZone AND tResource.uRRType=tRRType.uRRType ORDER BY"
				" tZone.cZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res))) 
	{
		if(strcmp(cZone,field[2]))
		{
			strcpy(cZone,field[2]);
			printf("%s\n",cZone);
		}

		if(field[1][0])
			sprintf(gcQuery,"dig @%s %s %s.%s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz1 2>&1",
					cDNSServer1IP,field[0],field[1],field[2]);
		else
			sprintf(gcQuery,"dig @%s %s %s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz1 2>&1",
					cDNSServer1IP,field[0],field[2]);
		system(gcQuery);
		//printf("%s\n",gcQuery);

		if(field[1][0])
			sprintf(gcQuery,"dig @%s %s %s.%s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz2 2>&1",
					cDNSServer2IP,field[0],field[1],field[2]);
		else
			sprintf(gcQuery,"dig @%s %s %s +noall +answer | awk '{print $1,$3,$4,$5,$6,$7,$8}'"
					" > /tmp/idns.cz2 2>&1",
					cDNSServer2IP,field[0],field[2]);
		system(gcQuery);
		//printf("%s\n",gcQuery);

		if(system("diff --ignore-all-space /tmp/idns.cz1 /tmp/idns.cz2 > /dev/null 2>&1"))
		{
			system("cat /tmp/idns.cz1 /tmp/idns.cz2");
		}
			
	}
	mysql_free_result(res);

}//void CompareZones()


void UpdateSchema(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned ucClrPasswd=1;
	unsigned ucMessage=1;
	unsigned ucServer=1;
	unsigned uNameServer=0;
	unsigned uNSSet=1;
	unsigned uZoneImportNSSet=1;
	unsigned uViewNSSet=1;
	unsigned uJobNSSet=1;
	unsigned uDeletedZoneNSSet=1;
	unsigned uClient=1;
	unsigned uClientDel=1;
	unsigned ucParam3=1;
	unsigned ucParam4=1;

	printf("UpdateSchema() start\n");

	//Gather current schema info for new columns and new or complex index mods/changes
	//Modifies do nothing bad but in the future for the sake of efficiency we should
	//	also not repeat them.
	//Organized by table not in alphabetical order yet TODO


	//tAuthorize
	sprintf(gcQuery,"SHOW COLUMNS IN tAuthorize");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
		if(!strcmp(field[0],"cClrPasswd")) ucClrPasswd=0;
       	mysql_free_result(res);
	if(ucClrPasswd)
	{
		sprintf(gcQuery,"ALTER TABLE tAuthorize ADD cClrPasswd VARCHAR(32) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	sprintf(gcQuery,"ALTER TABLE tAuthorize MODIFY cPasswd VARCHAR(35) NOT NULL DEFAULT ''");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		printf("%s\n",mysql_error(&gMysql));

	//tView
	sprintf(gcQuery,"SHOW COLUMNS IN tView");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uViewNSSet=0;
	}
       	mysql_free_result(res);
	if(uViewNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tView ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tLog
	sprintf(gcQuery,"SHOW COLUMNS IN tLog");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"cMessage")) ucMessage=0;
		else if(!strcmp(field[0],"cServer")) ucServer=0;
	}
       	mysql_free_result(res);
	if(ucMessage)
	{
		sprintf(gcQuery,"ALTER TABLE tLog ADD cMessage VARCHAR(255) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(ucServer)
	{
		sprintf(gcQuery,"ALTER TABLE tLog ADD cServer VARCHAR(64) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tHit
	sprintf(gcQuery,"ALTER TABLE tHit MODIFY cZone VARCHAR(255) NOT NULL DEFAULT ''");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		printf("%s\n",mysql_error(&gMysql));

	//tJob
	sprintf(gcQuery,"SHOW COLUMNS IN tJob");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uJobNSSet=0;
	}
       	mysql_free_result(res);
	if(uJobNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tJob ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tZoneImport
	sprintf(gcQuery,"SHOW COLUMNS IN tZoneImport");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uZoneImportNSSet=0;
	}
       	mysql_free_result(res);
	if(uZoneImportNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tZoneImport ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tZone
	sprintf(gcQuery,"SHOW COLUMNS IN tZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uNSSet")) uNSSet=0;
		else if(!strcmp(field[0],"uClient")) uClient=0;
		else if(!strcmp(field[0],"uNameServer")) uNameServer=1;//Note rev logic
	}
       	mysql_free_result(res);
	if(uNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tZone ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(uClient)
	{
		sprintf(gcQuery,"ALTER TABLE tZone ADD uClient INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tDeletedZone
	sprintf(gcQuery,"SHOW COLUMNS IN tDeletedZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"uClient")) uClientDel=0;
		else if(!strcmp(field[0],"uNSSet")) uDeletedZoneNSSet=0;
	}
       	mysql_free_result(res);
	if(uClientDel)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedZone ADD uClient INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(uDeletedZoneNSSet)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedZone ADD uNSSet INT UNSIGNED NOT NULL DEFAULT 0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}

	//tDeletedResource
	sprintf(gcQuery,"SHOW COLUMNS IN tDeletedResource");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"cParam3")) ucParam3=0;
		else if(!strcmp(field[0],"cParam4")) ucParam4=0;
	}
       	mysql_free_result(res);
	if(ucParam3)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedResource ADD cParam3 VARCHAR(255) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	if(ucParam4)
	{
		sprintf(gcQuery,"ALTER TABLE tDeletedResource ADD cParam4 VARCHAR(255) NOT NULL DEFAULT ''");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			printf("%s\n",mysql_error(&gMysql));
		printf("%s\n",gcQuery);
	}
	
	//
	//Upgrading from very old pre tNSSet versions
	//Check for tNameServer
	unsigned utNameServer=0;
	sprintf(gcQuery,"SHOW TABLES");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	while((field=mysql_fetch_row(res)))
	{
		if(!strcmp(field[0],"tNameServer")) utNameServer=1;
	}
       	mysql_free_result(res);
	if(utNameServer && uNameServer)//uNameServer set in tZone check above
	{
		unsigned uNSSet=0;

		printf("\tStarting conversion to tNSSet version from tNameServer data\n");
		sprintf(gcQuery,"SELECT cLabel,cList,cMasterIPs,uOwner FROM tNameServer");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			printf("\t\t%s\n",field[0]);
			//Create single new tNSSet from cLabel
			uNSSet=NewNSSet(field[0],field[2],field[3]);

			//Create tNS entries from cList
			if(field[1][0])
			{
				register int i,j=0;
				char cFQDN[100];
				char cNSType[33];
				char *cp;

				//Parse each line get string for tNS.cFQDN and uNSType
				for(i=0;field[1][i];i++)
				{
					cNSType[0]=0;
					cFQDN[j++]=field[1][i];
					if(field[1][i]=='\r' || field[1][i]=='\n')
					{
						cFQDN[j-1]=0;
						if((cp=strchr(cFQDN,' ')))
						{
							*cp=0;
							sprintf(cNSType,"%.32s",cp+1);
						}
						printf("\t\t\t%s %s\n",cFQDN,cNSType);
						NewNS(cFQDN,cNSType,uNSSet);
						j=0;
					}
				}
				if(cFQDN[j])
				{
					cFQDN[j-1]=0;
					if((cp=strchr(cFQDN,' ')))
					{
						*cp=0;
						sprintf(cNSType,"%.32s",cp+1);
					}
					printf("\t\t\t%s %s\n",cFQDN,cNSType);
					NewNS(cFQDN,cNSType,uNSSet);
				}
			}
			//Not sure yet about cMasterIPs
		}
	       	mysql_free_result(res);

		//Set tZone.uNSSet from tZone.uNameServer
		sprintf(gcQuery,"SELECT tZone.uZone,tNSSet.uNSSet FROM tZone,tNameServer,tNSSet"
				" WHERE tZone.uNameServer=tNameServer.uNameServer AND tNameServer.cLabel=tNSSet.cLabel"
				" AND tZone.uNSSet=0");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		res=mysql_store_result(&gMysql);
		while((field=mysql_fetch_row(res)))
		{
			sprintf(gcQuery,"UPDATE tZone SET uNSSet=%s WHERE uZone=%s",field[1],field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				printf("%s\n",mysql_error(&gMysql));
				exit(1);
			}
		}
	       	mysql_free_result(res);

		printf("\tEnd of tNameServer based conversion\n");
	}
	printf("UpdateSchema() end\n");

}//void UpdateSchema(void)


void UpdateTables(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uMax=0;
	char cCommand[256];
	char *cDBIP="localhost";
	char cSQLPath[100]={"/usr/local/share/iDNS/data"};

	//1-. We need a way to update fixed tables like tRRType
	//without disruption. Small tables can be updated directly
	//via fixed queries, but that seems like bloat to me.

	//2-. We need a way to add new template sets, like for vdnsOrg
	//for example. This can be done with special data/*.sql files.
	//These files can be included in source distributions and also
	//provided by rpm/yum. But how to we make sure we don't update
	//more than once? We can check here and then do a system call to mysql.

	//3-. This should be a first step to deprecating the data/*.txt
	//old mess we inherited 10 years ago.

	//ISMROOT should also be changed to unxsVZDataPath or similar.

	//DBIP0 must be up an running OR localhost must be up and running.
	if(DBIP0!=NULL && DBIP1!=NULL)
		cDBIP=DBIP0;

	if(getenv("ISMROOT")!=NULL)
		sprintf(cSQLPath,"%.99s/iDNS/data",getenv("ISMROOT"));
	//Else we use the unxsbind.spec path above

	printf("UpdateTables(%s) start\n",cDBIP);

	//tGlossary always updated
	sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tGlossary.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
	if(system(cCommand))
	{
		printf("Error: %s\n",cCommand);
		exit(1);
	}
	printf("Updated tGlossary.\n");

	//tRRType
	//see mysqlrad.h defines
	uMax=RRTYPE_NAPTR;
	sprintf(gcQuery,"SELECT MAX(uRRType) FROM tRRType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<RRTYPE_NAPTR)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tRRType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tRRType.\n");
	}

	//tNSType
#define NSTYPE_MASTER 1
#define NSTYPE_MHIDDEN 2
#define NSTYPE_MEXTERN 3
#define NSTYPE_SLAVE 4
	uMax=NSTYPE_SLAVE;
	sprintf(gcQuery,"SELECT MAX(uNSType) FROM tNSType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<NSTYPE_SLAVE)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tNSType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tNSType.\n");
	}

	//tLogType
#define LOGTYPE_IDNSORG_LOGIN 8
	uMax=LOGTYPE_IDNSORG_LOGIN;
	sprintf(gcQuery,"SELECT MAX(uLogType) FROM tLogType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<LOGTYPE_IDNSORG_LOGIN)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tLogType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tLogType.\n");
	}


	//tTemplateType
#define TEMPLATETYPE_VDNSORG 3
	uMax=TEMPLATETYPE_VDNSORG;
	sprintf(gcQuery,"SELECT MAX(uTemplateType) FROM tTemplateType");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<TEMPLATETYPE_VDNSORG)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplateType.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplateType.\n");
	}

	//tTemplateSet
#define TEMPLATESET_PLAIN 1
	uMax=TEMPLATESET_PLAIN;
	sprintf(gcQuery,"SELECT MAX(uTemplateSet) FROM tTemplateSet");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	if(uMax<TEMPLATESET_PLAIN)
	{
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplateSet.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplateSet.\n");
	}

	//
	//tTemplate updates --much harder problem
	//For initial testing we will keep old no type (and probably no set) templates
	//The end user will have to deal with his mods post initial install accordingly.
	//tTemplate-vdnsOrg.sql example dump:
	//mysqldump --compact --no-create-info --where="uTemplateType=3" -psecret idns tTemplate
	//					 > data/tTemplate-vdnsOrg.sql
	//see mysqlrad.h defines
	uMax=TEMPLATETYPE_VDNSORG;
	sprintf(gcQuery,"SELECT MAX(uTemplateType) FROM tTemplate");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uMax);
       	mysql_free_result(res);
	//We are assuming that for an rpm update user, if user does not have vdnsOrg templates, that this 
	//means the user needs all the other template tables updated also.
	//Any old template will still exist but will not have a type and probably not a set either
	//unless they modified them themselves.
	if(uMax<TEMPLATETYPE_VDNSORG)
	{
		//Save old templates, this is pretty much a temp hack.
		//It is needed since the .sql files have uTemplate values.
		sprintf(gcQuery,"UPDATE tTemplate SET uTemplate=uTemplate+1000");
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}

		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplate-vdnsOrg.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplate-vdnsOrg.\n");
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplate-idnsOrg.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplate-idnsOrg. Old idnsOrg templates can still be found in tTemplate.\n");
		sprintf(cCommand,"/usr/bin/mysql -h %.64s -u %.32s -p%.32s %.32s < %.99s/tTemplate-idnsAdmin.sql",
					cDBIP,
					DBLOGIN,
					DBPASSWD,
					DBNAME,	
					cSQLPath);
		if(system(cCommand))
		{
			printf("Error: %s\n",cCommand);
			exit(1);
		}
		printf("Updated tTemplate-idnsAdmin. Old idnsAdmin templates can still be found in tTemplate.\n");
	}

	printf("UpdateTables() end\n");

}//void UpdateTables(void)


void ExtracttLog(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)
{
	char cTableName[33]={""};
	char cNextMonth[4]={""};
	char cNextYear[8]={""};
	char cThisYear[8]={""};
	char cThisMonth[4]={""};
	unsigned uRows=0;
	time_t clock, uStart=0, uEnd=0;
	struct tm *structTime;
	struct stat info;

	time(&clock);
	structTime=localtime(&clock);
	strftime(cThisYear,8,"%Y",structTime);
	strftime(cThisMonth,4,"%b",structTime);

	printf("ExtracttLog() Start\n");

	printf("cThisMonth:%s cThisYear:%s\n",cThisMonth,cThisYear);

	if(!strcmp(cThisMonth,cMonth) && !strcmp(cThisYear,cYear))
	{
		fprintf(stderr,"Can't extract this months data!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisampack",&info) )
	{
		fprintf(stderr,"/usr/bin/myisampack is not installed!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisamchk",&info))
	{
		fprintf(stderr,"/usr/bin/myisamchk is not installed!\n");
		exit(1);
	}


	mySQLRootConnect(cPasswd);
	mysql_query(&gMysql,"USE idns");
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttLog() Start...',cServer='%s',uLogType=4,uOwner=1,"
				"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));

	sprintf(cTableName,"t%.3s%.7s",cMonth,cYear);

	sprintf(gcQuery,"1-%s-%s",cMonth,cYear);
	uStart=cDateToUnixTime(gcQuery);
	printf("Start: %s",ctime(&uStart));

	if(uStart== -1 || !uStart)
	{
		fprintf(stderr,"Garbled month year input (uStart)\n");
		exit(1);
	}

	NextMonthYear(cMonth,cYear,cNextMonth,cNextYear);
	//Debug only
	//printf("%s %s to %s %s\n",cMonth,cYear,cNextMonth,cNextYear);
	//exit(0);

	sprintf(gcQuery,"1-%s-%s",cNextMonth,cNextYear);
	uEnd=cDateToUnixTime(gcQuery);
	printf("End:   %s",ctime(&uEnd));
	if(uEnd== -1 || !uEnd)
	{
		fprintf(stderr,"Garbled month year input (uEnd)\n");
		exit(1);
	}

	
	CreatetLogTable(cTableName);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Clearing data from %s...\n",cTableName);
	sprintf(gcQuery,"DELETE FROM %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Getting data from tLog...\n");
	sprintf(gcQuery,"INSERT %s (uLog,cLabel,uLogType,cHash,uPermLevel,uLoginClient,cLogin,cHost,uTablePK,cTableName,"
			"uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate) SELECT uLog,cLabel,uLogType,cHash,uPermLevel,"
			"uLoginClient,cLogin,cHost,uTablePK,cTableName,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate FROM"
			" tLog WHERE uCreatedDate>=%lu AND uCreatedDate<%lu",cTableName,uStart,uEnd);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	//If 0 records inserted delete from tMonth and exit now
	uRows=mysql_affected_rows(&gMysql);

	printf("Number of rows found and inserted: %u\n",uRows);

	if(uRows)
	{

		//This looks suspect, please check TODO
		sprintf(gcQuery,"REPLACE tMonth SET cLabel='%s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
			cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	else
	{
		sprintf(gcQuery,"DELETE FROM tMonth WHERE cLabel='%s'",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}

		sprintf(gcQuery,"DROP TABLE %s",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}


		printf("Exiting early no data to be archived\n");
		exit(0);
	}

	sprintf(gcQuery,"/usr/bin/myisampack %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	sprintf(gcQuery,"/usr/bin/myisamchk -rq %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	printf("Flushing compressed RO table...\n");
	sprintf(gcQuery,"FLUSH TABLE %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Removing records from tLog...\n");
	sprintf(gcQuery,"DELETE QUICK FROM tLog WHERE uCreatedDate>=%lu AND uCreatedDate<%lu AND uLogType!=5",uStart,uEnd);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}


	printf("Extracted and Archived. Table flushed: %s\n",cTableName);
	printf("ExtracttLog() End\n");
	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttLog() End',cServer='%s',uLogType=4,uOwner=1,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
	exit(0);

}//void ExtracttLog(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)


time_t cDateToUnixTime(char *cDate)
{
        struct  tm locTime;
        time_t  res;

        bzero(&locTime, sizeof(struct tm));
	if(strchr(cDate,'-'))
        	strptime(cDate,"%d-%b-%Y", &locTime);
	else if(strchr(cDate,'/'))
        	strptime(cDate,"%d/%b/%Y", &locTime);
	else if(strchr(cDate,' '))
        	strptime(cDate,"%d %b %Y", &locTime);
        locTime.tm_sec = 0;
        locTime.tm_min = 0;
        locTime.tm_hour = 0;
        res = mktime(&locTime);
        return(res);
}//time_t cDateToUnixTime(char *cDate)


//Another schema dependent item
void CreatetLogTable(char *cTableName)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS %s ( uTablePK VARCHAR(32) NOT NULL DEFAULT '', cHost VARCHAR(32) NOT NULL DEFAULT '', uLoginClient INT UNSIGNED NOT NULL DEFAULT 0, cLogin VARCHAR(32) NOT NULL DEFAULT '', uPermLevel INT UNSIGNED NOT NULL DEFAULT 0, cTableName VARCHAR(32) NOT NULL DEFAULT '', uLog INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cLabel VARCHAR(64) NOT NULL DEFAULT '', uOwner INT UNSIGNED NOT NULL DEFAULT 0,index (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, cHash VARCHAR(32) NOT NULL DEFAULT '', uLogType INT UNSIGNED NOT NULL DEFAULT 0,index (uLogType) )",cTableName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
}//CreatetLogTable()


void ExtracttHit(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)
{
	char cTableName[33]={""};
	char cNextMonth[4]={""};
	char cNextYear[8]={""};
	char cThisYear[8]={""};
	char cThisMonth[4]={""};
	unsigned uRows=0;
	time_t clock, uStart=0, uEnd=0;
	struct tm *structTime;
	struct stat info;

	time(&clock);
	structTime=localtime(&clock);
	strftime(cThisYear,8,"%Y",structTime);
	strftime(cThisMonth,4,"%b",structTime);

	printf("ExtracttHit() Start\n");
	printf("cThisMonth:%s cThisYear:%s\n",cThisMonth,cThisYear);

	if(!strcmp(cThisMonth,cMonth) && !strcmp(cThisYear,cYear))
	{
		fprintf(stderr,"Can't extract this months data!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisampack",&info) )
	{
		fprintf(stderr,"/usr/bin/myisampack is not installed!\n");
		exit(1);
	}

	if(stat("/usr/bin/myisamchk",&info))
	{
		fprintf(stderr,"/usr/bin/myisamchk is not installed!\n");
		exit(1);
	}


	mySQLRootConnect(cPasswd);
	mysql_query(&gMysql,"USE idns");
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttHit() Start...',cServer='%s',uLogType=4,uOwner=1,"
			"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));

	sprintf(cTableName,"t%.3s%.7sHit",cMonth,cYear);

	sprintf(gcQuery,"1-%s-%s",cMonth,cYear);
	uStart=cDateToUnixTime(gcQuery);
	printf("Start: %s",ctime(&uStart));

	if(uStart== -1 || !uStart)
	{
		fprintf(stderr,"Garbled month year input (uStart)\n");
		exit(1);
	}

	NextMonthYear(cMonth,cYear,cNextMonth,cNextYear);
	//Debug only
	//printf("%s %s to %s %s\n",cMonth,cYear,cNextMonth,cNextYear);
	//exit(0);

	sprintf(gcQuery,"1-%s-%s",cNextMonth,cNextYear);
	uEnd=cDateToUnixTime(gcQuery);
	printf("End:   %s",ctime(&uEnd));
	if(uEnd== -1 || !uEnd)
	{
		fprintf(stderr,"Garbled month year input (uEnd)\n");
		exit(1);
	}

	CreatetHitTable(cTableName);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Clearing (truncate) data from %s...\n",cTableName);
	sprintf(gcQuery,"TRUNCATE %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Getting data from tHit...\n");
	sprintf(gcQuery,"INSERT %s (uHit,cZone,uHitCount,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate,uSuccess,"
			"uReferral,uNxrrset,uNxdomain,uRecursion,uFailure,cHost) SELECT uHit,cZone,uHitCount,uOwner,"
			"uCreatedBy,uCreatedDate,uModBy,uModDate,uSuccess,uReferral,uNxrrset,uNxdomain,uRecursion,"
			"uFailure,cHost FROM tHit",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	//If 0 records inserted delete from tMonth and exit now
	uRows=mysql_affected_rows(&gMysql);

	printf("Number of rows found and inserted: %u\n",uRows);

	if(uRows)
	{

		//TODO check this
		sprintf(gcQuery,"REPLACE tMonthHit SET cLabel='%s',uOwner=1,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	else
	{
		sprintf(gcQuery,"DELETE FROM tMonthHit WHERE cLabel='%s'",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}

		sprintf(gcQuery,"DROP TABLE %s",cTableName);
        	mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			exit(1);
		}


		printf("Exiting early no data to be archived\n");
		exit(0);
	}

	sprintf(gcQuery,"/usr/bin/myisampack %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	sprintf(gcQuery,"/usr/bin/myisamchk -rq %s/%s",cTablePath,cTableName);
	if(system(gcQuery))
	{
		fprintf(stderr,"Failed: %s\n",gcQuery);
		exit(1);
	}

	printf("Flushing compressed RO table...\n");
	sprintf(gcQuery,"FLUSH TABLE %s",cTableName);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Removing records from tHit...\n");
	sprintf(gcQuery,"TRUNCATE tHit");
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		exit(1);
	}

	printf("Extracted and Archived. Table flushed: %s\n",cTableName);
	printf("ExtracttHit() End\n");
	sprintf(gcQuery,"INSERT INTO tLog SET cMessage='ExtracttHit() End',cServer='%s',uLogType=4,uOwner=1,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW())",gcHostname);
        mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
	exit(0);

}//void ExtracttHit(char *cMonth, char *cYear, char *cPasswd, char *cTablePath)


//schema dependency created here
void CreatetHitTable(char *cTableName)
{
	sprintf(gcQuery,"CREATE TABLE IF NOT EXISTS %s ( uHit INT UNSIGNED PRIMARY KEY AUTO_INCREMENT, cZone VARCHAR(255) NOT NULL DEFAULT '',INDEX (cZone), uOwner INT UNSIGNED NOT NULL DEFAULT 0,INDEX (uOwner), uCreatedBy INT UNSIGNED NOT NULL DEFAULT 0, uCreatedDate INT UNSIGNED NOT NULL DEFAULT 0, uModBy INT UNSIGNED NOT NULL DEFAULT 0, uModDate INT UNSIGNED NOT NULL DEFAULT 0, uHitCount BIGINT UNSIGNED NOT NULL DEFAULT 0, uSuccess BIGINT UNSIGNED NOT NULL DEFAULT 0, uReferral BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxrrset BIGINT UNSIGNED NOT NULL DEFAULT 0, uNxdomain BIGINT UNSIGNED NOT NULL DEFAULT 0, uRecursion BIGINT UNSIGNED NOT NULL DEFAULT 0, uFailure BIGINT UNSIGNED NOT NULL DEFAULT 0, cHost VARCHAR(255) NOT NULL DEFAULT '',INDEX (cHost) )",cTableName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
}//CreatetHitTable()


void NextMonthYear(char *cMonth,char *cYear,char *cNextMonth,char *cNextYear)
{
	unsigned uYear=0;

	//Preset for all but Dec cases
	sprintf(cNextYear,"%.7s",cYear);
	sscanf(cNextYear,"%u",&uYear);

	if(!strcmp(cMonth,"Jan"))
	{
		strcpy(cNextMonth,"Feb");
	}
	else if(!strcmp(cMonth,"Feb"))
	{
		strcpy(cNextMonth,"Mar");
	}
	else if(!strcmp(cMonth,"Mar"))
	{
		strcpy(cNextMonth,"Apr");
	}
	else if(!strcmp(cMonth,"Apr"))
	{
		strcpy(cNextMonth,"May");
	}
	else if(!strcmp(cMonth,"May"))
	{
		strcpy(cNextMonth,"Jun");
	}
	else if(!strcmp(cMonth,"Jun"))
	{
		strcpy(cNextMonth,"Jul");
	}
	else if(!strcmp(cMonth,"Jul"))
	{
		strcpy(cNextMonth,"Aug");
	}
	else if(!strcmp(cMonth,"Aug"))
	{
		strcpy(cNextMonth,"Sep");
	}
	else if(!strcmp(cMonth,"Sep"))
	{
		strcpy(cNextMonth,"Oct");
	}
	else if(!strcmp(cMonth,"Oct"))
	{
		strcpy(cNextMonth,"Nov");
	}
	else if(!strcmp(cMonth,"Nov"))
	{
		strcpy(cNextMonth,"Dec");
	}
	else if(!strcmp(cMonth,"Dec"))
	{
		strcpy(cNextMonth,"Jan");
		uYear++;
		sprintf(cNextYear,"%u",uYear);
	}

}//NextMonthYear()


void MonthHitData(void)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
        MYSQL_RES *mysqlRes2;
        MYSQL_ROW mysqlField2;

	printf("#MonthHitData designed for use with gnuplot\n");

	sprintf(gcQuery,"SELECT cLabel FROM tMonthHit ORDER BY uMonth");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		sprintf(gcQuery,"SELECT COUNT(uHit),(SUM(uHitCount) DIV 1000) FROM %s",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
		{
			//tApr2007Hit
			printf("%.3s-%.2s\t%s\t%s\n",mysqlField[0]+1,
				mysqlField[0]+6,mysqlField2[0],mysqlField2[1]);
		}
		mysql_free_result(mysqlRes2);
	}
	mysql_free_result(mysqlRes);

}//void MonthHitData(void)


void MonthUsageData(unsigned uSimile)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;
        MYSQL_RES *mysqlRes2;
        MYSQL_ROW mysqlField2;
	unsigned uNewCount;
	unsigned uModCount;
	unsigned uDelCount;
	unsigned uBackendCount,uOrgCount,uAdminCount,uErrorCount;

	if(uSimile>1)
		printf("Content-type: text/plain\n\n");

	printf("#MonthUsageData designed for use with gnuplot or MIT Simile\n");
	//2008-03-10,32,63,5 --Simile format
	if(uSimile)
		printf("#Year-Mon-Day uNewCount uModCount uDelCount uTotal"
				" uBackendCount uOrgCount uAdminCount uErrorCount\n");
	else
		printf("#Mon-Yr uNewCount uModCount uDelCount uTotal"
				" uBackendCount uOrgCount uAdminCount uErrorCount\n");


	sprintf(gcQuery,"SELECT cLabel,FROM_UNIXTIME(uCreatedDate-3600,'%%Y-%%m-%%d') FROM tMonth ORDER BY uMonth");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
        while((mysqlField=mysql_fetch_row(mysqlRes)))
	{
		uNewCount=0;
		uModCount=0;
		uDelCount=0;
		uBackendCount=0;
		uOrgCount=0;
		uAdminCount=0;
		uErrorCount=0;

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE cLabel='New'",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uNewCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE cLabel='Mod'",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uModCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE cLabel='Del'",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uDelCount);
		mysql_free_result(mysqlRes2);

		//Totals per interface
		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=1",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uBackendCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=2",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uOrgCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=3",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uAdminCount);
		mysql_free_result(mysqlRes2);

		sprintf(gcQuery,"SELECT COUNT(uLog) FROM %s WHERE uLogType=5",mysqlField[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes2=mysql_store_result(&gMysql);
        	if((mysqlField2=mysql_fetch_row(mysqlRes2)))
			sscanf(mysqlField2[0],"%u",&uErrorCount);
		mysql_free_result(mysqlRes2);

		//Output row
		//tApr2007 --> Apr-07
		//2008-03-10,32,63,5
		if(uSimile)
		printf("%s,%u,%u,%u,%u,%u,%u,%u,%u\n", mysqlField[1],
				uNewCount,uModCount,uDelCount,uNewCount+uModCount+uDelCount,
				uBackendCount,uOrgCount,uAdminCount,uErrorCount);
		else
		printf("%.3s-%.2s \t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
				mysqlField[0]+1,mysqlField[0]+6,
				uNewCount,uModCount,uDelCount,uNewCount+uModCount+uDelCount,
				uBackendCount,uOrgCount,uAdminCount,uErrorCount);
	}
	mysql_free_result(mysqlRes);

}//void MonthUsageData(void)


void DayUsageData(unsigned uLogType)
{
        MYSQL_RES *mysqlRes;
        MYSQL_ROW mysqlField;

	//This SQL is only good for mySQL 4.1+

	if(uLogType>100)
	{
		printf("Content-type: text/plain\n\n");
		printf("#DayUsageData designed for use with MIT Simile, uLogType=%u\n",uLogType);
		printf("#Year-Mon-Day COUNT(uLog)\n");
		sprintf(gcQuery,"SELECT FROM_UNIXTIME(uCreatedDate,'%%Y-%%m-%%d'),COUNT(uLog) FROM tLog"
				" WHERE uLogType=%u AND MONTH(FROM_UNIXTIME(uCreatedDate))=MONTH(NOW()) GROUP BY"
				" DAY(FROM_UNIXTIME(uCreatedDate)) ORDER BY uCreatedDate",uLogType-100);
	}
	else
	{
		printf("#DayUsageData designed for use with gnuplot, uLogType=%u\n",uLogType);
		printf("#Day-Mon-Yr COUNT(uLog)\n");
		sprintf(gcQuery,"SELECT FROM_UNIXTIME(uCreatedDate,'%%d-%%m-%%y'),COUNT(uLog) FROM tLog"
				" WHERE uLogType=%u AND MONTH(FROM_UNIXTIME(uCreatedDate))=MONTH(NOW()) GROUP BY"
				" DAY(FROM_UNIXTIME(uCreatedDate)) ORDER BY uCreatedDate",uLogType);
	}

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
        mysqlRes=mysql_store_result(&gMysql);
	if(mysql_num_rows(mysqlRes))
	{
        	while((mysqlField=mysql_fetch_row(mysqlRes)))
			printf("%s %s\n",mysqlField[0],mysqlField[1]);
	}
	else
	{
		//No data SELECT FROM_UNIXTIME(UNIX_TIMESTAMP(NOW()),'%d-%m-%y')
		if(uLogType>100)
			sprintf(gcQuery,"SELECT FROM_UNIXTIME(UNIX_TIMESTAMP(NOW()),'%%Y-%%m-%%d')");
		else
			sprintf(gcQuery,"SELECT FROM_UNIXTIME(UNIX_TIMESTAMP(NOW()),'%%d-%%m-%%y')");

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
        	mysqlRes=mysql_store_result(&gMysql);
        	if((mysqlField=mysql_fetch_row(mysqlRes)))
		{
			printf("%s 0\n",mysqlField[0]);
		}
	}
	mysql_free_result(mysqlRes);

}//void DayUsageData(unsigned uLogType)


void CheckAllZones(void)
{
	//
	//Run named-checkzones for all local zones and prints a summary report
	//Local zones are zones with uSecondaryOnly=0
	
	MYSQL_RES *res;
	MYSQL_ROW field;
	
	char cZoneFile[512]={""};
	
	unsigned uZonesOK=0;
	unsigned uZonesWithErrors=0;
	unsigned uTotalZones=0;
	float fSystemHealth=0.00;
	
	sprintf(gcQuery,
		"SELECT tZone.cZone,(SELECT tView.cLabel FROM tView WHERE tView.uView=tZone.uView) FROM tZone"
		" WHERE tZone.uSecondaryOnly=0 ORDER BY tZone.uZone");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	res=mysql_store_result(&gMysql);

	uTotalZones=(unsigned)mysql_num_rows(res);
	
	while((field=mysql_fetch_row(res)))
	{
		//named-checkzone e-s.co.uk /usr/local/idns/named.d/master/external/e/e-s.co.uk
		
		sprintf(cZoneFile,"/usr/local/idns/named.d/master/%s/%c/%s",field[1],field[0][0],field[0]);
		sprintf(gcQuery,"/usr/sbin/named-checkzone %s %s > /dev/null  2>&1",field[0],cZoneFile);
		
		if(system(gcQuery))
		{
			printf("Zone %s has errors. Zonefile at %s\n",field[0],cZoneFile);
			uZonesWithErrors++;
		}
		else
			uZonesOK++;
	}
	
	fSystemHealth=((float)uZonesOK*100.00)/(float)uTotalZones;
	
	printf("Summary:\n");
	printf("--------------------------------------\n");
	printf("Total zones          : %u\n",uTotalZones);
	printf("Zones with no errors : %u\n",uZonesOK);
	printf("Zones with errors    : %u\n",uZonesWithErrors);
	printf("System health is     : %.2f\n",fSystemHealth);
	
}//void CheckAllZones(void)


void ImportFromDb(char *cSourceDbName, char *cTargetDbName, char *cPasswd)
{
	printf("ImportFromDb() start\n");

	mySQLRootConnect(cPasswd);

	sprintf(gcQuery,"truncate %s.tZoneImport",cTargetDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"INSERT INTO %s.tZoneImport (uZone,cZone,uNSSet,cHostmaster,uSerial,"
			"uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
			"uMailServers,uView,cMainAddress,uRegistrar,uSecondaryOnly,cOptions,uClient,uOwner,"
			"uCreatedBy,uCreatedDate,uModBy,uModDate)"
			" SELECT uZone,cZone,1,cHostmaster,uSerial,uExpire,uRefresh,uTTL,uRetry,uZoneTTL,"
			"uMailServers,2,cMainAddress,0,0,'',0,uOwner,"
			"uCreatedBy,uCreatedDate,uModBy,uModDate FROM %s.tZone",
								cTargetDbName,cSourceDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"truncate %s.tResourceImport",cTargetDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}

	sprintf(gcQuery,"INSERT INTO %1$s.tResourceImport (uZone,cName,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,"
			"uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate)"
			" SELECT tZone.uZone,tResource.cName,tResource.uTTL,tResource.uRRType,tResource.cParam1,"
			"tResource.cParam2,'','',tResource.cComment,"
			"tResource.uOwner,tResource.uCreatedBy,tResource.uCreatedDate,tResource.uModBy,tResource.uModDate"
			" FROM %2$s.tResource,%2$s.tZone"
			" WHERE tResource.uZone=tZone.uZone",cTargetDbName,cSourceDbName);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}


	printf("ImportFromDb() end\n");

}//void ImportFromDb()


void ZeroSystem(void)
{
	register int i;

	//Any failure will need CLI initialize!
	for(i=0;cTableList[i][0];i++)
	{
        	sprintf(gcQuery,"TRUNCATE %s",cTableList[i]);
        	mysql_query(&gMysql,gcQuery);
        	if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}
	
	//Only valid for new rpm based layout
	for(i=0;cInitTableList[i][0];i++)
	{
		sprintf(gcQuery,"LOAD DATA LOCAL INFILE '/usr/local/share/iDNS/data/%s.txt' REPLACE INTO TABLE %s",
				cInitTableList[i],cInitTableList[i]);
        	mysql_query(&gMysql,gcQuery);
        	if(mysql_errno(&gMysql))
			htmlPlainTextError(mysql_error(&gMysql));
	}

	//tResourceTest not in cTableList, is not supposed to be accesed
	sprintf(gcQuery,"TRUNCATE tResourceTest");
	mysql_query(&gMysql,gcQuery);
	//Removed error handling since this is obviously wrong.

}//void ZeroSystem(void)


unsigned NewNSSet(char *cLabel,char *cMasterIPs,char *cuOwner)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	sprintf(gcQuery,"SELECT uNSSet FROM tNSSet WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		unsigned uNSSet=0;

		sscanf(field[0],"%u",&uNSSet);
		printf("\t\ttNSSet record already exists...updating\n");
		sprintf(gcQuery,"UPDATE tNSSet SET cMasterIPs='%s',uOwner=%s,"
				"uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
				" WHERE uNSSet=%u",
					cMasterIPs,cuOwner,uNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		return(uNSSet);
	}
	else
	{
		sprintf(gcQuery,"INSERT tNSSet SET cLabel='%s',cMasterIPs='%s',uOwner=%s,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
				cLabel,cMasterIPs,cuOwner);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
		return(mysql_insert_id(&gMysql));
	}
	mysql_free_result(res);

}//unsigned NewNSSet(char *cLabel,char *cMasterIPs,char *cuOwner)


void NewNS(char *cFQDN,char *cNSType,unsigned uNSSet)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSType=1;

	sprintf(gcQuery,"SELECT uNSType FROM tNSType WHERE cLabel='%s'",cNSType);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uNSType);
	else
		printf("\t\tCould not determine uNSType...using default\n");
	mysql_free_result(res);
		

	sprintf(gcQuery,"SELECT uNS FROM tNS WHERE cFQDN='%s' AND uNSSet=%u",cFQDN,uNSSet);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		printf("%s\n",mysql_error(&gMysql));
		exit(1);
	}
	res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		printf("\t\ttNS record already exists...updating\n");
		sprintf(gcQuery,"UPDATE tNS SET uNSType=%u,"
				"uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
				" WHERE uNS=%s",
					uNSType,field[0]);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	else
	{
		sprintf(gcQuery,"INSERT tNS SET cFQDN='%s',uNSType=%u,uNSSet=%u,uOwner=1,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())",
					cFQDN,uNSType,uNSSet);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			printf("%s\n",mysql_error(&gMysql));
			exit(1);
		}
	}
	mysql_free_result(res);

}//void NewNS(...)
