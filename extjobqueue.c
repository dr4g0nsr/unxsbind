/*
FILE
	extjobqueue.c
	$Id: extjobqueue.c 1469 2010-05-20 22:44:03Z Gary $
AUTHOR
	(C) 2001-2009 Gary Wallis and Hugo Urquiza for Unixservice, LLC.
	(C) 2010 Gary Wallis for Unixservice, LLC.
	GPL License applies. See LICENSE file.
PURPOSE
	GUI independent code:
	Processing mysqlISP2 or other externally created jobs.
	Externally means not created by mysqlBind2/iDNS or it's interfaces.
NOTES
	Some functions herein are mainfunc.h CLI commands
TODO
	Most of the old code has to be totally changed it is really an awful mess.
	uNSSset change from tNameServer
	See TODO
*/

#include "mysqlrad.h"
#include "extjobqueue.h"

//
//TOC protos
///
void ParseExtParams(structExtJobParameters *structExtParam, char *cJobData);
unsigned GetuServer(char *cLabel, char *cTable);
unsigned GetuZone(char *cLabel, char *cTable);
void InitializeParams(structExtJobParameters *structExtParam);
int InformExtISPJob(const char *cRemoteMsg,const char *cServer,unsigned uJob,unsigned uJobStatus);
int SubmitISPJob(const char *cJobName,const char *cJobData,const char *cServer,unsigned uJobDate);
unsigned uGetClientOwner(unsigned uClient);
void ProcessExtJobQueue(char *cServer);
void ProcessVZJobQueue(void);
unsigned WebMod(structExtJobParameters *structExtParam,
				unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner);
unsigned ModZone(structExtJobParameters *structExtParam,
				unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner);
unsigned CancelZone(structExtJobParameters *structExtParam,
				unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner);
int GetApacheIPNumber(MYSQL *mysql,char *cDomain,char *cMainAddress);
unsigned WebNew(structExtJobParameters *structExtParam,unsigned uJob, char *cServer,unsigned uClient,unsigned uOwner);
unsigned NewSimpleZone(structExtJobParameters *structExtParam,unsigned uJob, char *cServer,unsigned uClient,unsigned uOwner);
void CreateWebZone(char *cDomain, char *cIP, char *cNameServer, char *cMailServer,unsigned uClient,unsigned uOwner);
void DropZone(char *cDomain, char *cNameServer);
unsigned NewSimpleWebZone(structExtJobParameters *structExtParam,unsigned uJob,
					char *cServer,unsigned uClient,unsigned uOwner);
int SubmitExtJob(const char *cCommand, unsigned uNSSetArg, const char *cZoneArg,
			unsigned uPriorityArg, unsigned uTimeArg, unsigned uExtJob,unsigned uOwner);
int SubmitSingleExtJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, unsigned uTimeArg
	       			,unsigned *uMasterJob,unsigned uExtJob,unsigned uOwner);
void CreateNewClient(structExtJobParameters *structExtParam);

//
//Ext protos
//
unsigned TextConnectDb(void);//mysqlconnect.c
unsigned TextConnectExtDb(MYSQL *Mysql, unsigned uMode);
void SerialNum(char *cSerialNum);//bind.c
int PopulateArpaZone(const char *cZone, const char *cIPNum, const unsigned uHtmlMode, 
					const unsigned uFromZone, const unsigned uZoneOwner);
void UpdateSerialNum(unsigned uZone);//tzonefunc.h


