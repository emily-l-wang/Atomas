#include "ui.hxx"
#include <stdexcept>

using namespace std;

int main(int argc, char* argv[])
{
    // run it with 0 args to get the normal version, 1 arg to cheat
    try {
        bool cheat;
        switch (argc) {
        case 1:
            cheat = false;
            break;
        case 2:
            cheat = true;
            break;
        default:
            cerr << "Usage: " << argv[0] << "not a valid number of "
                                            "parameters\n";
            return 1;
        }
        Model m(cheat);
        Ui ui(m);
        ui.run();
    }

    catch (exception const& e) {
        cerr << argv[0] << ": " << e.what() << "\n";
        return 1;
    }

}
