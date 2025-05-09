#include <OpenXLSX.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

string normalize(const string &line)
{
    string result;
    bool first_esc = false;
    string::size_type last_esc = string::npos;
    if (line.find("Date/Time") != string::npos){
        cout << line << endl;
    }
    for (auto it = line.begin(); it != line.end(); it++)
    {
        if (static_cast<unsigned char>(*it) <= 0x7f && *it != ';')
        {
            if (*it == '\"')
            {
                if (!first_esc)
                {
                    first_esc = true;
                    continue;
                }
                result.push_back(*it);
                last_esc = result.size() - 1;
            }
            else
            {
                if(first_esc && *it == ','){
                    continue;
                }
                result.push_back(*it);
            }
        }
    }
    if (last_esc != string::npos)
    {
        result.erase(last_esc, 1);
    }
    boost::algorithm::replace_all(result, "\"\"", "\"");
    //boost::algorithm::erase_all(result, ",");

//    cout << "line:" << result << endl;
    return result;
}

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "<help message>")
        ("input-file,i", po::value<string>(), "input file")
        ("output-file,o", po::value<string>(), "output file")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("input-file") != 1){
        cerr << "input-file is not specified." << endl;
    }

    ifstream infile(vm["input-file"].as<string>());
    if (infile.fail())
    {
        cerr << "Input file cannot be opened." << endl;
        return -1;
    }
    typedef boost::tokenizer<boost::escaped_list_separator<char> , std::string::const_iterator, std::string> Tokenizer;
    
    OpenXLSX::XLDocument doc;
    switch(vm.count("output-file")){
        case 0:
            {
                string output_file = vm["input-file"].as<string>();
                auto last_separator_pos = output_file.find_last_of(".");
                if(last_separator_pos != string::npos){
                    output_file.erase(last_separator_pos);
                }
                output_file.append(".xlsx");
                cout << "Output file: " << output_file << endl;
                doc.create(output_file, true);
            }
            break;
        case 1:
            doc.create(vm["output-file"].as<string>(), true);
            break;
        default:
            cerr << "More than one output-file parameter" << endl;
            return -1;
    }

    auto workbook = doc.workbook();

    string line;
    while (getline(infile, line))
    {
        // string normalized_line = normalize(line);
        Tokenizer tok(line, boost::escaped_list_separator<char>("", ",", "\""));
        if (tok.begin() != tok.end())
        {
            auto worksheet_name = *(tok.begin());
            if (!workbook.worksheetExists(worksheet_name))
            {
                workbook.addWorksheet(worksheet_name);
                cout << "adding new worksheet: " << worksheet_name << endl;
            }
            auto wks = workbook.worksheet(worksheet_name);
            unsigned rowNumber = wks.rowCount() + 1;
            unsigned cellNumber = 1;
            auto first = tok.begin().operator++();
            for (auto i = first; i != tok.end(); i++, cellNumber++)
            {
                string value = *i;
                auto cell = wks.cell(rowNumber, cellNumber);
                cell.value() = value;
                try{
                    string val_no_spc = boost::algorithm::replace_all_copy(value, " ", "");
                    if (val_no_spc.find_first_not_of("0123456789-.") != string::npos)
                    {
                        throw std::invalid_argument("");
                    }
                    double valued = stod(value);
                    cell.value() = valued;
                }
                catch(std::invalid_argument& ex)
                {
                    cell.value() = value;
                }
                catch(std::out_of_range& ex)
                {
                    cerr << "out of range value: " << value;
                }
                // cout << wks.cell(rowNumber, cellNumber).value() << " | ";
            }
            // cout << endl;
        }
        else
        {
            cout << "Skipping empty line:  " << line << endl;
        }
    }
    doc.save();

    return 0;
}
