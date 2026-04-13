#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

const string VERSION = "1.0";

struct Section {
    string section_title;
    string text;
    string reference;
    string kjv_text;
    string bsb_text;
};

struct Tract {
    string name;
    vector<Section> sections;
};

// Romans Road sections
vector<Section> romansRoadSections = {
    {"What is The Romans Road?", "\nSimply put, the Romans Road shows the path to Heaven. It uses points from the book of Romans in the Bible to explain God’s gift of salvation. At each stop we learn something new about why we need salvation, how God has provided for it, and how we receive it.\n"
"\nAs you look at each of the six points outlined here, take time to reflect on what God’s Word has to say. Remember that the book of Romans was written to people just like you and me: people living in a busy culture, trying to make sense of what is true and what is most important in life.\n"
"\nThe Romans Road describes the only way to Heaven. Study it, learn it, and follow it!", "", "", ""},
    {"1. Everyone Needs Salvation", "", "Romans 3:23", "For all have sinned, and come short of the glory of God;", "for all have sinned and fall short of the glory of God,"},
    {"2. Jesus Died For Our Salvation", "", "Romans 5:8", "But God commendeth his love toward us, in that, while we were yet sinners, Christ died for us.", "But God proves His love for us in this: While we were still sinners, Christ died for us."},
    {"3. Salvation Is A Gift", "", "Romans 6:23", "For the wages of sin is death; but the gift of God is eternal life through Jesus Christ our Lord.", "For the wages of sin is death, but the gift of God is eternal life in Christ Jesus our Lord."},
    {"4. We Are Saved By Grace", "", "Romans 11:6", "And if by grace, then is it no more of works: otherwise grace is no more grace.", "And if it is by grace, then it is no longer by works. Otherwise, grace would no longer be grace."},
    {"5. Salvation Comes Through Faith", "", "Romans 4:5", "But to him that worketh not, but believeth on him that justifieth the ungodly, his faith is counted for righteousness.", "However, to the one who does not work, but believes in Him who justifies the ungodly, his faith is credited as righteousness."},
    {"6. God Saves All Who Call Upon Him", "", "Romans 10:13", "For whosoever shall call upon the name of the Lord shall be saved.", "for, ""Everyone who calls on the name of the Lord will be saved."""},
    {"","Follow the Romans Road to salvation today. Recognize that you are a sinner and that your sin must be judged by God. See that Jesus died to pay the penalty for your sin, but that you must choose to accept His provision. Understand that you cannot earn your way to Heaven through good works or religious activity. Now turn to God and put your faith in Jesus Christ who died for you and rose again.\n","","",""},
    {"","","Romans 10:9-10","For if you confess with your mouth the Lord Jesus and believe in your heart that God has raised Him from the dead, you will be saved. For with the heart man believeth unto righteousness; and with the mouth confession is made unto salvation", "that if you confess with your mouth, ""Jesus is Lord,"" and believe in your heart that God raised Him from the dead, you will be saved. For with your heart you believe and are justified, and with your mouth you confess and are saved. "}
};
// Available tracts
map<string, Tract> availableTracts = {
    {"The Romans Road", {"The Romans Road", romansRoadSections}}
};

void printHelp() {
    cout << "gospel v" << VERSION << endl;
    cout << "\nUsage: gospel [OPTIONS]" << endl;
    cout << "\nOptions:" << endl;
    cout << "  -h, --help              Show this help message and exit" << endl;
    cout << "  -v, --version           Show version information and exit" << endl;
    cout << "  -bv=VERSION             Set Bible version (default: KJV)" << endl;
    cout << "  --bibleversion=VERSION    Specify Bible version (KJV, BSB)" << endl;
    cout << "  -tn=NAME                Set tract name (default: 'The Romans Road')" << endl;
    cout << "  --tractname=NAME        Specify tract presentation by name" << endl;
    cout << "\nExamples:" << endl;
    cout << "  gospel                              Display default tract in KJV" << endl;
    cout << "  gospel -bv=BSB                      Display default tract in BSB" << endl;
    cout << "  gospel -tn=\"The Romans Road\" -bv=BSB  Display Romans Road in BSB" << endl;
}
int main(int argc, char* argv[]) {
    string version = "KJV";
    string tractName = "The Romans Road"; // Default tract
    
    // Parse command-line arguments
    for(int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            cout << "gospel v" << VERSION << endl;
            return 0;
        } else if (arg.find("-bv=") == 0) {
            size_t eq = arg.find('=');
            if (eq != string::npos) {
                version = arg.substr(eq + 1);
            }
        } else if (arg.find("--bibleversion=") == 0) {
            size_t eq = arg.find('=');
            if (eq != string::npos) {
                version = arg.substr(eq + 1);
            }
        } else if (arg.find("-tn=") == 0) {
            size_t eq = arg.find('=');
            if (eq != string::npos) {
                tractName = arg.substr(eq + 1);
            }
        } else if (arg.find("--tractname=") == 0) {
            size_t eq = arg.find('=');
            if (eq != string::npos) {
                tractName = arg.substr(eq + 1);
            }
        }
    }
    
    // Check if tract exists
    if (availableTracts.find(tractName) == availableTracts.end()) {
        cerr << "Error: Tract '" << tractName << "' not found." << endl;
        cerr << "Available tracts:" << endl;
        for (const auto& tract : availableTracts) {
            cerr << "  - " << tract.first << endl;
        }
        return 1;
    }
    
    // Get the selected tract
    const Tract& selectedTract = availableTracts[tractName];
    
    // Convert to upper case for comparison
    transform(version.begin(), version.end(), version.begin(), ::toupper);
    for (const auto& v : selectedTract.sections) {
        if (v.section_title.length() > 0) {
            cout << v.section_title << endl;
        }
        if (v.text.length() > 0) {
            cout << v.text << endl;
        }
        if (v.reference.length() > 0) {
            cout << v.reference << " (" << (version == "BSB" ? "BSB" : "KJV") << "): ";
            if (version == "BSB") {
                cout << v.bsb_text;
            } else {
                cout << v.kjv_text;
            }
            cout << endl << endl;
        }
    }
    return 0;
}