#ifndef __TABLES_H__
#define __TABLES_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TABLES_MAX_NUMBER_OF_PIDS_IN_PAT    20 	    /* Max number of PMT pids in one PAT table */
#define TABLES_MAX_NUMBER_OF_ELEMENTARY_PID 20      /* Max number of elementary pids in one PMT table */
#define TABLES_MAX_NUMBER_OF_EVENTS_IN_EIT  20		/* Max number of event info in EIT table */

/**
 * @brief Enumeration of possible tables parser error codes
 */
typedef enum _ParseErrorCode
{
    TABLES_PARSE_ERROR = 0,                         /* TABLES_PARSE_ERROR */
	TABLES_PARSE_OK = 1                             /* TABLES_PARSE_OK */
}ParseErrorCode;

/**
 * @brief Structure that defines PAT Table Header
 */
typedef struct _PatHeader
{
    uint8_t     tableId;                            /* The type of table */
    uint8_t     sectionSyntaxIndicator;             /* The format of the table section to follow */
    uint16_t    sectionLength;                      /* The length of the table section beyond this field */
    uint16_t    transportStreamId;                  /* Transport stream identifier */
    uint8_t     versionNumber;                      /* The version number the private table section */
    uint8_t     currentNextIndicator;               /* Signals what a particular table will look like when it next changes */
    uint8_t     sectionNumber;                      /* Section number */
    uint8_t     lastSectionNumber;                  /* Signals the last section that is valid for a particular MPEG-2 private table */
}PatHeader;

/**
 * @brief Structure that defines PAT service info
 */
typedef struct _PatServiceInfo
{    
    uint16_t    programNumber;                      /* Identifies each service present in a transport stream */
    uint16_t    pid;                                /* Pid of Program Map table section or pid of Network Information Table  */
}PatServiceInfo;

/**
 * @brief Structure that defines PAT table
 */
typedef struct _PatTable
{    
    PatHeader patHeader;                                                     /* PAT Table Header */
    PatServiceInfo patServiceInfoArray[TABLES_MAX_NUMBER_OF_PIDS_IN_PAT];    /* Services info presented in PAT table */
    uint8_t serviceInfoCount;                                                /* Number of services info presented in PAT table */
}PatTable;

/**
 * @brief Structure that defines PMT table header
 */
typedef struct _PmtTableHeader
{
    uint8_t tableId;
    uint8_t sectionSyntaxIndicator;
    uint16_t sectionLength;
    uint16_t programNumber;
    uint8_t versionNumber;
    uint8_t currentNextIndicator;
    uint8_t sectionNumber;
    uint8_t lastSectionNumber;
    uint16_t pcrPid;
    uint16_t programInfoLength;
}PmtTableHeader;

/**
 * @brief Structure that defines PMT elementary info
 */
typedef struct _PmtElementaryInfo
{
    uint8_t streamType;
    uint16_t elementaryPid;
    uint16_t esInfoLength;
}PmtElementaryInfo;

/**
 * @brief Structure that defines PMT table
 */
typedef struct _PmtTable
{
    PmtTableHeader pmtHeader;
    PmtElementaryInfo pmtElementaryInfoArray[TABLES_MAX_NUMBER_OF_ELEMENTARY_PID];
    uint8_t elementaryInfoCount;
}PmtTable;

/**
 * @brief Structure that defines EIT Table Header
 */
typedef struct _EitHeader{
    uint8_t     tableId;
    uint8_t     sectionSyntaxIndicator;
    uint16_t    sectionLength;
    uint16_t    serviceId;
    uint8_t     versionNumber;
    uint8_t     currentNextIndicator;
    uint8_t     sectionNumber;
    uint8_t     lastSectionNumber;
    uint16_t    transportStreamId;
    uint16_t    originalNetworkId;
    uint8_t     segmentLastSectionNumber;
    uint8_t     lastTabeId;
}EitHeader;

/**
 * @brief Structure that defines EIT Short Event Descriptor
 */
typedef struct _Short_Event_Descriptor{
	uint8_t		descriptorTag;
	uint8_t		descriptorLength;
	uint32_t	Iso639LanguageCode;
	uint8_t		eventNameLength;
	char		eventName[256];
}Short_Event_Descriptor;

/**
 * @brief Structure that defines EIT event info
 */
typedef struct _EitEventInfo{
	uint16_t 	eventId;
	uint8_t 	startTime[5];
	uint8_t		duration[3];
	uint8_t		runningStatus;
	uint8_t		freeCaMode;
	uint16_t	descriptorsLoopLength;
	Short_Event_Descriptor	shortEventDescriptor;
}EitEventInfo;

