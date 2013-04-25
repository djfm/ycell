#ifndef CSV_H
#define CSV_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <set>
#include <map>

class CSV
{
public:

    class ParseError : public std::exception
    {
        int row;
        int column;
        std::string msg;
    public:
        ParseError(int r, int c, const std::string &message);
        int getRow() const;
        int getColumn() const;
        const char *what() const noexcept(true);
    };

    class Row : public std::vector<std::string>
    {
        friend class CSV;

        CSV *parent;
    public:
        const CSV& getParent() const;
        Row& operator<<(const std::string & str);
    private:
        Row(CSV *parent);
    };

    struct Huffman
    {
        std::map<char, Huffman> follow;
        std::map<char, int> uses;
        Huffman *current = this;
        std::string path;

        void insert(const std::string &s);
        void remove(const std::string &s);
        bool validate(char c);
        bool complete() const;
        void reset();
    };

    enum class TokenType
    {
        Separator,
        Quote,
        NewLine,
        Escape,
        Unknown
    };

    enum class State
    {
        BeforeField,
        Field,
        QuotedField,
        Escape,
        MaybeOOEscape,
        BeforeSeparator
    };

    CSV(    std::vector<std::string> separators   = {";", ",", "\t"},
            std::vector<std::string> quotes       = {"'","\""},
            std::vector<std::string> newlines     = {"\n","\r\n"},
            std::vector<std::string> escapes      = {"\\"}
        );

    void read(const std::string &path);
    void parse(const std::string & str);
    void parse(char c);

    void gotField();
    void gotRow();

    virtual std::string onField(const std::string &field);
    virtual Row &onRow(Row &row);

    static std::string replaceAll( std::string const& original, std::string const& before, std::string const& after, int *replaced = nullptr);

    const std::map<std::string, TokenType>& getTokenTypes() const;

    const std::string& getSeparator() const;
    const std::string& getQuote() const;
    const std::string& getNewline() const;
    const std::string& getEscape() const;

protected:


private:

    void purge(TokenType tt, const std::string &except);
    bool handle(const std::string &token, TokenType tt);
    void transition(State s);

    Huffman huff;
    std::map<std::string, TokenType> token_types;
    std::string data;
    TokenType   token_type;
    State state = State::BeforeField;

    int row = 0;
    int col = 0;

    Row current_row;

    std::string default_separator = ";";
    std::string default_quote     = "\"";
    std::string default_newline   = "\n";
    std::string default_escape    = "\'";

    bool allow_oo_escape = true;

    std::string separator;
    std::string quote;
    std::string newline;
    std::string escape;

};

std::ostream& operator<< (std::ostream &o, const CSV::Row row);

#endif // CSV_H
