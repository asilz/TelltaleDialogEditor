#include "include/widgets.hpp"

int Main(int argc, char **argv)
{
    DlgApplication exampe("Widgets", argc, argv);

    if (exampe.Create())
        return exampe.Run();

    return 0;
}