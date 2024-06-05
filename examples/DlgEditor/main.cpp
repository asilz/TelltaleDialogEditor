#include <gui.hpp>
extern "C"
{
#include <stream.h>
}

int Main(int argc, char **argv)
{
    int ret;
    if (argc < 2)
    {
        printf("Too few arguments\n");
        return -1;
    }
    FILE *inputStream = cfopen(argv[1], "rb");
    if (inputStream == NULL)
    {
        printf("invalid input path\n");
        return -2;
    }
    DlgApplication exampe("Blueprints", inputStream);

    if (exampe.Create())
        ret = exampe.Run();

    if (argc == 3)
    {
        FILE *outputStream = cfopen(argv[2], "wb");
        printf("argc %d\n", argc);
        writeMetaStreamHeader(outputStream, &(exampe.header));
        writeTree(outputStream, &(exampe.dlg));
        fclose(outputStream);
    }

    return ret;

    return 0;
}