void ParseExtParams(structExtJobParameters *structExtParam, char *cJobData)
{
	char *cp,*cp2;

	//New universal data elements
	if((cp=strstr(cJobData,"cName=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+6,';'))) *cp2=0;
			sprintf(structExtParam->cName,"%.99s",cp+6);
		if(cp2) *cp2=';';
		structExtParam->ucName=1;
	}
	if((cp=strstr(cJobData,"cuTTL=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+6,';'))) *cp2=0;
			sprintf(structExtParam->cuTTL,"%.15s",cp+6);
		if(cp2) *cp2=';';
		structExtParam->ucuTTL=1;
	}
	if((cp=strstr(cJobData,"cIPv4=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+6,';'))) *cp2=0;
			sprintf(structExtParam->cIPv4,"%.31s",cp+6);
		if(cp2) *cp2=';';
		structExtParam->ucIPv4=1;
	}
	if((cp=strstr(cJobData,"cZone=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+6,';'))) *cp2=0;
			sprintf(structExtParam->cZone,"%.99s",cp+6);
		if(cp2) *cp2=';';
		structExtParam->ucZone=1;
	}
	if((cp=strstr(cJobData,"cRRType=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+8,';'))) *cp2=0;
			sprintf(structExtParam->cRRType,"%.31s",cp+8);
		if(cp2) *cp2=';';
		structExtParam->ucRRType=1;
	}
	if((cp=strstr(cJobData,"cNSSet=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+7,';'))) *cp2=0;
			sprintf(structExtParam->cNSSet,"%.31s",cp+7);
		if(cp2) *cp2=';';
		structExtParam->ucNSSet=1;
	}
	if((cp=strstr(cJobData,"cParam1=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+8,';'))) *cp2=0;
			sprintf(structExtParam->cParam1,"%.254s",cp+8);
		if(cp2) *cp2=';';
		structExtParam->ucParam1=1;
	}
	if((cp=strstr(cJobData,"cView=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+6,';'))) *cp2=0;
			sprintf(structExtParam->cView,"%.31s",cp+6);
		if(cp2) *cp2=';';
		structExtParam->ucView=1;
	}



	//Keep these old cJobData elements for backwards compatability for a couple more
	//years while we deprecate via unxsISP
	if((cp=strstr(cJobData,"Domain=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+7,'\n'))) *cp2=0;
			sprintf(structExtParam->cZone,"%.99s",cp+7);
		if(cp2) *cp2='\n';
		structExtParam->uParamZone=1;
	}
	if((cp=strstr(cJobData,"Zone=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+5,'\n'))) *cp2=0;
			sprintf(structExtParam->cZone,"%.99s",cp+5);
		if(cp2) *cp2='\n';
		structExtParam->uParamZone=1;
	}

	if((cp=strstr(cJobData,"IP=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+3,'\n'))) *cp2=0;
			sprintf(structExtParam->cMainAddress,"%.15s",cp+3);
		if(cp2) *cp2='\n';
		structExtParam->uParamMainAddress=1;
	}
	if((cp=strstr(cJobData,"MainAddress=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+12,'\n'))) *cp2=0;
			sprintf(structExtParam->cMainAddress,"%.15s",cp+12);
		if(cp2) *cp2='\n';
		structExtParam->uParamMainAddress=1;
	}

	if((cp=strstr(cJobData,"Target=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+7,'\n'))) *cp2=0;
			sprintf(structExtParam->cTarget,"%.99s",cp+7);
		if(cp2) *cp2='\n';
		structExtParam->uParamTarget=1;
	}
	if((cp=strstr(cJobData,"ParkedDomains=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+14,'\n'))) *cp2=0;
			sprintf(structExtParam->cParkedDomains,"%.255s",cp+14);
		if(cp2) *cp2='\n';
		if(structExtParam->cParkedDomains[0])
			structExtParam->uParamParkedDomains=1;
	}
	if((cp=strstr(cJobData,"NameServer=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+11,'\n'))) *cp2=0;
			sprintf(structExtParam->cNameServer,"%.99s",cp+11);
		if(cp2) *cp2='\n';
		structExtParam->uParamNSSet=1;
		//Determine structExtParam->uNSSet
	}
	if((cp=strstr(cJobData,"RevDns=")))
	{
		if(*(cp+7)=='y' || *(cp+7)=='Y')
			structExtParam->uRevDns=1;
		structExtParam->uParamRevDns=1;
	}

	if((cp=strstr(cJobData,"MX1=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+4,'\n'))) *cp2=0;
			sprintf(structExtParam->cMX1,"%.99s",cp+4);
		if(cp2) *cp2='\n';
		structExtParam->uParamMX1=1;
	}

	if((cp=strstr(cJobData,"MX2=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+4,'\n'))) *cp2=0;
			sprintf(structExtParam->cMX2,"%.99s",cp+4);
		if(cp2) *cp2='\n';
		structExtParam->uParamMX1=1;
	}

	//tClient
	if((cp=strstr(cJobData,"ISPClient=")))
	{
		sscanf(cp+10,"%u",&structExtParam->uISPClient);
	}
	if((cp=strstr(cJobData,"ClientName=")))
	{
		cp2=NULL;
		if((cp2=strchr(cp+11,'\n'))) *cp2=0;
		sprintf(structExtParam->cClientName,"%.32s",cp+11);
		if(cp2) *cp2='\n';
		structExtParam->uParamClientName=1;
	}
	
}//void ParseExtParams()


unsigned GetuServer(char *cLabel, char *cTable)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cQuery[512];
	unsigned uServer=0;
	
	sprintf(cQuery,"SELECT _rowid FROM %s WHERE cLabel='%s'",cTable,cLabel);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));
		return(0);
	}
	res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uServer);
	mysql_free_result(res);
	return(uServer);

}//unsigned GetuServer(char *cLabel, char *cTable)


unsigned GetuZone(char *cLabel, char *cTable)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cQuery[512];
	unsigned uServer=0;
	
	sprintf(cQuery,"SELECT _rowid FROM %s WHERE cZone='%s'",cTable,cLabel);
	mysql_query(&gMysql,cQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));
		return(0);
	}
	res=mysql_store_result(&gMysql);
        if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uServer);
	mysql_free_result(res);
	return(uServer);

}//unsigned GetuZone(char *cLabel, char *cTable)


void InitializeParams(structExtJobParameters *structExtParam)
{
	MYSQL_RES *res;
	MYSQL_ROW field;

	static unsigned uOnlyOnce=1;
	static unsigned uNSSet=0;
	static char cHostmaster[256];
	static char cNameServer[256];

	if(uOnlyOnce)
	{
		//Load presets
		GetConfiguration("cHostmaster",cHostmaster,0);

		GetConfiguration("cNameServer",cNameServer,0);
		if(cNameServer[0])
			uNSSet=GetuServer(cNameServer,"tNameServer");


		//We always need one, get the first one by uNSSet
		if(!uNSSet)
		{
			//TODO
			sprintf(gcQuery,"SELECT uNSSet FROM tNSSet LIMIT 1");
 
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				exit(1);
			}
			res=mysql_store_result(&gMysql);
		        if((field=mysql_fetch_row(res)))
				sscanf(field[0],"%u",&uNSSet);
			mysql_free_result(res);
		}

		if(!uNSSet)
		{
			fprintf(stderr,"Undefined name server group. Please configure.\n");
			exit(1);
		}

		uOnlyOnce=0;
	}

	structExtParam->cZone[0]=0;
	structExtParam->cMainAddress[0]=0;
	strcpy(structExtParam->cHostmaster,cHostmaster);
	structExtParam->cTarget[0]=0;
	structExtParam->cParkedDomains[0]=0;
	structExtParam->cNameServer[0]=0;
	structExtParam->cMailServer[0]=0;
	structExtParam->cMX1[0]=0;
	structExtParam->cMX2[0]=0;
	structExtParam->uRevDns=0;

	structExtParam->uExpire=604800;
	structExtParam->uRefresh=28800;
	structExtParam->uTTL=86400;
	structExtParam->uRetry=7200;
	structExtParam->uZoneTTL=86400;

	structExtParam->uNSSet=uNSSet;
	structExtParam->uMailServer=0;

	structExtParam->uISPClient=0;
	structExtParam->cClientName[0]=0;

	//Presence data
	structExtParam->uParamZone=0;
	structExtParam->uParamMainAddress=0;
	structExtParam->uParamTarget=0;
	structExtParam->uParamParkedDomains=0;
	structExtParam->uParamNSSet=0;
	structExtParam->uParamMailServer=0;
	structExtParam->uParamRevDns=0;
	structExtParam->uParamMX1=0;
	structExtParam->uParamMX2=0;

}//void InitializeParams(structExtJobParameters *structExtParam)


int InformExtISPJob(const char *cRemoteMsg,const char *cServer,unsigned uJob,unsigned uJobStatus)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uJobGroup=0;
	unsigned uInstance=0;

	sprintf(gcQuery,"UPDATE tJob SET cServer='%s',cRemoteMsg='%.32s',uModBy=1,"
			"uModDate=UNIX_TIMESTAMP(NOW()),uJobStatus=%u WHERE uJob=%u",
				cServer,cRemoteMsg,uJobStatus,uJob);
	printf("%s\n",gcQuery);
	mysql_query(&gMysql2,gcQuery);
	if(mysql_errno(&gMysql2))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));
		SubmitISPJob("iDNS.InformExtISPJob.Failed",
				mysql_error(&gMysql2),cServer,0);
		return(1);
	}

	//Do not continue unless closing ext job	
	if(uJobStatus!=mysqlISP_Deployed)
		return(0);
	
	//See if all services of client product instance (now grouped via unique 
	//uJobGroup by mysqlISP SubmitJob routines) are now deployed. If they are 
	//then change mysqlISP.tInstance.uStatus to mysqlISP_Deployed, 
	//				mysqlISP_Canceled or mysqlISP_OnHold
	//
	sprintf(gcQuery,"SELECT uJobGroup,uInstance FROM tJob WHERE uJob=%u",uJob);
	mysql_query(&gMysql2,gcQuery);
	if(mysql_errno(&gMysql2))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));
		return(1);
	}
	res=mysql_store_result(&gMysql2);
        if((field=mysql_fetch_row(res)))
	{
		sscanf(field[0],"%u",&uJobGroup);
		sscanf(field[1],"%u",&uInstance);
	}
	mysql_free_result(res);

	if(!uJobGroup || !uInstance)
	{
		fprintf(stderr,"Unexpected missing uJobGroup/uInstance for uJob=%u\n"
						,uJob);
		return(0);
	}

	sprintf(gcQuery,"SELECT uJobStatus=%u,(MAX(uJobStatus)=MIN(uJobStatus)),cJobName FROM tJob WHERE uJobGroup=%u"
			" GROUP BY uJobGroup",mysqlISP_Deployed,uJobGroup);

	mysql_query(&gMysql2,gcQuery);
	if(mysql_errno(&gMysql2))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));
	}
	else
	{
		res=mysql_store_result(&gMysql2);
        	if((field=mysql_fetch_row(res)))
		{
			unsigned uInstanceStatus=mysqlISP_Deployed;

			if(field[0][0]=='1' && field[1][0]=='1')
			{
				if(strstr(field[2],".Cancel"))
					uInstanceStatus=mysqlISP_Canceled;
				else if(strstr(field[2],".Hold"))
					uInstanceStatus=mysqlISP_OnHold;

				sprintf(gcQuery,"UPDATE tInstance SET uStatus=%u,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
						" WHERE uInstance=%u",uInstanceStatus,uInstance);
				printf("%s\n",gcQuery);
				mysql_query(&gMysql2,gcQuery);
				if(mysql_errno(&gMysql2))
					fprintf(stderr,"%s\n",mysql_error(&gMysql2));
			}
		}
		mysql_free_result(res);
	}

	return(0);

}//int InformExtISPJob()


int SubmitISPJob(const char *cJobName,const char *cJobData,const char *cServer,unsigned uJobDate)
{
	sprintf(gcQuery,"INSERT INTO tJob SET cServer='%s',cJobName='%s',cJobData='%.1024s',"
			"uJobDate=UNIX_TIMESTAMP(NOW()),uOwner=1,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW()),uJobStatus=%u,cLabel='iDNS.SubmitISPJob'",
					cServer,cJobName,cJobData,mysqlISP_Waiting);
	mysql_query(&gMysql2,gcQuery);
	if(mysql_errno(&gMysql2))
	{
                fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	return(0);

}//int SubmitISPJob()


unsigned uGetClientOwner(unsigned uClient)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uRet=0;
	
	sprintf(gcQuery,"SELECT uOwner FROM "TCLIENT" WHERE uClient=%u",uClient);
	printf("uGetClientOwner:%s\n",gcQuery);

	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s",mysql_error(&gMysql));
		exit(0);
	}
	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uRet);
	mysql_free_result(res);
	printf("uGetClientOwner:uRet=%u\n",uRet);
	return(uRet);

}//unsigned uGetClientOwner(unsigned uClient)


void ProcessExtJobQueue(char *cServer)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	MYSQL_RES *res2;
	MYSQL_ROW field2;

	structExtJobParameters structExtParam;

	unsigned uJob=0;
	unsigned uJobClient=0;
	unsigned uZone=0;
	unsigned uOwner=0;

	char cQuery[10240];
		
	//Must be first
	if(TextConnectDb())
		return;
	if(TextConnectExtDb(&gMysql2,TEXT_CONNECT_ISP))
	{
		mysql_close(&gMysql);
		return;
	}

	sprintf(cQuery,"SELECT cJobName,cJobData,uJob,uJobClient FROM tJob WHERE (cServer='Any' OR cServer='%s')"
			" AND uJobStatus=%u AND uJobDate<=UNIX_TIMESTAMP(NOW())"
			" AND cJobName LIKE 'iDNS.%%'",cServer,mysqlISP_Waiting);

	//Debug only	
	//printf("%s\n",cQuery);


	mysql_query(&gMysql2,cQuery);
        if(mysql_errno(&gMysql2))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));
		return;
	}

	res=mysql_store_result(&gMysql2);
        while((field=mysql_fetch_row(res)))
	{
		InitializeParams(&structExtParam);

		sscanf(field[2],"%u",&uJob);
		sscanf(field[3],"%u",&uJobClient);
		if(!(uOwner=uGetClientOwner(uJobClient)))
		{
			printf("Warning: uGetClientOwner(%u) didn't return a value.\n",uJobClient);
			printf("Warning: setting default uOwner=1\n");
			uOwner=1; //Safe default. 
		}

		//Automated website DNS
		if(!strcmp("iDNS.Web.New",field[0]))
		{	
			printf("\n%s(%s):\n",field[0],field[2]);
			ParseExtParams(&structExtParam,field[1]);
			sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",structExtParam.cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
        		{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				InformExtISPJob("Select query Web.New failed",
						cServer,uJob,mysqlISP_Waiting);
			}
			else
			{
        			res2=mysql_store_result(&gMysql);
        			if((field2=mysql_fetch_row(res2)))
				{
					//Modify
					sscanf(field2[0],"%u",&uZone);

					//Already exists change IP
					//and other parameters
					if(!WebMod(&structExtParam,
							uZone,uJob,cServer,uOwner))
					InformExtISPJob("WebMod() Ok",
						cServer,uJob,mysqlISP_RemotelyQueued);
					else
					InformExtISPJob("WebMod() Error",
						cServer,uJob,mysqlISP_Waiting);
				}
				else
				{
					unsigned uRetVal;
					//New
					uRetVal=WebNew(&structExtParam,uJob,cServer,uJobClient,uOwner);
					switch(uRetVal)
					{
						case 0:
						InformExtISPJob("WebNew() Ok",
						cServer,uJob,mysqlISP_RemotelyQueued);
						//CreateNewClient(&structExtParam);
						break;
						
						case 1:
						InformExtISPJob("WebNew() Error",
						cServer,uJob,mysqlISP_Waiting);
						break;
						
						case 2:
						InformExtISPJob("WebNew() Waiting for IP",
						cServer,uJob,mysqlISP_Waiting);
						break;
					}
				}
				mysql_free_result(res2);
			}
		}//Web.New


		else if(	!strcmp("iDNS.Web.Mod",field[0]) ||
				!strcmp("iDNS.WebZone.Mod",field[0]) ||
				!strcmp("iDNS.Zone.Mod",field[0]) )
		{	
			printf("\n%s(%s):\n",field[0],field[2]);
			ParseExtParams(&structExtParam,field[1]);
			sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",structExtParam.cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
        		{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				InformExtISPJob("Select query Mod failed",
						cServer,uJob,mysqlISP_Waiting);
			}
			else
			{
        			res2=mysql_store_result(&gMysql);
        			if((field2=mysql_fetch_row(res2)))
				{
					//Modify
					sscanf(field2[0],"%u",&uZone);

					if(!ModZone(&structExtParam,
							uZone,uJob,cServer,uOwner))
					InformExtISPJob("ModZone() Ok",
						cServer,uJob,mysqlISP_RemotelyQueued);
					else
					InformExtISPJob("ModZone() Error",
						cServer,uJob,mysqlISP_Waiting);
				}
				else
				{
					InformExtISPJob("Zone does not exist",
						cServer,uJob,mysqlISP_Waiting);
				}
				mysql_free_result(res2);
			}

		}//Web/Zone/WebZone.Mod


		else if(	!strcmp("iDNS.Web.Cancel",field[0]) ||
				!strcmp("iDNS.WebZone.Cancel",field[0]) ||
				!strcmp("iDNS.Zone.Cancel",field[0]) )
		{	
			printf("\n%s(%s):\n",field[0],field[2]);
			ParseExtParams(&structExtParam,field[1]);
			sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",structExtParam.cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
        		{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				InformExtISPJob("Select query Cancel failed",
						cServer,uJob,mysqlISP_Waiting);
			}
			else
			{
        			res2=mysql_store_result(&gMysql);
        			if((field2=mysql_fetch_row(res2)))
				{
					//Cancel
					sscanf(field2[0],"%u",&uZone);

					if(!CancelZone(&structExtParam,
							uZone,uJob,cServer,uOwner))
					InformExtISPJob("CancelZone() Ok",
						cServer,uJob,mysqlISP_RemotelyQueued);
					else
					InformExtISPJob("CancelZone() Error",
						cServer,uJob,mysqlISP_Waiting);
				}
				else
				{
					InformExtISPJob("No such zone error",
							cServer,uJob,mysqlISP_Waiting);
				}
				mysql_free_result(res2);
			}

		}//Web/Zone/WebZone.Cancel


		//OnHold: Do nothing
		else if(	!strcmp("iDNS.Web.OnHold",field[0]) ||
				!strcmp("iDNS.WebZone.OnHold",field[0]) ||
				!strcmp("iDNS.Zone.OnHold",field[0]) )
		{	
			printf("\n%s(%s):\n",field[0],field[2]);
			ParseExtParams(&structExtParam,field[1]);
			sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",structExtParam.cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
        		{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				InformExtISPJob("Select query OnHold failed",
						cServer,uJob,mysqlISP_Waiting);
			}
			else
			{
        			res2=mysql_store_result(&gMysql);
        			if(mysql_num_rows(res2))
				{
					//OnHold: Do nothing
					InformExtISPJob("OnHold NOP",
						cServer,uJob,mysqlISP_Deployed);
				}
				else
				{
					InformExtISPJob("No such cZone",
							cServer,uJob,mysqlISP_Waiting);
				}
				mysql_free_result(res2);
			}

		}//Web/Zone/WebZone.OnHold


		else if(!strcmp("iDNS.Zone.New",field[0]))
		{	
			printf("\n%s(%s):\n",field[0],field[2]);
			ParseExtParams(&structExtParam,field[1]);
			sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",structExtParam.cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
        		{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				InformExtISPJob("Select Zone.New failed",
						cServer,uJob,mysqlISP_Waiting);
			}
			else
			{
        			res2=mysql_store_result(&gMysql);
				if(mysql_num_rows(res2))
				{
					InformExtISPJob("Zone already exists!",
						cServer,uJob,mysqlISP_Waiting);
				}
				else
				{
					unsigned uRetVal;
					uRetVal=NewSimpleZone(&structExtParam,uJob,cServer,uJobClient,uOwner);
					switch(uRetVal)
					{
						case 0:
						InformExtISPJob("NewSimpleZone() Ok",
						cServer,uJob,mysqlISP_RemotelyQueued);
						//CreateNewClient(&structExtParam);
						break;
						
						default:
						InformExtISPJob("NewSimpleZone() Error",
						cServer,uJob,mysqlISP_Waiting);
						break;
						
					}
				}
				mysql_free_result(res2);
			}
		}//Zone.New


		else if(!strcmp("iDNS.WebZone.New",field[0]))
		{	
			printf("\n%s(%s):\n",field[0],field[2]);
			ParseExtParams(&structExtParam,field[1]);
			sprintf(cQuery,"SELECT uZone FROM tZone WHERE cZone='%s'",structExtParam.cZone);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
        		{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				InformExtISPJob("Select WebZone.New failed",
						cServer,uJob,mysqlISP_Waiting);
			}
			else
			{
        			res2=mysql_store_result(&gMysql);
				if(mysql_num_rows(res2))
				{
					InformExtISPJob("Zone already exists!",
						cServer,uJob,mysqlISP_Waiting);
				}
				else
				{
					unsigned uRetVal;
					uRetVal=NewSimpleWebZone(&structExtParam,uJob,cServer,uJobClient,uOwner);
					switch(uRetVal)
					{
						case 0:
						InformExtISPJob("NewSimpleWebZone() Ok",
						cServer,uJob,mysqlISP_RemotelyQueued);
						//CreateNewClient(&structExtParam);
						break;
						
						default:
						InformExtISPJob("NewSimpleWebZone() Error",
						cServer,uJob,mysqlISP_Waiting);
						break;
						
					}
				}
				mysql_free_result(res2);
			}
		}//WebZone.New


		else if(1)
		{
			fprintf(stderr,"Unknown job:%s\n",field[0]);
		}
	}
	mysql_free_result(res);

}//void ProcessExtJobQueue(char *cServer)


unsigned WebMod(structExtJobParameters *structExtParam,
				unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner)
{
	//Very limited changes allowed so far...
	sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1,uMailServers=%u,cMainAddress='%s',uModBy=1,"
			"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uZone=%u"
			,structExtParam->uMailServer
			,structExtParam->cMainAddress
			,uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	if(SubmitExtJob("ExtModify",structExtParam->uNSSet,structExtParam->cZone,0,0,uExtJob,uOwner))
		return(1);

	return(0);

}//unsigned WebMod()


unsigned ModZone(structExtJobParameters *structExtParam,
				unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	
	//Complicated NS operations as additional parameters
	if(!structExtParam->uMailServer)
	{
		//uRRType=3 = MX record TODO
		if(structExtParam->cMX1[0])
		{
			sprintf(gcQuery,"SELECT uResource FROM tResource WHERE uRRType=3 AND uZone=%u AND cParam1='1'",uZone);
        		mysql_query(&gMysql,gcQuery);
        		if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		        res=mysql_store_result(&gMysql);
			if(!mysql_num_rows(res))
			{
				sprintf(gcQuery,"INSERT INTO tResource SET uRRType=3,uZone=%u,cParam2='%s.',cParam1='1',"
						"cName='%s.',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
						",cComment='ModZone()'",
						uZone
						,structExtParam->cMX1
						,structExtParam->cZone
						,uOwner);
        			mysql_query(&gMysql,gcQuery);
	        		if(mysql_errno(&gMysql))
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					return(1);
				}
			}
			else
			{
        			if((field=mysql_fetch_row(res)))
				{
					sprintf(gcQuery,"UPDATE tResource SET cParam2='%s.',uModBy=1,"
							"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uResource=%s",
							structExtParam->cMX1,
							field[0]);

	        			mysql_query(&gMysql,gcQuery);
		        		if(mysql_errno(&gMysql))
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						return(1);
					}
				}
			}
        		mysql_free_result(res);
		}//End of MX1 case

		if(structExtParam->cMX2[0])
		{
			sprintf(gcQuery,"SELECT uResource FROM tResource WHERE uRRType=3 AND uZone=%u AND cParam1='2'",uZone);
        		mysql_query(&gMysql,gcQuery);
        		if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		        res=mysql_store_result(&gMysql);
			if(!mysql_num_rows(res))
			{
				sprintf(gcQuery,"INSERT INTO tResource SET uRRType=3,uZone=%u,cParam2='%s.',cParam1='2',"
						"cName='%s.',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),"
						"cComment='ModZone()'",
						uZone,
						structExtParam->cMX2,
						structExtParam->cZone,
						uOwner);
        			mysql_query(&gMysql,gcQuery);
	        		if(mysql_errno(&gMysql))
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					return(1);
				}
			}
			else
			{
        			if((field=mysql_fetch_row(res)))
				{
					sprintf(gcQuery,"UPDATE tResource SET cParam2='%s.',uModBy=1,"
							"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uResource=%s",
							structExtParam->cMX2,
							field[0]);

	        			mysql_query(&gMysql,gcQuery);
		        		if(mysql_errno(&gMysql))
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						return(1);
					}
				}
			}
        		mysql_free_result(res);
		}//End of MX2 case

	}//End of MX business

	sprintf(gcQuery,"UPDATE tZone SET uSerial=uSerial+1,uMailServers=%u,uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
			" WHERE uZone=%u"
			,structExtParam->uMailServer
			//,structExtParam->cMainAddress
			,uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	
	//Update cMainAddress RR if any, note that we rely on the cComment field for this
	//Should a new RR be added if the cMainAddress RR doesn't exist? Perhaps, but it might
	//conflict with other record added after zone creation, we will just print a warning at the logfile.

	sprintf(gcQuery,"SELECT uResource FROM tResource WHERE cName='@' AND cComment='Zone cMainAddress'"
				" AND uRRType=1 AND uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		 fprintf(stderr,"%s\n",mysql_error(&gMysql));
		 return(1);
	}
	
	res=mysql_store_result(&gMysql);

	if(!mysql_num_rows(res))
		fprintf(stderr,"Warning: It seems there's no cMainAddress RR for uZone=%u\n",uZone);
	else
	{
		if((field=mysql_fetch_row(res)))
		{
			sprintf(gcQuery,"UPDATE tResource SET cParam1='%s',uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
					" WHERE uResource=%s",
					structExtParam->cMainAddress
					,field[0]);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s (%s)\n",mysql_error(&gMysql),gcQuery);
				return(1);
			}
		}
		mysql_free_result(res);
	}

	if(SubmitExtJob("ExtModify",structExtParam->uNSSet,structExtParam->cZone,0,0,uExtJob,uOwner))
		return(1);

	return(0);

}//unsigned ModZone()


unsigned CancelZone(structExtJobParameters *structExtParam,
				unsigned uZone,unsigned uExtJob, char *cServer,unsigned uOwner)
{
	sprintf(gcQuery,"DELETE FROM tZone WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
        {
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}

	if(mysql_affected_rows(&gMysql)>0)
	{
		sprintf(gcQuery,
		"DELETE FROM tResource WHERE uZone=%u",uZone);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql)) 
			fprintf(stderr,"%s\n",mysql_error(&gMysql));

		if(SubmitExtJob("ExtDelete",structExtParam->uNSSet,structExtParam->cZone,0,
					0,uExtJob,uOwner))
			return(1);

		return(0);
	}

	return(1);

}//unsigned CancelZone()


int GetApacheIPNumber(MYSQL *mysql,char *cDomain,char *cMainAddress)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uRetVal=1;

        char cQuery[512];

        sprintf(cQuery,"SELECT cIP FROM tVirtualHost WHERE cDomain='%s'",cDomain);
        mysql_query(mysql,cQuery);
        if(mysql_errno(mysql))
	{
		fprintf(stderr,"%s\n",mysql_error(mysql));
		return(1);
	}
        res=mysql_store_result(mysql);
        if((field=mysql_fetch_row(res)))
	{
                strcpy(cMainAddress,field[0]);
		uRetVal=0;
	}
        mysql_free_result(res);
	
	return(uRetVal);

}//int GetApacheIPNumber(MYSQL *mysql,char *cDomain,char *cMainAddress)


unsigned WebNew(structExtJobParameters *structExtParam,unsigned uJob, char *cServer,unsigned uClient,unsigned uOwner)
{
	char cSerial[32]={""};
	unsigned uSerial=0;
	unsigned uZone=0;
	MYSQL mysqlApache;
	char cuView[256]="2";//Standard distribution external view

	if(!strcmp(structExtParam->cZone+strlen(structExtParam->cZone)-5,".arpa"))
	{
		fprintf(stderr,"Can't add .arpa zones via mysqlISP\n");
		return(1);
	}

	GetConfiguration("cuView",cuView,0);
	SerialNum(cSerial);
	sscanf(cSerial,"%u",&uSerial);

	if(TextConnectExtDb(&mysqlApache,TEXT_CONNECT_APACHE))
		return(1);
	if(GetApacheIPNumber(&mysqlApache,structExtParam->cZone,
					structExtParam->cMainAddress))
			return(2);
	
	if(!structExtParam->cHostmaster[0])
		//Note non @ format
		sprintf(structExtParam->cHostmaster,"dns.%.94s",structExtParam->cZone);

		sprintf(gcQuery,"INSERT INTO tZone SET  cZone='%s', uNSSet=%u, cHostmaster='%s', "
				"uSerial=%u, uExpire=%u, uRefresh=%u, uTTL=%u, uRetry=%u, uZoneTTL=%u, "
				"uMailServers=%u, cMainAddress='%s', uClient=%u,uOwner=%u, uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW()), uView=%.2s"
			,structExtParam->cZone
			,structExtParam->uNSSet
			,structExtParam->cHostmaster
			,uSerial
			,structExtParam->uExpire
			,structExtParam->uRefresh
			,structExtParam->uTTL
			,structExtParam->uRetry
			,structExtParam->uZoneTTL
			,structExtParam->uMailServer
			,structExtParam->cMainAddress
			,uClient
			,uOwner
			,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}

	uZone=mysql_insert_id(&gMysql);

	if(structExtParam->cMainAddress[0] && structExtParam->uRevDns
			&& strcmp(structExtParam->cMainAddress,"0.0.0.0"))
	{
		if(PopulateArpaZone(structExtParam->cZone,
					structExtParam->cMainAddress,0,uZone,1))
			return(1);
	}

	//Add CNAME uRRType=5 TODO
	sprintf(gcQuery,"INSERT INTO tResource SET uRRType=5,uZone=%u,cParam1='%s.',cName='www',uOwner=%u,"
				"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='WebNew()'",
					uZone,
					structExtParam->cZone,
					uOwner);
       	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}

	if(SubmitExtJob("ExtNew",structExtParam->uNSSet,
					structExtParam->cZone,0,0,uJob,uOwner))
		return(1);

	return(0);

}//unsigned WebNew()


unsigned NewSimpleZone(structExtJobParameters *structExtParam,unsigned uJob, char *cServer,unsigned uClient,unsigned uOwner)
{
	char cSerial[32]={""};
	char cuView[256]="2";

	unsigned uSerial=0;
	unsigned uZone=0;
	
	if(!strcmp(structExtParam->cZone+strlen(structExtParam->cZone)-5,".arpa"))
	{
		fprintf(stderr,"Can't add .arpa zones via mysqlISP\n");
		return(1);
	}
	
	if(!structExtParam->cMainAddress[0])
	{
		fprintf(stderr,"Can't setup zone without specifying cMainAddress\n");
		return(1);
	}

	//cClientName
	if(!structExtParam->cClientName[0])
	{
		fprintf(stderr,"Can't setup zone without specifying cClientName\n");
		return(1);
	}

	GetConfiguration("cuView",cuView,0);
	SerialNum(cSerial);
	sscanf(cSerial,"%u",&uSerial);

	if(!structExtParam->cHostmaster[0])
		//Note non @ format
		sprintf(structExtParam->cHostmaster,"dns.%.94s",structExtParam->cZone);

	sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=%u,cHostmaster='%s',uSerial=%u,uExpire=%u,"
			"uRefresh=%u,uTTL=%u,uRetry=%u,"
			"uZoneTTL=%u,uMailServers=%u,uClient=%u,uOwner=%u,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW()),uView=%.2s"
			,structExtParam->cZone
			,structExtParam->uNSSet
			,structExtParam->cHostmaster
			,uSerial
			,structExtParam->uExpire
			,structExtParam->uRefresh
			,structExtParam->uTTL
			,structExtParam->uRetry
			,structExtParam->uZoneTTL
			,structExtParam->uMailServer
			,uClient
			,uOwner
			,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	uZone=mysql_insert_id(&gMysql);
	
	//Insert the A record at tResource for cMainAddress
	sprintf(gcQuery,"INSERT INTO tResource SET uRRType=1,uZone=%u,cName='@',cParam1='%s',uOwner=%u,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='Zone cMainAddress'",
			uZone
			,structExtParam->cMainAddress
			,uOwner);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}

	if(structExtParam->cMainAddress[0] && structExtParam->uRevDns && strcmp(structExtParam->cMainAddress,"0.0.0.0"))
	{
		if(PopulateArpaZone(structExtParam->cZone,structExtParam->cMainAddress,0,uZone,1))
		{
			fprintf(stderr,"PopulateArpaZone() call failed\n");
			return(1);
		}
	}

	if(!structExtParam->uMailServer)
	{
		//uRRType=3 = MX record TODO
		if(structExtParam->cMX1[0])
		{
			sprintf(gcQuery,"INSERT INTO tResource SET uRRType=3,uZone=%u,cParam2='%s.',cParam1='1',"
					"cName='%s.',uOwner=%u,uCreatedBy=1,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='NewSimpleZone()'",
					uZone
					,structExtParam->cMX1
					,structExtParam->cZone
					,uOwner);
	        	mysql_query(&gMysql,gcQuery);
		        if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		}//End of MX1 case

		if(structExtParam->cMX2[0])
		{
			sprintf(gcQuery,"INSERT INTO tResource SET uRRType=3,uZone=%u,cParam2='%s.',cParam1='2',"
					"cName='%s.',uOwner=%u,uCreatedBy=1,"
					"uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='NewSimpleZone()'",
					uZone
					,structExtParam->cMX2
					,structExtParam->cZone
					,uOwner);
	        	mysql_query(&gMysql,gcQuery);
		        if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		}//End of MX2 case
	}//End of MX1/2 and no uMailServer

	if(SubmitExtJob("ExtNew",structExtParam->uNSSet,structExtParam->cZone,0,0,uJob,uOwner))
	{
		fprintf(stderr,"SubmitExtJob() call failed\n");
		return(1);
	}

	return(0);

}//unsigned NewSimpleZone()


