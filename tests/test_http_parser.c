#include <stdio.h>
#include "../http/parser.h"

int main(void)
{
    const char* request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Http_Request* parsed = Http_Parser_Parse(request);

    /* Parser should not fail */
    if (parsed == NULL)
        return 1;

    /* Method should be GET */
    if (parsed->start_line.method != GET)
        return 1;

    /* Method string should be correct */
    if (Http_Request_Get_Method_String(parsed)[0] == '\0')
        return 1;

    Http_Parser_Cleanup(parsed);

    printf("HTTP parser test passed\n");
    return 0;
}