/**
 * @brief Structure that defines EIT table
 */
typedef struct  _EitTable{
    EitHeader       eitHeader;
    EitEventInfo    eitEventInfoArray[TABLES_MAX_NUMBER_OF_EVENTS_IN_EIT];
	uint8_t 		eventInfoCount;
}EitTable;


typedef struct _CurrentEventsInfo{
    uint16_t    serviceId;
	uint8_t 	startTime[5];
	char		eventName[256];
}CurrentEventsInfo;

/**
 * @brief Parse EIT header
 *
 * @param [in]  eitHeaderBuffer - Buffer that contains EIT header
 * @param [out] eitHeader - EIT table header
 * @return tables error code
 */
ParseErrorCode parseEitHeader(const uint8_t* eitHeaderBuffer, EitHeader* eitHeader);

/**
 * @brief Parse EIT event info
 *
 * @param [in]  eitEventInfoBuffer - Buffer that contains eit event info
 * @param [out] eitEventInfo - EIT event info
 * @return tables error code
 */
ParseErrorCode parseEitEventInfo(const uint8_t* eitEventInfoBuffer, EitEventInfo* eitEventInfo);

/**
 * @brief Parse EIT short event descriptor
 *
 * @param [in]  shortEventDescriptorBuffer - Buffer that contains eit short event descriptor
 * @param [out] eitEventInfo - EIT event info
 * @return tables error code
 */
ParseErrorCode parseShortEventDescriptor(const uint8_t* shortEventDescriptorBuffer, EitEventInfo* eitEventInfo);

/**
 * @brief Parse EIT table
 *
 * @param [in]  eitSectionBuffer - Buffer that contains EIT table section
 * @param [out] eitTable - EIT table
 * @return tables error code
 */
ParseErrorCode parseEitTable(const uint8_t* eitSectionBuffer, EitTable* eitTable);

/**
 * @brief Print EIT table
 *
 * @param [in] eitTable - EIT table to print
 * @return tables error code
 */
ParseErrorCode printEitTable(EitTable* eitTable);


/**
 * @brief  Parse PAT header.
 * 
 * @param  [in]   patHeaderBuffer Buffer that contains PAT header
 * @param  [out]  patHeader PAT header
 * @return tables error code
 */
ParseErrorCode parsePatHeader(const uint8_t* patHeaderBuffer, PatHeader* patHeader);

/**
 * @brief  Parse PAT Service information.
 * 
 * @param  [in]   patServiceInfoBuffer Buffer that contains PAT Service info
 * @param  [out]  descriptor PAT Service info
 * @return tables error code
 */
ParseErrorCode parsePatServiceInfo(const uint8_t* patServiceInfoBuffer, PatServiceInfo* patServiceInfo);

/**
 * @brief  Parse PAT Table.
 * 
 * @param  [in]   patSectionBuffer Buffer that contains PAT table section
 * @param  [out]  patTable PAT Table
 * @return tables error code
 */
ParseErrorCode parsePatTable(const uint8_t* patSectionBuffer, PatTable* patTable);

/**
 * @brief  Print PAT Table
 * 
 * @param  [in]   patTable PAT table to be printed
 * @return tables error code
 */
ParseErrorCode printPatTable(PatTable* patTable);

/**
 * @brief Parse pmt table
 *
 * @param [in]  pmtHeaderBuffer Buffer that contains PMT header
 * @param [out] pmtHeader PMT table header
 * @return tables error code
 */
ParseErrorCode parsePmtHeader(const uint8_t* pmtHeaderBuffer, PmtTableHeader* pmtHeader);

/**
 * @brief Parse PMT elementary info
 *
 * @param [in]  pmtElementaryInfoBuffer Buffer that contains pmt elementary info
 * @param [out] PMT elementary info
 * @return tables error code
 */
ParseErrorCode parsePmtElementaryInfo(const uint8_t* pmtElementaryInfoBuffer, PmtElementaryInfo* pmtElementaryInfo);

/**
 * @brief Parse PMT table
 *
 * @param [in]  pmtSectionBuffer Buffer that contains pmt table section
 * @param [out] pmtTable PMT table
 * @return tables error code
 */
ParseErrorCode parsePmtTable(const uint8_t* pmtSectionBuffer, PmtTable* pmtTable);

/**
 * @brief Print PMT table
 *
 * @param [in] pmtTable PMT table
 * @return tables error code
 */
ParseErrorCode printPmtTable(PmtTable* pmtTable);

#endif /* __TABLES_H__ */


