#include "tables.h"

ParseErrorCode parsePatHeader(const uint8_t* patHeaderBuffer, PatHeader* patHeader)
{    
    if(patHeaderBuffer==NULL || patHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    patHeader->tableId = (uint8_t)* patHeaderBuffer; 
    if (patHeader->tableId != 0x00)
    {
        printf("\n%s : ERROR it is not a PAT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    lower8Bits = (uint8_t)(*(patHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    patHeader->sectionSyntaxIndicator = lower8Bits & 0x01;

    higher8Bits = (uint8_t) (*(patHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patHeader->sectionLength = all16Bits & 0x0FFF;
    
    higher8Bits = (uint8_t) (*(patHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patHeader->transportStreamId = all16Bits & 0xFFFF;
    
    lower8Bits = (uint8_t) (*(patHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    patHeader->versionNumber = lower8Bits & 0x1F;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 5));
    patHeader->currentNextIndicator = lower8Bits & 0x01;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 6));
    patHeader->sectionNumber = lower8Bits & 0xFF;

    lower8Bits = (uint8_t) (*(patHeaderBuffer + 7));
    patHeader->lastSectionNumber = lower8Bits & 0xFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePatServiceInfo(const uint8_t* patServiceInfoBuffer, PatServiceInfo* patServiceInfo)
{
    if(patServiceInfoBuffer==NULL || patServiceInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    higher8Bits = (uint8_t) (*(patServiceInfoBuffer));
    lower8Bits = (uint8_t) (*(patServiceInfoBuffer + 1));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patServiceInfo->programNumber = all16Bits & 0xFFFF; 

    higher8Bits = (uint8_t) (*(patServiceInfoBuffer + 2));
    lower8Bits = (uint8_t) (*(patServiceInfoBuffer + 3));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    patServiceInfo->pid = all16Bits & 0x1FFF;
    
    return TABLES_PARSE_OK;
}

ParseErrorCode parsePatTable(const uint8_t* patSectionBuffer, PatTable* patTable)
{
    uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(patSectionBuffer==NULL || patTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parsePatHeader(patSectionBuffer,&(patTable->patHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing PAT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 12 /*PAT header size*/ - 3 /*Not in section length*/;
    currentBufferPosition = (uint8_t *)(patSectionBuffer + 8); /* Position after last_section_number */
    patTable->serviceInfoCount = 0; /* Number of services info presented in PAT table */
    
    while(parsedLength < patTable->patHeader.sectionLength)
    {
        if(patTable->serviceInfoCount > TABLES_MAX_NUMBER_OF_PIDS_IN_PAT - 1)
        {
            printf("\n%s : ERROR there is not enough space in PAT structure for Service info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
        }
        
        if(parsePatServiceInfo(currentBufferPosition, &(patTable->patServiceInfoArray[patTable->serviceInfoCount])) == TABLES_PARSE_OK)
        {
            currentBufferPosition += 4; /* Size from program_number to pid */
            parsedLength += 4; /* Size from program_number to pid */
            patTable->serviceInfoCount ++;
        }    
    }
    
    return TABLES_PARSE_OK;
}

ParseErrorCode printPatTable(PatTable* patTable)
{
    uint8_t i=0;
    
    if(patTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************PAT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",patTable->patHeader.tableId);
    printf("section_length           |      %d\n",patTable->patHeader.sectionLength);
    printf("transport_stream_id      |      %d\n",patTable->patHeader.transportStreamId);
    printf("section_number           |      %d\n",patTable->patHeader.sectionNumber);
    printf("last_section_number      |      %d\n",patTable->patHeader.lastSectionNumber);
    
    for (i=0; i<patTable->serviceInfoCount;i++)
    {
        printf("-----------------------------------------\n");
        printf("program_number           |      %d\n",patTable->patServiceInfoArray[i].programNumber);
        printf("pid                      |      %d\n",patTable->patServiceInfoArray[i].pid); 
    }
    printf("\n********************PAT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}


ParseErrorCode parsePmtHeader(const uint8_t* pmtHeaderBuffer, PmtTableHeader* pmtHeader)
{

    if(pmtHeaderBuffer==NULL || pmtHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

    pmtHeader->tableId = (uint8_t)* pmtHeaderBuffer; 
    if (pmtHeader->tableId != 0x02)
    {
        printf("\n%s : ERROR it is not a PMT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    pmtHeader->sectionSyntaxIndicator = lower8Bits & 0x01;
    
    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->sectionLength = all16Bits & 0x0FFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->programNumber = all16Bits & 0xFFFF;
    
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    pmtHeader->versionNumber = lower8Bits & 0x1F;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 5));
    pmtHeader->currentNextIndicator = lower8Bits & 0x01;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 6));
    pmtHeader->sectionNumber = lower8Bits & 0xFF;

    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 7));
    pmtHeader->lastSectionNumber = lower8Bits & 0xFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 8));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 9));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->pcrPid = all16Bits & 0xFFFF;

    higher8Bits = (uint8_t) (*(pmtHeaderBuffer + 10));
    lower8Bits = (uint8_t) (*(pmtHeaderBuffer + 11));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtHeader->programInfoLength = all16Bits & 0x0FFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePmtElementaryInfo(const uint8_t* pmtElementaryInfoBuffer, PmtElementaryInfo* pmtElementaryInfo)
{
    if(pmtElementaryInfoBuffer==NULL || pmtElementaryInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;
    
    pmtElementaryInfo->streamType = (uint8_t) (*(pmtElementaryInfoBuffer));

    higher8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 1));
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtElementaryInfo->elementaryPid = all16Bits & 0x1FFF; 

    higher8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 3));
    lower8Bits = (uint8_t) (*(pmtElementaryInfoBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    pmtElementaryInfo->esInfoLength = all16Bits & 0x0FFF;

    return TABLES_PARSE_OK;
}

ParseErrorCode parsePmtTable(const uint8_t* pmtSectionBuffer, PmtTable* pmtTable)
{
    uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(pmtSectionBuffer==NULL || pmtTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parsePmtHeader(pmtSectionBuffer,&(pmtTable->pmtHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing PMT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 12 + pmtTable->pmtHeader.programInfoLength /*PMT header size*/ + 4 /*CRC size*/ - 3 /*Not in section length*/;
    currentBufferPosition = (uint8_t *)(pmtSectionBuffer + 12 + pmtTable->pmtHeader.programInfoLength); /* Position after last descriptor */
    pmtTable->elementaryInfoCount = 0; /* Number of elementary info presented in PMT table */
    
    while(parsedLength < pmtTable->pmtHeader.sectionLength)
    {
        if(pmtTable->elementaryInfoCount > TABLES_MAX_NUMBER_OF_ELEMENTARY_PID - 1)
        {
            printf("\n%s : ERROR there is not enough space in PMT structure for elementary info\n", __FUNCTION__);
            return TABLES_PARSE_ERROR;
        }
        
        if(parsePmtElementaryInfo(currentBufferPosition, &(pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount])) == TABLES_PARSE_OK)
        {
            currentBufferPosition += 5 + pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount].esInfoLength; /* Size from stream type to elemntary info descriptor*/
            parsedLength += 5 + pmtTable->pmtElementaryInfoArray[pmtTable->elementaryInfoCount].esInfoLength; /* Size from stream type to elementary info descriptor */
            pmtTable->elementaryInfoCount++;
        }    
    }

    return TABLES_PARSE_OK;
}

ParseErrorCode parseEitHeader(const uint8_t* eitHeaderBuffer, EitHeader* eitHeader)
{
	//printf("parse eit heder: eitHeder=%d\n", eitHeader);
	
	if(eitHeaderBuffer==NULL || eitHeader==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

	uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

	//table id
    eitHeader->tableId = (uint8_t)* eitHeaderBuffer; 
    if (eitHeader->tableId != 0x4E)
    {
        printf("\n%s : ERROR it is not a EIT Table\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }

	//printf("parse eit heder: pocelo parsiranje eit hedera.\n");	

	//section sintax indicator
	lower8Bits = (uint8_t)(*(eitHeaderBuffer + 1));
    lower8Bits = lower8Bits >> 7;
    eitHeader->sectionSyntaxIndicator = lower8Bits & 0x01;

	//section length
	higher8Bits = (uint8_t) (*(eitHeaderBuffer + 1));
    lower8Bits = (uint8_t) (*(eitHeaderBuffer + 2));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitHeader->sectionLength = all16Bits & 0x0FFF;
    
	//service id
	higher8Bits = (uint8_t) (*(eitHeaderBuffer + 3));
    lower8Bits = (uint8_t) (*(eitHeaderBuffer + 4));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitHeader->serviceId = all16Bits & 0xFFFF;
	
	//version number
	lower8Bits = (uint8_t) (*(eitHeaderBuffer + 5));
    lower8Bits = lower8Bits >> 1;
    eitHeader->versionNumber = lower8Bits & 0x1F;
	
	//current next indicator
	lower8Bits = (uint8_t) (*(eitHeaderBuffer + 5));
    eitHeader->currentNextIndicator = lower8Bits & 0x01;
	
	//section number
	lower8Bits = (uint8_t) (*(eitHeaderBuffer + 6));
    eitHeader->sectionNumber = lower8Bits & 0xFF;

	//last section number
	lower8Bits = (uint8_t) (*(eitHeaderBuffer + 7));
    eitHeader->lastSectionNumber = lower8Bits & 0xFF;

	//transport stream id
	higher8Bits = (uint8_t) (*(eitHeaderBuffer + 8));
    lower8Bits = (uint8_t) (*(eitHeaderBuffer + 9));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitHeader->transportStreamId = all16Bits & 0xFFFF;

	//original network id
	higher8Bits = (uint8_t) (*(eitHeaderBuffer + 10));
    lower8Bits = (uint8_t) (*(eitHeaderBuffer + 11));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitHeader->originalNetworkId = all16Bits & 0xFFFF;

	//segment last section number
	lower8Bits = (uint8_t) (*(eitHeaderBuffer + 12));
    eitHeader->segmentLastSectionNumber = lower8Bits & 0xFF;

	//last table id
	lower8Bits = (uint8_t) (*(eitHeaderBuffer + 13));
    eitHeader->lastTableId = lower8Bits & 0xFF;

	//printf("parseEitHeder : zavrseno parsiranje eit hedera\n");
	return TABLES_PARSE_OK;
}

ParseErrorCode parseEitServiceInfo(const uint8_t* eitServiceInfoBuffer, EitServiceInfo* eitServiceInfo,	uint32_t* parsedLen)
{
	if(eitServiceInfoBuffer==NULL || eitServiceInfo==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    uint8_t lower8Bits = 0;
    uint8_t higher8Bits = 0;
    uint16_t all16Bits = 0;

	//event id
    higher8Bits = (uint8_t) (*(eitServiceInfoBuffer));
    lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 1));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitServiceInfo->eventId = all16Bits & 0xFFFF; 

	//start time
	eitServiceInfo->startTime = 0x0;
    lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 2));
    eitServiceInfo->startTime += lower8Bits & 0xFF;
	eitServiceInfo->startTime = eitServiceInfo->startTime << 8;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 3));
    eitServiceInfo->startTime += lower8Bits & 0xFF;
	eitServiceInfo->startTime = eitServiceInfo->startTime << 8;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 4));
    eitServiceInfo->startTime += lower8Bits & 0xFF;
	eitServiceInfo->startTime = eitServiceInfo->startTime << 8;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 5));
    eitServiceInfo->startTime += lower8Bits & 0xFF;
	eitServiceInfo->startTime = eitServiceInfo->startTime << 8;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 6));
    eitServiceInfo->startTime += lower8Bits & 0xFF;

	//duration
	eitServiceInfo->duration = 0x0;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 7));
	eitServiceInfo->duration += lower8Bits & 0xFF;
	eitServiceInfo->duration = eitServiceInfo->duration << 8;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 8));
	eitServiceInfo->duration += lower8Bits & 0xFF;
	eitServiceInfo->duration = eitServiceInfo->duration << 8;
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 9));
	eitServiceInfo->duration += lower8Bits & 0xFF;

	//running status
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 10));
    eitServiceInfo->runningStatus = (lower8Bits >> 5) & 0x07;

	//free CA MODE
	lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 10));
    eitServiceInfo->freeCAMode = (lower8Bits >> 4) & 0x01;

	//descriptors loop length
	higher8Bits = (uint8_t) (*(eitServiceInfoBuffer + 10));
    lower8Bits = (uint8_t) (*(eitServiceInfoBuffer + 11));
    all16Bits = (uint16_t) ((higher8Bits << 8) + lower8Bits);
    eitServiceInfo->descriptorsLoopLength = all16Bits & 0x0FFF;

	/*--------------------------------------------DESKRIPTOR-----------------------------------*/

	const uint8_t* pointer = (eitServiceInfoBuffer + 12);

	uint8_t descriptor_tag = *pointer;
	
	while(descriptor_tag != 0x4D)
	{
		pointer += 2 + *(pointer+1);
		descriptor_tag = *pointer;
	}
	/*
	if(descriptor_tag == 0x4D)
	{
		printf("Nasao sam ga\n");
	}
	*/

	//descriptor tag
	lower8Bits = (uint8_t) (*(pointer));
    eitServiceInfo->eitDescriptor.descriptorTag = lower8Bits & 0xFF;

	//descriptor length
	lower8Bits = (uint8_t) (*(pointer + 1));
    eitServiceInfo->eitDescriptor.descriptorLength = lower8Bits & 0xFF;

	//IS0 639 language code
	eitServiceInfo->eitDescriptor.ISO639LanguageCode = 0x0;
	lower8Bits = (uint8_t) (*(pointer + 2));
    eitServiceInfo->eitDescriptor.ISO639LanguageCode += lower8Bits & 0xFF;
	eitServiceInfo->eitDescriptor.ISO639LanguageCode = eitServiceInfo->eitDescriptor.ISO639LanguageCode << 8;
	lower8Bits = (uint8_t) (*(pointer + 3));
    eitServiceInfo->eitDescriptor.descriptorLength += lower8Bits & 0xFF;
	eitServiceInfo->eitDescriptor.ISO639LanguageCode = eitServiceInfo->eitDescriptor.ISO639LanguageCode << 8;
	lower8Bits = (uint8_t) (*(pointer + 4));
    eitServiceInfo->eitDescriptor.descriptorLength += lower8Bits & 0xFF;
	eitServiceInfo->eitDescriptor.ISO639LanguageCode = eitServiceInfo->eitDescriptor.ISO639LanguageCode & 0x0FFF;

	//event name length
	lower8Bits = (uint8_t) (*(pointer + 5));
    eitServiceInfo->eitDescriptor.eventNameLength = lower8Bits & 0xFF;

	memset(eitServiceInfo->eitDescriptor.eventNameChar, '\0', 512 * sizeof(char));
	//event name char
	int nameIterator;
	for(nameIterator = 0; nameIterator < eitServiceInfo->eitDescriptor.eventNameLength; nameIterator++)
	{
		eitServiceInfo->eitDescriptor.eventNameChar[nameIterator] = (uint8_t) (*(pointer + 6 + nameIterator));
	}
		
	//text length
	lower8Bits = (uint8_t) (*(pointer + 5 + nameIterator + 1));
    eitServiceInfo->eitDescriptor.textLength = lower8Bits & 0xFF;

	memset(eitServiceInfo->eitDescriptor.textChar, '\0', 512 * sizeof(char));
	//text char
	int textIterator;
	for(textIterator = 0; textIterator < eitServiceInfo->eitDescriptor.textLength; textIterator++)
	{
		if(*(pointer + 6 + nameIterator + textIterator) != '\0')
		{
			eitServiceInfo->eitDescriptor.textChar[textIterator] = (uint8_t) (*(pointer + 6 + nameIterator + textIterator + 1));
		}
	}

	//printf("Samo opis serije da se isprinta %s\n", eitServiceInfo->eitDescriptor.textChar);

	eitServiceInfoBuffer += 12 + eitServiceInfo->descriptorsLoopLength;
	(*parsedLen) += 12 + eitServiceInfo->descriptorsLoopLength;	
	//printf("CurrentBusPro i parseLen iz funkcije za parsiranje: %d %d\n", eitServiceInfoBuffer, (*parsedLen));

    return TABLES_PARSE_OK;
}

