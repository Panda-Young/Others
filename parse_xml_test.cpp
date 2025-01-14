/*
 * @Description: parse xml
 * @Date: 2024-04-28 15:26:20
 * @Version: 0.1.0
 * @Author: Panda-Young
 * Copyright (c) 2024 by @Panda-Young, All Rights Reserved.
 */
#include <libxml/parser.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <map>
#include <string>
#include "log.h"

#define XML_GET_INT_ATTR(_var_, _node_, _attr_, _opt_, _type_)                        \
    do {                                                                              \
        xmlChar *_p_ = xmlGetProp(_node_, (const xmlChar *)_attr_);                   \
        if (_p_) {                                                                    \
            _var_ = (_type_)strtoul((const char *)_p_, NULL, 0);                      \
            xmlFree(_p_);                                                             \
        } else if (!_opt_) {                                                          \
            LOGE("sound effect parser: could not get attribute %s", _attr_);          \
        } else {                                                                      \
            LOGE("sound effect parser: optional attribute %s not defined\n", _attr_); \
        }                                                                             \
    } while (0)

#define XML_GET_STRING_ATTR(_var_, _node_, _attr_, _opt_)                             \
    do {                                                                              \
        xmlChar *_p_ = xmlGetProp(_node_, (const xmlChar *)_attr_);                   \
        if (_p_) {                                                                    \
            snprintf(_var_, sizeof(_var_), "%s", _p_);                                \
            xmlFree(_p_);                                                             \
        } else if (!_opt_) {                                                          \
            LOGE("sound effect parser: could not get attribute %s\n", _attr_);        \
        } else {                                                                      \
            LOGE("sound effect parser: optional attribute %s not defined\n", _attr_); \
        }                                                                             \
    } while (0)

typedef enum {
    E_FAILED = -1,
    E_OK = 0
} ERROR_CODE;

#define LOGALL LOGI

std::map<int32_t, std::string> m3DChimeFileMap;

int32_t parse_3DChime_config_info(xmlNodePtr parent)
{
    int32_t pos_id = 0;
    int32_t file_number = 0;
    char file_name[128] = {0};
    xmlNodePtr pCur = NULL;

    XML_GET_INT_ATTR(file_number, parent, "number", 0, int32_t);
    pCur = parent->xmlChildrenNode;
    while (pCur != NULL) {
        if (!(xmlStrcmp(pCur->name, (const xmlChar*) "pos"))) {
            XML_GET_INT_ATTR(pos_id, pCur, "id", 0, uint32_t);
            XML_GET_STRING_ATTR(file_name, pCur, "fileName", 0);
            m3DChimeFileMap.insert(std::pair<int, std::string>(pos_id, file_name));
            LOGI("pos_id %d 3dc_file_name %s\n", pos_id, file_name);
        }
        pCur = pCur->next;
    }

    LOGI("%s Exit. file_number %d\n", __func__, file_number);
    return E_OK;
}

int32_t parse3DChimeConfig(const char *config_file_path)
{
    LOGI("%s Enter config_file_path %s\n", __func__, config_file_path);
    int32_t rc = 0;
    xmlDocPtr pXmlDoc = NULL;
    xmlNodePtr pCur = NULL;

    if (access(config_file_path, F_OK)) {
        LOGE("can't access config file:[%s]\n", config_file_path);
        return E_FAILED;
    }

    pXmlDoc = xmlParseFile(config_file_path);
    if (pXmlDoc == NULL) {
        LOGE("parse file faild\n");
        return E_FAILED;
    }

    pCur = xmlDocGetRootElement(pXmlDoc);
    if (pCur == NULL) {
        LOGE("get root element faild\n");
        return E_FAILED;
    } else {
        LOGI("root name %s\n", pCur->name);
    }

    pCur = pCur->xmlChildrenNode;
    while (pCur != NULL && !rc) {
        if (!(xmlStrcmp(pCur->name, (const xmlChar *)"Chime3DMapping"))) {
            rc = parse_3DChime_config_info(pCur);
            if (rc) {
                LOGE("parse sound effect config faild %s\n", __func__);
            }
        } else {
            LOGE("unknown tag %s\n", pCur->name);
        }
        pCur = pCur->next;
    }
    LOGI("%s Exit\n", __func__);
    return E_OK;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        LOGE("please input xml path.\n");
        return E_FAILED;
    }
    parse3DChimeConfig(argv[1]);
}

/* compile command
g++ parse_xml_test.cpp ^
-I "D:\WorkSpace\XML\libxml2\libxml2-v2.12.4\include" ^
-I "D:\WorkSpace\XML\libxml2\libxml2-v2.12.4-build" ^
-L "D:\WorkSpace\XML\libxml2\libxml2-v2.12.4-build" ^
-lxml2 -lstdc++ ^
-o parse_xml_test && ^
parse_xml_test.exe D:\WorkSpace\ALPS\3D-Chime\chime3D.xml
*/
