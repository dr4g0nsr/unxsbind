/*
FILE
        $Id: local.h.default 1581 2010-10-26 14:55:47Z Gary $
PURPOSE
        Localization constants
LEGAL
        Copyright 2002-2009 Gary Wallis and Hugo Urquiza for Unixservice
	GPL license applies (see www.fsf.org for more info.)
	License included with this distribution.
*/

//Set these up to match your mySQL database settings and location
//NULL is for stream socket on local server. Much faster than localhost tcp/ip.
#define DBIP0 NULL
#define DBIP1 NULL
#define DBNAME "idns"
#define DBLOGIN "idns"
#define DBPASSWD "wsxedc"
#define DBPORT 0
#define DBSOCKET NULL
//Example for host running two mysqld
//See makefile and mysqlrad.h CustomExample sections
//#define DBPORT 4306
//#define DBSOCKET "/var/lib/mysql/mysql4.sock"

//Master tClient
#define TCLIENT "tClient"
#define TAUTHORIZE "tAuthorize"
//#define TCLIENT "mysqlisp2.tClient"
//#define TAUTHORIZE "mysqlisp2.tAuthorize"

//LDAP Login Configuration 
//tClient must contain the Organization returned by
//LDAP as configured below. If not the login even if
//correct as far as LDAP BIND goes will not work.
//Use the log file defined below to debug your configuration.
//Undefine the following line to not compile in LDAP support
//#define cLDAPURI "ldap://127.0.0.1"
//
#ifdef cLDAPURI
	//Log LDAP login data
	#define DEBUG_LDAP
	#ifdef DEBUG_LDAP
		//Must be created with apache write perms
		//and should not be readable by anyone except root.
		//touch, chmod 240 and chown apache:root, for example.
		#define cVDNSORGLOGFILE "/var/log/unxsBindLDAP.log"
	#endif

	//Use OPenLDAP client "ldapsearch" to determine the following settings:

	//Here is a hacked example where memberOf was set in a SN attr.
	//ex. ldapsearch -LLL -x -h 127.0.0.1  -D "cn=johndoe,ou=people,dc=example,dc=com" -w "123123" 
	//	-b "ou=people,dc=example,dc=com" "cn=johndoe" sn
	//returns:
	//dn: cn=johndoe,ou=people,dc=example,dc=com
	//sn: Doe
	//sn: Doe-Jones
	//sn: memberOf=DNS ASP Company
	//
	//The cLDAPLINEPATTERN would be memberOf
	//The cLDAPPREFIXPATTERN would be memberOf=


	//This is the filter as specified in man ldapsearch
	#define cLDAPFILTER "(objectClass=*)"

	//This is the search base DN
	#define cLDAPSEARCHDN "ou=people,dc=example,dc=com";

	//LDAP search attribute (single attr only) as specified in man ldapsearch
	#define cLDAPATTR0 "memberOf"

	//This is used to refine the search result set
	#define cLDAPLINEPATTERN "_admins"

	//This is the prefix of the tClient.cLabel organization base label
	//as returned by ldapsearch. Must end with a "," or be the last value.
	#define cLDAPPREFIXPATTERN "memberOf="

	//Since the login usually is not LDAP BIND ready then
	//cLDAPLOGINPREFIX and cLDAPLOGINSUFFIX can be used.
	//Login format examples: 
	#define cLDAPLOGINPREFIX "cn="
	#define cLDAPLOGINSUFFIX ",ou=people,dc=example,dc=com"
	//ex1: cn=johndoe,ou=people,dc=example,dc=com (DN)
	//#define cLDAPLOGINPREFIX ""
	//#define cLDAPLOGINSUFFIX "@example.com"
	//ex2: johndoe@example.com (UPN)
	//#define cLDAPLOGINPREFIX "example.com\"
	//#define cLDAPLOGINSUFFIX ""
	//ex3: example.com\johndoe (Windows)

	//Org format: In case your LDAP data does not match what is configured in tClient.cLabel
	//then cLDAPORGPREFIX and cLDAPORGSUFFIX can be used. If not leave as empty strings "".
	#define	cLDAPORGPREFIX "The City of "
	#define	cLDAPORGSUFFIX ", California"
	//ex. For "...memberOf=Los Angeles,..." would translate into "The City of Los Angeles, California"

	//Search filter is composed with the login presented along with:
	#define cLDAPFILTERPREFIX "cn="
	#define cLDAPFILTERSUFFIX ""
	//ex. cn=johndoe
	//See man ldapsearch for more information
#endif

//If you have Apache/mod_ssl uncomment below much better unless intranet
//ultra firewalled and secured.
//#define SSLONLY

//Set the default hostmaster for zone operations
#define HOSTMASTER "dns.gromnet.net"

//Set to your ISP name if you like
#define EXTISPNAME "gromnet.net"
#define EXTISPURL "gromnet.net"

//iDNS can report to us once a week some useful for
//debugging and development statistics.
//This is very useful to us and yourself for improving
//and developing this software
//On any mysql_error will send us the problem so we can fix it.
//#define DEBUG_REPORT_STATS_OFF

//If you would like to use your native language edit language.h or if 
//appropiate language2.h. cp it to language.h and recompile. This only covers 
//60-70% of language used. The rest can be changed in the .h schema independent 
//table related files.

//For openisp staff mod only. Staff: Remember to change makefile also.
#define RELEASE "Release 3.1/rpm-3.1"
#define REV "3.1/rpm-3.1"