void CreateWebZone(char *cDomain, char *cIP, char *cNameServer, char *cMailServer,unsigned uClient,unsigned uOwner)
{

	//We use this for faster setup of defaults and re-use of code issues.
	structExtJobParameters structExtParam;
	char cSerial[32]={""};
	char cuView[256]="2";

	unsigned uZone=0;

	if(GetuZone(cDomain,"tZone"))
	{
		fprintf(stderr,"tZone.cZone=%s already exists\n",cDomain);
		return;
	}

	GetConfiguration("cuView",cuView,0);
	InitializeParams(&structExtParam);

	if(cNameServer[0])
	{
		//TODO
		structExtParam.uNSSet=GetuServer(cNameServer,"tNameServer");
	}
	else
	{
		structExtParam.uNSSet=1;
		printf("Using uNSSet=1 since cNameServer was not specified\n");
	}

	if(!structExtParam.uNSSet)
	{
		structExtParam.uNSSet=1;
		printf("Reverting to uNSSet=1 after GetuServer() failure\n");
	}

	if(cMailServer[0])
	{
		structExtParam.uMailServer=GetuServer(cMailServer,"tMailServer");
	}
	else
	{
		structExtParam.uMailServer=1;
		printf("Using uMailServer=1 since cMailServer was not specified\n");
	}

	if(!structExtParam.uMailServer)
	{
		structExtParam.uMailServer=1;
		printf("Reverting to uMailServer=1 after GetuServer() failure\n");
	}

	sprintf(structExtParam.cZone,"%.99s",cDomain);
	sprintf(structExtParam.cMainAddress,"%.15s",cIP);

	if(!strcmp(structExtParam.cZone+strlen(structExtParam.cZone)-5,".arpa"))
	{
		fprintf(stderr,"Can't add .arpa zones via CreateWebZone()\n");
		return;
	}

	if(!structExtParam.cMainAddress[0])
	{
		fprintf(stderr,"Can't setup zone without specifying cMainAddress\n");
		return;
	}

	//cClientName
	if(!structExtParam.cClientName[0])
	{
		fprintf(stderr,"Can't setup zone without specifying cClientName\n");
		return;
	}	
	if(!structExtParam.cHostmaster[0])
		//Note non @ format
		sprintf(structExtParam.cHostmaster,"dns.%.94s",structExtParam.cZone);
	
	SerialNum(cSerial);

	sprintf(gcQuery,"INSERT INTO tZone SET cZone='%s',uNSSet=%u,cHostmaster='%s',uSerial='%s',uExpire=%u,"
			"uRefresh=%u,uTTL=%u,uRetry=%u,"
			"uZoneTTL=%u,uMailServers=%u,uClient=%u,uOwner=%u,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW()),uView=%.2s"
			,structExtParam.cZone
			,structExtParam.uNSSet
			,structExtParam.cHostmaster
			,cSerial
			,structExtParam.uExpire
			,structExtParam.uRefresh
			,structExtParam.uTTL
			,structExtParam.uRetry
			,structExtParam.uZoneTTL
			,structExtParam.uMailServer
			,uClient
			,uOwner
			,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return;
	}
	uZone=mysql_insert_id(&gMysql);

	//Insert the A record at tResource for cMainAddress
	sprintf(gcQuery,"INSERT INTO tResource SET uRRType=1,uZone=%u,cName='@',cParam1='%s',uOwner=%u,uCreatedBy=1,"
			"uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='Zone cMainAddress'",
			uZone
			,structExtParam.cMainAddress
			,uOwner);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return;
	}

	if(structExtParam.cMainAddress[0] && structExtParam.uRevDns && strcmp(structExtParam.cMainAddress,"0.0.0.0"))
	{
		if(PopulateArpaZone(structExtParam.cZone,structExtParam.cMainAddress,0,uZone,1))
		{
			fprintf(stderr,"PopulateArpaZone() call failed\n");
			return;
		}
	}


	sprintf(gcQuery,"INSERT INTO tResource SET uRRType=5,uZone=%u,cParam1='%s.',cName='www',"
			"uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='CreateWebZone()'",
					uZone,
					structExtParam.cZone,
					uOwner);
       	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return;
	}

	if(SubmitJob("New",structExtParam.uNSSet,structExtParam.cZone,0,0))
	{
		fprintf(stderr,"SubmitJob() failed.\n");
		return;
	}

}//void CreateWebZone()



