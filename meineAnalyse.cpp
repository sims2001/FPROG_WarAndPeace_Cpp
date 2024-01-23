#define DOCTEST_CONFIG_IMPLEMENT
#include<iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <range/v3/all.hpp>
#include "doctest.h"

using namespace std;

auto readMyFileContent = [](string filename) -> vector<string> {
    ifstream file(filename);
    vector<string> lines;

    string line;
    while(getline(file, line))
        lines.push_back(line);

    return lines;
};

auto splitChapters = [](vector<string>& allLines) -> vector<vector<string>> {
    vector<vector<string>> allChapters;
    vector<string> currentChapter;
    
    bool inBook = false;
    bool splitLine;

    for(const auto& line: allLines) {

        splitLine = true;
        
        if( line.find("BOOK ") != string::npos ){
            inBook = true;
            splitLine = false;
        }

        if(inBook && splitLine) {
            
            if(line.find("CHAPTER ") != string::npos) {
                
                if( ! currentChapter.empty() ) {
                    allChapters.push_back(currentChapter);
                    currentChapter.clear();
                }

            } else {
                currentChapter.push_back(line);
            }

        }

    }

    if(!currentChapter.empty())
        allChapters.push_back(currentChapter);

    return allChapters;
};

auto tokenize = [](const string& line) -> vector<string> {
    regex myRegex(R"([\s\n,.!?\"\'-\(\)]+)");
    sregex_token_iterator it{ line.begin(), line.end(), myRegex, -1};
    sregex_token_iterator end;
    vector<string> tokenized{ it, end };

    //Remove empty tokens
    tokenized.erase(
        remove_if( tokenized.begin(), tokenized.end(), [](string const& s) {
            return s.size() == 0;
        }), 
        tokenized.end()
    );

    for(size_t i = 0; i < tokenized.size(); i++ ){
        for (auto &c : tokenized[i]){
            c = tolower(c);
        }
    }

    return tokenized;
};

auto tokenizeChapters = [](vector<vector<string>>& allChapters) -> vector<vector<string>> {
    vector<vector<string>> tokenizedChapters;

    for (const auto& currentChapter : allChapters) {
        vector<string> tokenizedChapter;
        for(const auto& line : currentChapter) {

            vector<string> words = tokenize( line );

            tokenizedChapter.insert(tokenizedChapter.end(), words.begin(), words.end());
        }

        tokenizedChapters.push_back(tokenizedChapter);
    }

    return tokenizedChapters;
};

auto filterTerms = [](const vector<string>& tokenizedList, const vector<string>& terms) -> vector<string> {
    vector<string> filteredList;

    set<string> termSet(terms.begin(), terms.end());

    copy_if(tokenizedList.begin(), tokenizedList.end(), back_inserter(filteredList),
        [&termSet](const string& token) {
            return termSet.find(token) != termSet.end();
        }
    );

    return filteredList;
};

auto countTerms = [](const vector<string>& theList) -> unordered_map<string, size_t> {
    unordered_map<string, size_t> wordCount;

    for(const auto& word : theList) {
        wordCount[word]++;
    }

    return wordCount;
};

auto calculateDensity = [](const size_t wordCount, const unordered_map<string, size_t>& wordOccurrences) -> double {
    double overallDensity = static_cast<double>( wordOccurrences.size() / wordCount );

    return overallDensity;
};

auto saveToFile = [](map<size_t, string> results) {
    string outputString = "";
    for(const auto& res : results) {
        outputString += "Chapter " + to_string(res.first) + ": " + res.second + "\n";
    }

    ofstream out("result.txt");
    out << outputString;
    out.close();

};

auto analyseWarPeaceFile = [](string fileToAnalyse, string peaceFile, string warFile) {
    vector<string> peaceTerms = readMyFileContent(peaceFile);
    vector<string> warTerms = readMyFileContent(warFile);
    vector<string> warAndPeace = readMyFileContent(fileToAnalyse);
    vector<vector<string>> theChapters = splitChapters( warAndPeace );
    vector<vector<string>> tokenizedChapters = tokenizeChapters( theChapters );
    tokenizedChapters.erase(
        remove_if(tokenizedChapters.begin(), tokenizedChapters.end(), [](const vector<string>& vec){
            return vec.empty();
        }), tokenizedChapters.end()
    );

    size_t i = 0;
    map<size_t, string> results;

    for(const auto& chapter : tokenizedChapters){
        i++;

        results[ i ] = "peace-related";

        vector<string> pT = filterTerms(chapter, peaceTerms);
        vector<string> wT = filterTerms(chapter, warTerms);

        if(pT.size() == wT.size() ){            
            unordered_map<string, size_t> countedPeaceTerms = countTerms(pT);
            double peaceTermDensity = calculateDensity(chapter.size(), countedPeaceTerms);

            unordered_map<string, size_t> countedWarTerms = countTerms(wT);
            double warTermDensity = calculateDensity(chapter.size(), countedWarTerms);

            if(peaceTermDensity != warTermDensity) {
                results[i] = peaceTermDensity > warTermDensity ? "peace-related" : "war-related";
            }
        } else {
            results[i] = pT.size() > wT.size() ? "peace-related" : "war-related";
        }
    }

    saveToFile(results);
};

int main(){

    analyseWarPeaceFile("war_and_peace.txt", "peace_terms.txt", "war_terms.txt");

    return 0;
}

TEST_CASE("Tokenizing test"){
    // Test input with various delimiters
    std::string input = "This is, \"a\" test! With some punctuation.";
    std::vector<std::string> expected = {"this", "is", "a", "test", "with", "some", "punctuation"};

    // Call the tokenize function
    std::vector<std::string> result = tokenize(input);

    // Check if the result matches the expected output
    CHECK(result == expected);
}
