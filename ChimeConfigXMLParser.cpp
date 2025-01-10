#include "ChimeConfigXMLParser.h"

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

std::map<int32_t, std::string> mChimeFileMap;

int32_t parse_chime_config_info(xmlNodePtr parent)
{
    int32_t pos_id = 0;
    int32_t file_number = 0;
    char file_name[128] = {0};
    xmlNodePtr pCur = NULL;

    XML_GET_INT_ATTR(file_number, parent, "number", 0, int32_t);
    pCur = parent->xmlChildrenNode;
    while (pCur != NULL) {
        if (!(xmlStrcmp(pCur->name, (const xmlChar *)"pos"))) {
            XML_GET_INT_ATTR(pos_id, pCur, "id", 0, uint32_t);
            XML_GET_STRING_ATTR(file_name, pCur, "fileName", 0);
            mChimeFileMap.insert(std::pair<int, std::string>(pos_id, file_name));
            LOGI("pos_id %d chime_name %s", pos_id, file_name);
        }
        pCur = pCur->next;
    }

    for (int i = 0; i < static_cast<int>(mChimeFileMap.size()); i++) {
        std::string name = static_cast<std::string>(mChimeFileMap[static_cast<int>(i)]);
        char name_temp[128] = {0};
        strcpy(name_temp, name.c_str());
        LOGE("mChimeFileMap[%d] name %s", i, name_temp);
    }

    LOGE("%s Exit file_number %d", __func__, file_number);
    return 0;
}

int32_t parseChimeConfigXML(const char *xml_file_path)
{
    LOGE("%s Enter xml_file_path %s", __func__, xml_file_path);
    xmlDocPtr pXmlDoc = NULL;
    xmlNodePtr pCur = NULL;

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
            if (parse_chime_config_info(pCur) != 0) {
                LOGE("parse chime config faild %s", __func__);
            }
        }
        pCur = pCur->next;
    }
    LOGE("%s Exit", __func__);
    return 0;
}

char *get_3dc_file_Path(int soundposition)
{
    LOGE("%s Enter", __func__);
    char *path = static_cast<char *>(calloc(1, 128));
    if (!mChimeFileMap.size()) {
        LOGE("%s, mChimeFileMap is null", __func__);
        free(path);
        return NULL;
    }

    if (soundposition < 0 || soundposition >= (int)mChimeFileMap.size()) {
        LOGE("%s, soundposition %d out of bound", __func__, soundposition);
        free(path);
        return NULL;
    }

    if (!strlen(mChimeFileMap[soundposition].c_str())) {
        LOGE("%s, mChimeFileMap[%d] is null in config", __func__, soundposition);
        free(path);
        return NULL;
    }
    strcat(path, "/mnt/nio/etc/chime3d/");
    strcat(path, mChimeFileMap[soundposition].c_str());
    LOGE("%s, %s", __func__, path);
    return path;
}