void DropZone(char *cDomain, char *cNameServer)
{
	unsigned uNSSet=0;
	unsigned uZone=0;

	if(!(uZone=GetuZone(cDomain,"tZone")))
	{
		fprintf(stderr,"tZone.cZone=%s not found\n",cDomain);
		return;
	}


	if(cNameServer[0])
	{
		uNSSet=GetuServer(cNameServer,"tNameServer");
	}
	else
	{
		uNSSet=1;
		printf("Using uNSSet=1 since cNameServer was not specified\n");
	}

	if(!uNSSet)
	{
		uNSSet=1;
		printf("Reverting to uNSSet=1 after GetuServer() failure\n");
	}

	if(SubmitJob("Delete",uNSSet,cDomain,0,0))
	{
		fprintf(stderr,"SubmitJob() failed.\n");
		return;
	}

	sprintf(gcQuery,"DELETE FROM tZone WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return;
	}
	sprintf(gcQuery,"DELETE FROM tResource WHERE uZone=%u",uZone);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return;
	}

}//void DropZone(char *cDomain, char *cNameServer)


unsigned NewSimpleWebZone(structExtJobParameters *structExtParam,unsigned uJob,
					char *cServer,unsigned uClient,unsigned uOwner)
{
	char cSerial[32]={""};
	unsigned uSerial=0;
	unsigned uZone=0;
	char cuView[256]="2";

	if(!strcmp(structExtParam->cZone+strlen(structExtParam->cZone)-5,".arpa"))
	{
		fprintf(stderr,"Can't add .arpa zones via mysqlISP\n");
		return(1);
	}

	GetConfiguration("cuView",cuView,0);
	SerialNum(cSerial);
	sscanf(cSerial,"%u",&uSerial);

	if(!structExtParam->cHostmaster[0])
		//Note non @ format
		sprintf(structExtParam->cHostmaster,"dns.%.94s",structExtParam->cZone);

	sprintf(gcQuery,"INSERT INTO tZone SET  cZone='%s',uNSSet=%u,cHostmaster='%s',uSerial=%u,"
			"uExpire=%u,uRefresh=%u,uTTL=%u,uRetry=%u,uZoneTTL=%u,uMailServers=%u,"
			"cMainAddress='%s',uClient=%u,uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),uView=%.2s"
			,structExtParam->cZone
			,structExtParam->uNSSet
			,structExtParam->cHostmaster
			,uSerial
			,structExtParam->uExpire
			,structExtParam->uRefresh
			,structExtParam->uTTL
			,structExtParam->uRetry
			,structExtParam->uZoneTTL
			,structExtParam->uMailServer
			,structExtParam->cMainAddress
			,uClient
			,uOwner
			,cuView);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	uZone=mysql_insert_id(&gMysql);


	if(structExtParam->cMainAddress[0] && structExtParam->uRevDns
			&& strcmp(structExtParam->cMainAddress,"0.0.0.0"))
	{
		if(PopulateArpaZone(structExtParam->cZone,
					structExtParam->cMainAddress,0,uZone,1))
			return(1);
	}


	if(!structExtParam->uMailServer)
	{
		//uRRType=3 = MX record TODO
		if(structExtParam->cMX1[0])
		{
			sprintf(gcQuery,"INSERT INTO tResource SET uRRType=3,uZone=%u,cParam2='%s.',cParam1='1',"
					"cName='%s.',uOwner=%u,uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
					",cComment='NewSimpleWebZone()'",
						uZone,
						structExtParam->cMX1,
						structExtParam->cZone,
						uOwner);
	        	mysql_query(&gMysql,gcQuery);
		        if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		}//End of MX1 case

		if(structExtParam->cMX2[0])
		{
			sprintf(gcQuery,"INSERT INTO tResource SET uRRType=3,uZone=%u,cParam2='%s.',"
					"cParam1='2',cName='%s.',uOwner=%u,"
					"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='NewSimpleWebZone()'",
						uZone,
						structExtParam->cMX2,
						structExtParam->cZone,
						uOwner);
	        	mysql_query(&gMysql,gcQuery);
		        if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		}//End of MX2 case
	}//End of MX1/2 and no uMailServer


	//Add CNAME uRRType=5 TODO
	sprintf(gcQuery,"INSERT INTO tResource SET uRRType=5,uZone=%u,cParam1='%s.',cName='www',uOwner=%u,"
			"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='NewSimpleWebZone()'",
					uZone, structExtParam->cZone, uOwner);
       	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	
	//Add @ record

	sprintf(gcQuery,"INSERT INTO tResource SET uRRType=1,uZone=%u,cParam1='%s.',cName='@',uOwner=%u,"
			"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='NewSimpleWebZone()'",
					uZone, structExtParam->cMainAddress, uOwner);
       	mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	
	if(SubmitExtJob("ExtNew",structExtParam->uNSSet,structExtParam->cZone,0,0,uJob,uOwner))
		return(1);

	return(0);

}//unsigned NewSimpleWebZone()



