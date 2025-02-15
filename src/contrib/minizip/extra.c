/* Written by Paul Edwards. Released to the public domain. */

#include <ctype.h>

int stricmp(const char *s1, const char *s2)
{
    const unsigned char *p1;
    const unsigned char *p2;
    
    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    while (*p1 != '\0')
    {
        if (toupper(*p1) < toupper(*p2)) return (-1);
        else if (toupper(*p1) > toupper(*p2)) return (1);
        p1++;
        p2++;
    }
    if (*p2 == '\0') return (0);
    else return (-1);
}
