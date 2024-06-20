#include <gui.hpp>
extern "C"
{
#include <stream.h>
}

int Main(int argc, char **argv)
{
    int ret = 0;
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
    uint32_t extensionIndex = 0;
    while (1)
    {
        if (argv[1][extensionIndex++] == '.')
        {
            break;
        }
    }

    if (strcmp(argv[1] + extensionIndex, "dlog") == 0)
    {
        DlgApplication exampe("Blueprints", inputStream);

        if (exampe.Create())
            ret = exampe.Run();

        if (argc == 3)
        {
            FILE *outputStream = cfopen(argv[2], "wb");
            if (outputStream == NULL)
            {
                treeFree(&exampe.dlg);
                printf("invalid output path\n");
                return -2;
            }
            printf("argc %d\n", argc);
            writeMetaStreamHeader(outputStream, &(exampe.header));
            writeTree(outputStream, &(exampe.dlg));
            fclose(outputStream);
        }

        treeFree(&exampe.dlg);

        return ret;
    }
    else
    {
        GeneralApplication exampe("Blueprints", argv[1] + extensionIndex, inputStream);

        if (exampe.Create())
            ret = exampe.Run();

        if (argc == 3)
        {
            FILE *outputStream = cfopen(argv[2], "wb");
            if (outputStream == NULL)
            {
                treeFree(&exampe.data);
                printf("invalid output path\n");
                return -2;
            }
            printf("argc %d\n", argc);
            writeMetaStreamHeader(outputStream, &(exampe.header));
            writeTree(outputStream, &(exampe.data));
            fclose(outputStream);
        }

        treeFree(&exampe.data);

        return ret;
    }
}