int SubmitExtJob(const char *cCommand, unsigned uNSSetArg, const char *cZoneArg,
			unsigned uPriorityArg, unsigned uTimeArg, unsigned uExtJob,unsigned uOwner)
{
	MYSQL_RES *res2;
	MYSQL_ROW field;
	char cTargetServer[256];
	static unsigned uMasterJob=0;
	unsigned uNSType=0;
	unsigned uJob=0;

	//Submit one job per EACH NS in the list, group with
	//uMasterJob
	sprintf(gcQuery,"SELECT tNS.cFQDN,tNS.uNSType,tNSType.cLabel FROM tNS,tNSSet,tNSType "
			"WHERE tNS.uNSType=tNSType.uNSType AND tNS.uNSSet=tNS.uNSSet AND tNS.uNSSet=%u",
			uNSSetArg);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	res2=mysql_store_result(&gMysql);
	
	while((field=mysql_fetch_row(res2)))
	{
		sscanf(field[0],"%u",&uNSType);
		sprintf(cTargetServer,"%s %s",field[0],field[2]);
		if(SubmitSingleExtJob(cCommand,cZoneArg,uNSSetArg,
				cTargetServer,uPriorityArg,uTimeArg,
				&uMasterJob,uExtJob,uOwner))
			return(1);
		uJob=mysql_insert_id(&gMysql);
		if(uNSType==1) uMasterJob=uJob;

		if(uMasterJob)
		{
			sprintf(gcQuery,"UPDATE tJob SET uMasterJob=%u WHERE uJob=%u",uMasterJob,uJob);
			mysql_query(&gMysql,gcQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				return(1);
			}
		}
	}
	mysql_free_result(res2);

	return(0);

}//int SubmitExtJob()


