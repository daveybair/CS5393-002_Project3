#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cstring>

using namespace std;

// String Class
class DSString {
private:
    char* data; // Pointer holds the string data
    int len;    // Length of the string

public:
    // Constructors and Destructor
    DSString() : data(nullptr), len(0) {} // Default constructor
    DSString(const char* str);            // Constructor with c-string input
    DSString(const DSString& other);      // copy constructor
    ~DSString();                          // destructor

    // overloaded operators
    DSString& operator=(const DSString& other); 
    bool operator==(const DSString& other) const; // Equality check
    bool operator<(const DSString& other) const;  // Less-than comparison
    friend ostream& operator<<(ostream& os, const DSString& str); // output-stream

    // Function to return c-style string
    const char* c_str() const { return data; }
};

// DSString

// Constructor: Initializes from a c-string
DSString::DSString(const char* str) {
    len = strlen(str);
    data = new char[len + 1];
    strcpy(data, str);
}

// Copy Constructor
DSString::DSString(const DSString& other) {
    len = other.len;
    data = new char[len + 1];
    strcpy(data, other.data);
}

// Destructor
DSString::~DSString() {
    delete[] data;
}

// copying
DSString& DSString::operator=(const DSString& other) {
    if (this != &other) {
        delete[] data; //delete through/old data
        len = other.len;
        data = new char[len + 1];
        strcpy(data, other.data);
    }
    return *this;
}

// = Operator
bool DSString::operator==(const DSString& other) const {
    return strcmp(data, other.data) == 0;
}

// < Operator 
bool DSString::operator<(const DSString& other) const {
    return strcmp(data, other.data) < 0;
}

// Output Stream Operator     pringint DSString
ostream& operator<<(ostream& os, const DSString& str) {
    os << str.data;
    return os;
}

// token function: seperate words, removes punctuation, & converts to lowercase
vector<DSString> tokenize(const DSString& tweet) {
    vector<DSString> tokens;
    stringstream ss(tweet.c_str());
    string word;

    while (ss >> word) { // seperate each word
        string cleanedWord;
        for (char c : word) {
            // to lovercase and only letters
            if (isalnum(c)) cleanedWord += tolower(c);
        }
        if (!cleanedWord.empty()) {
            tokens.push_back(DSString(cleanedWord.c_str()));
        }
    }
    return tokens;
}

// Sentiment Classifier
class SentimentClassifier {
private:
    // count positive and negative words 
    map<DSString, int> positiveWords;
    map<DSString, int> negativeWords;
    int positiveCount = 0, negativeCount = 0;

public:
    void train(const char* trainingFile); // use dataset
    void predict(const char* testFile, const char* resultFile); // Predict sentiment for test data
    void evaluatePredictions(const char* groundTruthFile, const char* accuracyFile); // calculating accuracy
};

// Train Function: Reads training data and builds word maps
void SentimentClassifier::train(const char* trainingFile) {
    ifstream file(trainingFile);
    if (!file.is_open()) {
        cerr << "Error opening training file.\n";
        return;
    }

    string line;
    getline(file, line); // Skip the first line
    while (getline(file, line)) {
        stringstream ss(line);
        string sentimentStr, tweetText;
        getline(ss, sentimentStr, ',');
        for (int i = 0; i < 4; ++i) ss.ignore(256, ','); 
        getline(ss, tweetText);

        // Convert sentiment from string to integer
        int sentiment;
        try {
            sentiment = stoi(sentimentStr);
        } catch (...) {
            cerr << "Error parsing sentiment: " << sentimentStr << endl;
            continue;
        }

        DSString tweet(tweetText.c_str());
        vector<DSString> words = tokenize(tweet);

        // Count words based on sentiment (4 = positive, 0 = negative)
        for (const auto& word : words) {
            if (sentiment == 4) {
                positiveWords[word]++;
                positiveCount++;
            } else if (sentiment == 0) {
                negativeWords[word]++;
                negativeCount++;
            }
        }
    }
    file.close();
    cout << "Training completed. Positive words: " << positiveCount 
         << ", Negative words: " << negativeCount << endl;
}

// Predict Function: Predicts sentiment for each tweet
void SentimentClassifier::predict(const char* testFile, const char* resultFile) {
    ifstream file(testFile);
    ofstream results(resultFile);
    if (!file.is_open() || !results.is_open()) {
        cerr << "Error opening test or results file.\n";
        return;
    }

    string line;
    int tweetCount = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        string id, tweetText;
        getline(ss, id, ',');
        for (int i = 0; i < 4; ++i) ss.ignore(256, ',');
        getline(ss, tweetText);

        DSString tweet(tweetText.c_str());
        vector<DSString> words = tokenize(tweet);

        // Calculate scores based on word counts
        double posScore = 0.0, negScore = 0.0;
        for (const auto& word : words) {
            posScore += positiveWords[word];
            negScore += negativeWords[word];
        }

        // Predict sentiment based on scores
        int predictedSentiment = (posScore > negScore) ? 4 : 0;
        results << predictedSentiment << ", " << id << "\n";
        tweetCount++;
    }
    file.close();
    results.close();
    cout << "Prediction completed for " << tweetCount << " tweets." << endl;
}

// Evaluate Predictions Function: to calc accuracy
void SentimentClassifier::evaluatePredictions(const char* groundTruthFile, const char* accuracyFile) {
    ifstream truthFile(groundTruthFile);
    ifstream resultFile("results.csv");
    ofstream accuracyOut(accuracyFile);

    if (!truthFile.is_open() || !resultFile.is_open()) {
        cerr << "Error opening ground truth or results file.\n";
        return;
    }

    int total = 0, correct = 0;
    string truthLine, resultLine;

    getline(truthFile, truthLine); 
    getline(resultFile, resultLine); 

    while (getline(truthFile, truthLine) && getline(resultFile, resultLine)) {
        int actualSentiment, predictedSentiment;

        try {
            actualSentiment = stoi(truthLine.substr(0, truthLine.find(',')));
            predictedSentiment = stoi(resultLine.substr(0, resultLine.find(',')));
        } catch (const invalid_argument&) {
            cerr << "Invalid line in ground truth or results.\n";
            continue;
        }

        if (actualSentiment == predictedSentiment) correct++;
        total++;
    }

    // Calculate and write accuracy
    double accuracy = (total > 0) ? static_cast<double>(correct) / total : 0.0;
    accuracyOut << accuracy << "\n";
    cout << "Evaluation completed. Accuracy: " << accuracy << endl;

    truthFile.close();
    resultFile.close();
    accuracyOut.close();
}

// Main Function: Handles command-line arguments and runs the program
int main(int argc, char* argv[]) {
    if (argc != 6) {
        cerr << "Usage: ./sentiment <trainFile> <testFile> <groundTruthFile> <resultsFile> <accuracyFile>\n";
        return 1;
    }

    SentimentClassifier classifier;
    classifier.train(argv[1]);
    classifier.predict(argv[2], argv[4]);
    classifier.evaluatePredictions(argv[3], argv[5]);

    return 0;
}
