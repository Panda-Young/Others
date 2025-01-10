#ifndef __ChimeConfigXMLParser_H
#define __ChimeConfigXMLParser_H

#include "log.h"
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <map>
#include <stdint.h>
#include <string>
#include <stdio.h>
#include <unistd.h>

int32_t parseChimeConfigXML(const char *xml_file_path);
char *get_3dc_file_Path(int soundposition);

#endif // __ChimeConfigXMLParser_H
