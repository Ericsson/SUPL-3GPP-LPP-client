#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#include <args.hpp>
#pragma GCC diagnostic pop

#include <generator/tokoro/coordinate.hpp>
#include <loglet/loglet.hpp>
#include <maths/float3.hpp>

using namespace generator::tokoro;

#if 0
struct Arguments {
    Itrf source_system;
    Itrf dest_system;
    double epoch;
    std::vector<Float3> position;
};

std::vector<Float3> parse_positions(const std::vector<double>& positions) {
    std::vector<Float3> result;
    for (size_t i = 0; i < positions.size(); i += 3) {
        result.push_back({positions[i], positions[i + 1], positions[i + 2]});
    }
    return result;
}

Arguments parse_arguments(int argc, char** argv) {
    args::ArgumentParser parser("Coordinate conversion example");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> source_system(parser, "source", "Source coordinate system", {'s', "source"});
    args::ValueFlag<std::string> dest_system(parser, "dest", "Destination coordinate system", {'d', "dest"});
    args::ValueFlag<double> epoch(parser, "epoch", "Epoch time", {'e', "epoch"});
    args::Flag position_in_llh(parser, "llh", "Position is in LLH format", {'l', "llh"});
    args::PositionalList<double> position(parser, "position", "Position to convert");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help&) {
        std::cout << parser;
        exit(0);
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    } catch (args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    }

    auto source = args::get(source_system);
    auto dest = args::get(dest_system);

    Itrf source_itrf;
    if(source == "itrf2020") source_itrf = Itrf::ITRF2020;
    else if(source == "itrf2014") source_itrf = Itrf::ITRF2014;
    else if(source == "itrf2008") source_itrf = Itrf::ITRF2008;
    else if(source == "itrf2005") source_itrf = Itrf::ITRF2005;
    else if(source == "itrf2000") source_itrf = Itrf::ITRF2000;
    else if(source == "itrf1997") source_itrf = Itrf::ITRF1997;
    else if(source == "itrf1996") source_itrf = Itrf::ITRF1996;
    else if(source == "itrf1994") source_itrf = Itrf::ITRF1994;
    else if(source == "itrf1993") source_itrf = Itrf::ITRF1993;
    else if(source == "itrf1992") source_itrf = Itrf::ITRF1992;
    else if(source == "itrf1991") source_itrf = Itrf::ITRF1991;
    else if(source == "itrf1990") source_itrf = Itrf::ITRF1990;
    else if(source == "itrf1989") source_itrf = Itrf::ITRF1989;
    else if(source == "itrf1988") source_itrf = Itrf::ITRF1988;
    else {
        std::cerr << "Invalid source coordinate system" << std::endl;
        exit(1);
    }

    Itrf dest_itrf;
    if(dest == "itrf2020") dest_itrf = Itrf::ITRF2020;
    else if(dest == "itrf2014") dest_itrf = Itrf::ITRF2014;
    else if(dest == "itrf2008") dest_itrf = Itrf::ITRF2008;
    else if(dest == "itrf2005") dest_itrf = Itrf::ITRF2005;
    else if(dest == "itrf2000") dest_itrf = Itrf::ITRF2000;
    else if(dest == "itrf1997") dest_itrf = Itrf::ITRF1997;
    else if(dest == "itrf1996") dest_itrf = Itrf::ITRF1996;
    else if(dest == "itrf1994") dest_itrf = Itrf::ITRF1994;
    else if(dest == "itrf1993") dest_itrf = Itrf::ITRF1993;
    else if(dest == "itrf1992") dest_itrf = Itrf::ITRF1992;
    else if(dest == "itrf1991") dest_itrf = Itrf::ITRF1991;
    else if(dest == "itrf1990") dest_itrf = Itrf::ITRF1990;
    else if(dest == "itrf1989") dest_itrf = Itrf::ITRF1989;
    else if(dest == "itrf1988") dest_itrf = Itrf::ITRF1988;
    else {
        std::cerr << "Invalid destination coordinate system" << std::endl;
        exit(1);
    }

    auto raw_positions = args::get(position);

    

    return Arguments{source_itrf, dest_itrf, args::get(epoch), parse_positions(raw_positions)};
}
#endif

int main(int /*argc*/, char** /*argv*/) {
    loglet::set_level(loglet::Level::Verbose);

    double epoch = 2024.0;

    Float3 etrf89 = {
        3227560.90670000016689,
        898383.43410000007134,
        5409177.11160000041127,
    };

    auto itrf89 = etrf89_to_itrf89(epoch, etrf89);
    auto itrf20 = itrf_transform(Itrf::ITRF1989, Itrf::ITRF2020, epoch, itrf89);

    printf("ETRF89   %+24.4f, %+24.4f, %+24.4f\n", etrf89.x, etrf89.y, etrf89.z);
    printf("ITRF89   %+24.4f, %+24.4f, %+24.4f\n", itrf89.x, itrf89.y, itrf89.z);
    printf("ITRF2020 %+24.4f, %+24.4f, %+24.4f\n", itrf20.x, itrf20.y, itrf20.z);

    return 0;
}
