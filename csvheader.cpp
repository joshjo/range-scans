#include "src/includes.h"
#include "src/tools.h"

using namespace std;

int main(int argc, char ** argv) {
    gflags::SetUsageMessage("CSV Header");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    CSVresults h;
    h.printHeader();

    gflags::ShutDownCommandLineFlags();

    return 0;
}