ParseErrorCode parseEitTable(const uint8_t* eitSectionBuffer, EitTable* eitTable)
{
	uint8_t * currentBufferPosition = NULL;
    uint32_t parsedLength = 0;
    
    if(eitSectionBuffer==NULL || eitTable==NULL)
    {
        printf("\n%s : ERROR received parameters are not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    if(parseEitHeader(eitSectionBuffer,&(eitTable->eitHeader))!=TABLES_PARSE_OK)
    {
        printf("\n%s : ERROR parsing EIT header\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    parsedLength = 14; //tolika je velicina zaglavlja
    currentBufferPosition = (uint8_t *)(eitSectionBuffer + parsedLength); /* Position after last_section_number */
    eitTable->serviceInfoCount = 0; /* Number of services info presented in PAT table */
	//printf("vrednost za eitTable->serviceInfoCount odmah nakonn inicijalizacije je: %d\n", eitTable->serviceInfoCount);
    
    while(parsedLength < eitTable->eitHeader.sectionLength - 1)
    {
        if(parseEitServiceInfo(currentBufferPosition, &(eitTable->eitServiceInfoArray[eitTable->serviceInfoCount]), &parsedLength) == TABLES_PARSE_OK)
        {
			//printf("prosao je parseServiceInfo[%d]\n", eitTable->serviceInfoCount);
            //currentBufferPosition += 12; /* Size from program_number to pid */
            //parsedLength += 12; /* Size from program_number to pid */
            eitTable->serviceInfoCount ++;
        }    
		//printf("Parsed length: \n", parsedLength);
		//printf("CurrentBufferPosition \n", currentBufferPosition);
		//printf("Duzina eit tabele cele section length + ostatak zaglavlja = %d",eitTable->eitHeader.sectionLength + 3);
    }
	//printf("Parsed length: %d\n", parsedLength);
	//printf("CurrentBufferPosition %d\n", currentBufferPosition);
    
    return TABLES_PARSE_OK;	
}

ParseErrorCode printEitTable(EitTable* eitTable)
{
	uint8_t i=0;
    
    if(eitTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************EIT TABLE SECTION********************\n");
    printf("table_id                    |      %d\n",eitTable->eitHeader.tableId);
    printf("section_syntax_indicator    |      %d\n",eitTable->eitHeader.sectionSyntaxIndicator);
    printf("section_length              |      %d\n",eitTable->eitHeader.sectionLength);
    printf("service_id           		|      %d\n",eitTable->eitHeader.serviceId);
    printf("version_number      		|      %d\n",eitTable->eitHeader.versionNumber);
	printf("current_next_indicator      |      %d\n",eitTable->eitHeader.currentNextIndicator);
    printf("section_number           	|      %d\n",eitTable->eitHeader.sectionNumber);
    printf("last_section_number      	|      %d\n",eitTable->eitHeader.lastSectionNumber);
    printf("transport_stream_id         |      %d\n",eitTable->eitHeader.transportStreamId);
    printf("original_network_id      	|      %d\n",eitTable->eitHeader.originalNetworkId);
	printf("segment_last_section_number |      %d\n",eitTable->eitHeader.segmentLastSectionNumber);
    printf("last_table_id           	|      %d\n",eitTable->eitHeader.lastTableId);
    
    for (i=0; i<eitTable->serviceInfoCount;i++)
    {
    	printf("-----------------------------------------\n");
        printf("event_id           			|      %d\n",eitTable->eitServiceInfoArray[i].eventId);
        printf("start_time         			|      %d\n",eitTable->eitServiceInfoArray[i].startTime);
		printf("duration           			|      %d\n",eitTable->eitServiceInfoArray[i].duration);
        printf("running_status              |      %d\n",eitTable->eitServiceInfoArray[i].runningStatus); 
		printf("free_CA_mode           		|      %d\n",eitTable->eitServiceInfoArray[i].freeCAMode);
        printf("descriptor_loop_length      |      %d\n",eitTable->eitServiceInfoArray[i].descriptorsLoopLength);
		printf("descriptor_tag           	|      %d\n",eitTable->eitServiceInfoArray[i].eitDescriptor.descriptorTag);
        printf("descriptor_length         	|      %d\n",eitTable->eitServiceInfoArray[i].eitDescriptor.descriptorLength);
		printf("ISO_693_language_code       |      %d\n",eitTable->eitServiceInfoArray[i].eitDescriptor.ISO639LanguageCode);
        printf("event_name_length           |      %d\n",eitTable->eitServiceInfoArray[i].eitDescriptor.eventNameLength); 
		printf("event_name_char           	|      %s\n",eitTable->eitServiceInfoArray[i].eitDescriptor.eventNameChar);
        printf("text_length      			|      %d\n",eitTable->eitServiceInfoArray[i].eitDescriptor.textLength);
		printf("text_char      				|      %s\n",eitTable->eitServiceInfoArray[i].eitDescriptor.textChar);		 
    }
    printf("\n********************EIT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}


ParseErrorCode printPmtTable(PmtTable* pmtTable)
{
    uint8_t i=0;
    
    if(pmtTable==NULL)
    {
        printf("\n%s : ERROR received parameter is not ok\n", __FUNCTION__);
        return TABLES_PARSE_ERROR;
    }
    
    printf("\n********************PMT TABLE SECTION********************\n");
    printf("table_id                 |      %d\n",pmtTable->pmtHeader.tableId);
    printf("section_length           |      %d\n",pmtTable->pmtHeader.sectionLength);
    printf("program_number           |      %d\n",pmtTable->pmtHeader.programNumber);
    printf("section_number           |      %d\n",pmtTable->pmtHeader.sectionNumber);
    printf("last_section_number      |      %d\n",pmtTable->pmtHeader.lastSectionNumber);
    printf("program_info_legth       |      %d\n",pmtTable->pmtHeader.programInfoLength);
    
    for (i=0; i<pmtTable->elementaryInfoCount;i++)
    {
        printf("-----------------------------------------\n");
        printf("stream_type              |      %d\n",pmtTable->pmtElementaryInfoArray[i].streamType);
        printf("elementary_pid           |      %d\n",pmtTable->pmtElementaryInfoArray[i].elementaryPid);
    }
    printf("\n********************PMT TABLE SECTION********************\n");
    
    return TABLES_PARSE_OK;
}