int SubmitSingleExtJob(const char *cCommand,const char *cZoneArg, unsigned uNSSetArg,
		const char *cTargetServer, unsigned uPriorityArg, unsigned uTimeArg
	       			,unsigned *uMasterJob,unsigned uExtJob,unsigned uOwner)
{
	MYSQL_RES *res;

	sprintf(gcQuery,"SELECT uJob FROM tJob WHERE cJob='%s' AND cZone='%s' AND uNSSet=%u AND cTargetServer='%s'",
			cCommand,cZoneArg,uNSSetArg,cTargetServer);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
	{
		fprintf(stderr,"%s\n",mysql_error(&gMysql));
		return(1);
	}
	res=mysql_store_result(&gMysql);
	
	if(mysql_num_rows(res)==0)
	{
		char cJobData[32];
		unsigned uJob=0;

		sprintf(cJobData,"uJob=%u",uExtJob);
		sprintf(gcQuery,"INSERT INTO tJob SET cJob='%s',cZone='%s',uNSSet=%u,cTargetServer='%s',"
				" uPriority=%u,uTime=UNIX_TIMESTAMP(NOW()),cJobData='%s',uOwner=%u,uCreatedBy=1,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW())"
					,cCommand
					,cZoneArg
					,uNSSetArg
					,cTargetServer
					,uPriorityArg
					,cJobData
					,uOwner);

		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			mysql_free_result(res);
			return(1);
		}

		if(*uMasterJob == 0)
		{
			uJob=*uMasterJob=mysql_insert_id(&gMysql);
			if(!strstr(cTargetServer,"MASTER"))
				fprintf(stderr,"MASTER must be first in cList\n");
		}
		else
		{
			uJob=mysql_insert_id(&gMysql);
		}
	
		sprintf(gcQuery,"UPDATE tJob SET uMasterJob=%u WHERE uJob=%u",*uMasterJob,uJob);
		mysql_query(&gMysql,gcQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			mysql_free_result(res);
			return(1);
		}
		if(mysql_affected_rows(&gMysql)==0)
			//debug only
			printf("uMasterJob %u",*uMasterJob);
	}
	mysql_free_result(res);

	return(0);

}//int SubmitSingleExtJob()


