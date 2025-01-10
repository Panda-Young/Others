/***************************************************************************
 * Description: xml test
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2024-06-06 23:18:12
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#ifdef __MINGW32__
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(doc, root_node);
    xmlNewChild(root_node, NULL, BAD_CAST "node1", BAD_CAST "content of node1");
    node = xmlNewChild(root_node, NULL, BAD_CAST "node3", BAD_CAST "node has attributes");
    xmlNewProp(node, BAD_CAST "attribute", BAD_CAST "yes");
    node = xmlNewNode(NULL, BAD_CAST "node4");
    node1 = xmlNewText(BAD_CAST "other way to create content");
    xmlAddChild(node, node1);
    xmlAddChild(root_node, node);
    xmlSaveFormatFileEnc(argc > 1 ? argv[1] : "-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();
    return (0);
}

/* Compilation command:
    gcc xml_test.c -I "C:\Program Files (x86)\libxml2\include\libxml2" ^
    -L"C:\Program Files (x86)\libxml2\bin" -lxml2 -o xml_test && xml_test
*/
#else
#include <stdio.h>
int main()
{
    printf("This program is only for Windows platform.\n");
    return 0;
}
#endif
