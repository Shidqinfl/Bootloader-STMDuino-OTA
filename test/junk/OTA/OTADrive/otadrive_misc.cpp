#include "otadrive_esp.h"
#include "MD5/MD5Builder.h"

String otadrive_ota::cutLine(String &str)
{
    String l = "";
    for (uint32_t i = 0; i < str.length(); i++)
    {
        char c = str[i];
        if (c == '\n')
        {
            str.remove(0, l.length() + 1);
            return l;
        }
        l += c;
    }

    str.remove(0, l.length());

    return l;
}

String otadrive_ota::file_md5(File &f)
{
    MD5_CTX *ctx;
    if (!f)
    {
        return String();
    }

    if (f.seek(0, SeekSet))
    {
        MD5Builder md5;
        // md5.begin();
        // md5.addStream(f, f.size());
        // md5.calculate();

        md5.MD5Init(ctx);
        // md5.MD5Update(MD5_CTX,)

        // return md5.toString();
    }
    return String();
}