void CreateNewClient(structExtJobParameters *structExtParam)
{
	char cQuery[256];
	MYSQL_RES *res;

	//Create new tClient if needed
	if(structExtParam->uISPClient>1)
	{
		sprintf(cQuery,"SELECT uClient,cInfo FROM tClient WHERE uClient=%u",
			structExtParam->uISPClient);
		mysql_query(&gMysql,cQuery);
		if(mysql_errno(&gMysql))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql));
			return;
		}

		res=mysql_store_result(&gMysql);
		if(!mysql_num_rows(res)) 
		{
			sprintf(cQuery,"INSERT INTO tClient SET cLabel='%s',cInfo='(uISPClient)',uClient=%u,uOwner=1,"
				"uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
					structExtParam->cClientName,structExtParam->uISPClient);
			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				mysql_free_result(res);
				return;
			}
		}
		else
		{
			MYSQL_ROW field;

			//default
			sprintf(cQuery,"UPDATE tClient SET cLabel='%s',uModBy=1,uModDate=UNIX_TIMESTAMP(NOW())"
					" WHERE uClient=%u",structExtParam->cClientName,
						structExtParam->uISPClient);

			if((field=mysql_fetch_row(res)))
			{
				if(!strstr(field[1],"(uISPClient)"))
					sprintf(cQuery,"UPDATE tClient SET cLabel='%s',"
					"cInfo=CONCAT(cInfo,'\n','(uISPClient)'),uModBy=1,"
					"uModDate=UNIX_TIMESTAMP(NOW()) WHERE uClient=%u",
						structExtParam->cClientName,structExtParam->uISPClient);
			}

			mysql_query(&gMysql,cQuery);
			if(mysql_errno(&gMysql))
			{
				fprintf(stderr,"%s\n",mysql_error(&gMysql));
				mysql_free_result(res);
				return;
			}
		}
		mysql_free_result(res);
	}

}//void CreateNewClient(structExtJobParameters *structExtParam)

