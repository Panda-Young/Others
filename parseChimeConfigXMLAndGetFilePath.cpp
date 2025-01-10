/* **************************************************************
 * @Description: parse xml file and get file path by soundposition
 * @Date: 2024-05-30 00:59:59
 * @Version: 0.1.0
 * @Author: 1641140221@qq.com
 * @Copyright (c) 2024 by @AAC Technologies, All Rights Reserved.
 **************************************************************/
#include <libxml/parser.h>
#include <map>
#include <string>

#include "log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define XML_GET_INT_ATTR(_var_, _node_, _attr_, _opt_, _type_)                 \
    do {                                                                       \
        xmlChar *_p_ = xmlGetProp(_node_, (const xmlChar *)_attr_);            \
        if (_p_) {                                                             \
            _var_ = (_type_)strtoul((const char *)_p_, NULL, 0);               \
            xmlFree(_p_);                                                      \
        } else if (!_opt_) {                                                   \
            LOGE("chime parser: could not get attribute %s", _attr_);          \
        } else {                                                               \
            LOGE("chime parser: optional attribute %s not defined\n", _attr_); \
        }                                                                      \
    } while (0)

/*Get string attribute*/
#define XML_GET_STRING_ATTR(_var_, _node_, _attr_, _opt_)                      \
    do {                                                                       \
        xmlChar *_p_ = xmlGetProp(_node_, (const xmlChar *)_attr_);            \
        if (_p_) {                                                             \
            snprintf(_var_, sizeof(_var_), "%s", _p_);                         \
            xmlFree(_p_);                                                      \
        } else if (!_opt_) {                                                   \
            LOGE("chime parser: could not get attribute %s\n", _attr_);        \
        } else {                                                               \
            LOGE("chime parser: optional attribute %s not defined\n", _attr_); \
        }                                                                      \
    } while (0)

int32_t parseChimeConfigXMLAndGetFilePath(const char *xml_file_path, char *path, int soundposition)
{
    std::map<int32_t, std::string> mChimeFileMap;

    LOGE("%s Enter xml_file_path %s", __func__, xml_file_path);
    xmlDocPtr pXmlDoc = NULL;
    xmlNodePtr pCur = NULL;
    char file_dir[128] = {0};

    if (access(xml_file_path, F_OK)) {
        LOGE("can't access config file:[%s]", xml_file_path);
        return -1;
    }

    pXmlDoc = xmlParseFile(xml_file_path);
    if (pXmlDoc == NULL) {
        LOGE("parse file faild");
        return -1;
    }

    pCur = xmlDocGetRootElement(pXmlDoc);
    if (pCur == NULL) {
        LOGE("get root element faild");
        return -1;
    }

    pCur = pCur->xmlChildrenNode;
    while (pCur != NULL) {
        if (!(xmlStrcmp(pCur->name, (const xmlChar *)"Chime3DMapping"))) {
            int32_t pos_id = 0;
            char file_name[128] = {0};
            xmlNodePtr pCurChild = NULL;

            XML_GET_STRING_ATTR(file_dir, pCur, "path", 0);
            pCurChild = pCur->xmlChildrenNode;
            while (pCurChild != NULL) {
                if (!(xmlStrcmp(pCurChild->name, (const xmlChar *)"pos"))) {
                    XML_GET_INT_ATTR(pos_id, pCurChild, "id", 0, uint32_t);
                    XML_GET_STRING_ATTR(file_name, pCurChild, "fileName", 0);
                    mChimeFileMap.insert(std::pair<int, std::string>(pos_id, file_name));
                    LOGI("pos_id %d chime_name %s", pos_id, file_name);
                }
                pCurChild = pCurChild->next;
            }
        }
        pCur = pCur->next;
    }

    if (!mChimeFileMap.size()) {
        LOGE("%s, mChimeFileMap is null", __func__);
        return -1;
    }

    if (soundposition < 0 || soundposition >= (int)mChimeFileMap.size()) {
        LOGE("%s, soundposition %d out of bound", __func__, soundposition);
        return -1;
    }

    if (!strlen(mChimeFileMap[soundposition].c_str())) {
        LOGE("%s, mChimeFileMap[%d] is null in config", __func__, soundposition);
        return -1;
    }
    strcat(path, file_dir);
    strcat(path, mChimeFileMap[soundposition].c_str());
    LOGE("%s, %s", __func__, path);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        LOGE("Usage: %s <xml_file_path> <soundposition>", argv[0]);
        return -1;
    }

    char path[256] = {0};
    if (parseChimeConfigXMLAndGetFilePath(argv[1], path, atoi(argv[2])) == 0) {
        LOGI("path: %s", path);
    } else {
        LOGE("parseChimeConfigXMLAndGetFilePath failed");
    }

    return 0;
}
