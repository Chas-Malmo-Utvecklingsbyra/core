
#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

/* (PR) The stuff we created last wednesday(?), start of basic http parser */

int http_parse(const char* _InBuffer, size_t _InBufferSize, char* _OutBuffer);

/* NYI */
int http_parseBody();

/* NYI */
int http_parseHeader();


#endif /* __HTTP_PARSER_H__ */