//We may want to add remote running, remote error and remotely queued job status to unxsVZ
//That involves changing dashboard and tjobfunc.h left panel logic.
//It also involves Update CLI changes and rpm update action.
void ProcessVZJobQueue(void)
{
	unsigned uJob=0;
	MYSQL gMysql2;

	printf("ProcessVZJobQueue() start\n");
	if(!TextConnectDb() && !TextConnectExtDb(&gMysql2,TEXT_CONNECT_UNXSVZ))
	{
		MYSQL_RES *res;
		MYSQL_ROW field;
		MYSQL_RES *res2;
		MYSQL_ROW field2;
		structExtJobParameters structExtParam;

		//debug only
		printf("ProcessVZJobQueue() connected ok\n");
	
		//mysqlISP_Waiting same as unxsVZ.tJobStatus.cLabel "RemoteWaiting" 10
		sprintf(gcQuery,"SELECT cJobName,cJobData,uJob,uOwner FROM tJob WHERE"
			" uJobStatus=%u AND uJobDate<=UNIX_TIMESTAMP(NOW())"
			" AND cJobName LIKE 'unxsVZ%%' LIMIT 10",unxsVZ_uREMOTEWAITING);
		mysql_query(&gMysql2,gcQuery);
	       	if(mysql_errno(&gMysql2))
		{
			fprintf(stderr,"%s\n",mysql_error(&gMysql2));
			return;
		}
		res=mysql_store_result(&gMysql2);
	        while((field=mysql_fetch_row(res)))
		{
			uJob=0;
			unsigned uZone=0;
			unsigned uOwner=0;
			unsigned uResource=0;
			unsigned uNSSet=0;
			char cZone[100]={""};

			InitializeParams(&structExtParam);
			sscanf(field[2],"%u",&uJob);
			sscanf(field[3],"%u",&uOwner);

			//unxsVZRR-FQName
			//cName is a fully qualified DNS RR name, i.e. it ends in the zone name.
			if(!strcmp("unxsVZRR-FQName",field[0]))
			{	
				//Update remote job queue running
				sprintf(gcQuery,"UPDATE tJob SET uJobStatus=%u,cRemoteMsg='unxsVZRR-FQName'"
						" WHERE uJob=%u",unxsVZ_uRUNNING,uJob);
				mysql_query(&gMysql2,gcQuery);
				if(mysql_errno(&gMysql2))
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql2));
					goto ErrorExit;
				}
	
				ParseExtParams(&structExtParam,field[1]);
				//debug only
				printf("%s(%u) data:\n",field[0],uJob);
				if(structExtParam.ucName)
					printf("\tcName=%s;\n",structExtParam.cName);
				if(structExtParam.ucuTTL)
					printf("\tcuTTL=%s;\n",structExtParam.cuTTL);
				if(structExtParam.ucRRType)
					printf("\tcRRType=%s;\n",structExtParam.cRRType);
				if(structExtParam.ucParam1)
					printf("\tcParam1=%s;\n",structExtParam.cParam1);
				if(structExtParam.ucNSSet)
					printf("\tcNSSet=%s;\n",structExtParam.cNSSet);
				if(structExtParam.ucView)
					printf("\tcView=%s;\n",structExtParam.cView);

				//Create local job mod zone, after checking and inserting new RR data.

				//cName must be FQDN
				if(structExtParam.cName[strlen(structExtParam.cName)-1]!='.')
				{
					fprintf(stderr,"%s does not end with a '.'\n",structExtParam.cName);
					goto ErrorExit;
				}

				//We need to determine the uZone from the FQName
				sprintf(gcQuery,"SELECT uZone,cZone,uNSSet,uView FROM tZone WHERE"
						" uView=(SELECT uView FROM tView WHERE cLabel='%s' LIMIT 1) AND"
						" uNSSet=(SELECT uNSSet FROM tNSSet WHERE cLabel='%s' LIMIT 1) AND"
						" LOCATE(cZone,'%s')>0 LIMIT 1",
							structExtParam.cView,
							structExtParam.cNSSet,
							structExtParam.cName);
				//debug only
				//printf("%s\n",gcQuery);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					goto ErrorExit;
				}
				res2=mysql_store_result(&gMysql);
				if((field2=mysql_fetch_row(res2)))
				{
					sscanf(field2[0],"%u",&uZone);
					sprintf(cZone,"%.99s",field2[1]);
					sscanf(field2[2],"%u",&uNSSet);
				}
				mysql_free_result(res2);

				if(!uZone)
				{
					fprintf(stderr,"No tZone.uZone for %s %s %s\n",
								structExtParam.cView,
								structExtParam.cNSSet,
								structExtParam.cName);
					goto ErrorExit;
				}
				//debug only
				printf("uZone=%u\n",uZone);

				//Do not add same record based on type and cName
				//If zone owner needs multiple records she will have to use an interface to do it.
				//Note that we will change cParam1 for existing record.
				sprintf(gcQuery,"SELECT uResource FROM tResource WHERE cNAME='%s' AND uZone=%u"
						" AND uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='%s' LIMIT 1)",
							structExtParam.cName,uZone,structExtParam.cRRType);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql));
					goto ErrorExit;
				}
				res2=mysql_store_result(&gMysql);
				if((field2=mysql_fetch_row(res2)))
					sscanf(field2[0],"%u",&uResource);
				mysql_free_result(res2);

				if(uResource)
				{
					sprintf(gcQuery,"UPDATE tResource SET cName='%s',cParam1='%s',uModBy=1,"
							"uModDate=UNIX_TIMESTAMP(NOW()),"
							"uTTL=%s WHERE uResource=%u",
							structExtParam.cName,structExtParam.cParam1,
							structExtParam.cuTTL,uResource);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						goto ErrorExit;
					}
					//debug only
					printf("Updating uResource=%u\n",uResource);
				}
				else
				{
					sprintf(gcQuery,"INSERT tResource SET cName='%s',cParam1='%s',uOwner=1,uCreatedBy=1,"
							"uCreatedDate=UNIX_TIMESTAMP(NOW()),cComment='unxsVZRR-FQName',"
							"uTTL=%s,"
							"uZone=%u,"
							"uRRType=(SELECT uRRType FROM tRRType WHERE cLabel='%s' LIMIT 1)",
							structExtParam.cName,structExtParam.cParam1,structExtParam.cuTTL,
								uZone,
								structExtParam.cRRType);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						fprintf(stderr,"%s\n",mysql_error(&gMysql));
						goto ErrorExit;
					}
					uResource=mysql_insert_id(&gMysql);
					printf("Inserting uResource=%u\n",uResource);
				}
				UpdateSerialNum(uZone);

				//Submit zone mod job
				if(SubmitJob("Modify",uNSSet,cZone,0,0))
				{
					fprintf(stderr,"SubmitJob() failed.\n");
					goto ErrorExit;
				}

				//Update remote job queue done ok
				sprintf(gcQuery,"UPDATE tJob SET uJobStatus=%u,cRemoteMsg='unxsVZRR-FQName ok'"
						" WHERE uJob=%u",unxsVZ_uDONEOK,uJob);
				mysql_query(&gMysql2,gcQuery);
				if(mysql_errno(&gMysql2))
				{
					fprintf(stderr,"%s\n",mysql_error(&gMysql2));
					goto ErrorExit;
				}
				goto NormalExit;
			}//unxsVZRR-FQName
		}//while
		mysql_free_result(res);
	
	}//connected

//Organize this later with another function
ErrorExit:
	//debug only
	sprintf(gcQuery,"UPDATE tJob SET uJobStatus=%u,cRemoteMsg='unxsBind ext jobqueue error'"
			" WHERE uJob=%u",unxsVZ_uERROR,uJob);
	mysql_query(&gMysql2,gcQuery);
	if(mysql_errno(&gMysql2))
		fprintf(stderr,"%s\n",mysql_error(&gMysql2));

NormalExit:
	printf("ProcessVZJobQueue() done\n");
	mysql_close(&gMysql);

}//void ProcessVZJobQueue(void)
