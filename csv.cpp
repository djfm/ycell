#include <algorithm>
#include <fstream>

#include "csv.h"

CSV::ParseError::ParseError(int r, int c, const std::string &message)
    :row(r), column(c), msg(message)
{
}

int CSV::ParseError::getRow() const
{
    return row;
}

int CSV::ParseError::getColumn() const
{
    return column;
}

const char *CSV::ParseError::what() const noexcept(true)
{
    std::ostringstream error;
    error<<"Parse error at ("<<row<<","<<column<<"): "<<msg;
    return error.str().c_str();
}

const CSV& CSV::Row::getParent() const
{
    return *parent;
}

CSV::Row &CSV::Row::operator <<(const std::string &str)
{
    push_back(str);
    return *this;
}

CSV::Row::Row(CSV *_parent)
    :parent(_parent)
{

}

CSV::CSV(std::vector<std::string> separators, std::vector<std::string> quotes, std::vector<std::string> newlines, std::vector<std::string> escapes)
    :current_row(this)
{
    for(auto s : separators)
    {
        token_types.insert({s, TokenType::Separator});
        huff.insert(s);
    }
    for(auto q : quotes    )
    {
        token_types.insert({q, TokenType::Quote});
        huff.insert(q);
    }
    for(auto n : newlines  )
    {
        token_types.insert({n, TokenType::NewLine});
        huff.insert(n);
    }
    for(auto e : escapes   )
    {
        token_types.insert({e, TokenType::Escape});
        huff.insert(e);
    }
}

void CSV::Huffman::insert(const std::string &s)
{
    if(s.empty())return;
    auto i = follow.find(s[0]);
    if(i == follow.end())
    {
        Huffman h;
        h.insert(s.substr(1));
        follow.insert({s[0], h});
        uses.insert({s[0],1});
    }
    else
    {
        ++uses[s[0]];
        i->second.insert(s.substr(1));
    }
}

void CSV::Huffman::remove(const std::string &s)
{
    if(s.empty())return;
    auto i = follow.find(s[0]);
    if(i != follow.end())
    {
        auto u = uses.find(s[0]);
        if(u->second == 1)
        {
            uses.erase(u);
            follow.erase(i);
        }
        else
        {
            --u->second;
            i->second.remove(s.substr(1));
        }
    }
}

bool CSV::Huffman::validate(char c)
{
    auto i = current->follow.find(c);
    if(i != current->follow.end())
    {
        path.push_back(c);
        current = &i->second;
        return true;
    }
    else
    {
        path.clear();
        current = this;
        return false;
    }
}

bool CSV::Huffman::complete() const
{
    return current->follow.empty();
}

void CSV::Huffman::reset()
{
    current = this;
    path.clear();
}

void CSV::read(const std::string &path)
{
    std::ifstream input(path);

    char buffer[256];
    int read;
    do
    {
        read = input.readsome(buffer,256);
        for(int i = 0; i < read; ++i)
        {
            parse(buffer[i]);
        }
    }while(read > 0);

    input.close();
}

void CSV::parse(const std::string &str)
{
    for(char c : str)parse(c);
}

void CSV::parse(char c)
{
    if(c == '\n')
    {
        ++row;
        col = 0;
    }
    else
    {
        ++col;
    }

    if(huff.validate(c))
    {
        if(huff.complete())
        {
            std::string token = huff.path;
            TokenType tt      = token_types[token];

            huff.reset();
            if(handle(token, tt))
            {
                data.clear();
            }
        }
    }
    else
    {
        data += c;
        handle("", TokenType::Unknown);
    }
}

bool CSV::handle(const std::string &token, CSV::TokenType tt)
{
    /*Guess Separator*/
    if(separator.empty() && tt == TokenType::Separator && (state == State::BeforeField || state == State::Field))
    {
        separator = token;
        purge(TokenType::Separator, separator);
    }

    /*Guess Quote*/
    if(quote.empty() && tt == TokenType::Quote && (state == State::BeforeField))
    {
        quote = token;
        purge(TokenType::Quote, quote);
    }

    switch(state)
    {
    case State::BeforeField:
        switch(tt)
        {
        case TokenType::Separator:
            gotField();
            transition(State::BeforeField);
            break;
        case TokenType::Quote:
            transition(State::QuotedField);
            break;
        case TokenType::NewLine:
            gotField();
            gotRow();
            transition(State::BeforeField);
            break;
       default:
            transition(State::Field);
            return false;
        }
        break;

    case State::Field:
        switch(tt)
        {
        case TokenType::Separator:
            gotField();
            transition(State::BeforeField);
            break;
        case TokenType::NewLine:
            gotField();
            gotRow();
            transition(State::BeforeField);
            break;
        case TokenType::Unknown:
            return false;
        default:
            data += token;
            return false;
        }
        break;

    case State::QuotedField:
        switch(tt)
        {
        case TokenType::Escape:
            transition(State::Escape);
            return false;
        case TokenType::Quote:
            if(allow_oo_escape)
            {
                transition(State::MaybeOOEscape);
            }
            else
            {
                transition(State::BeforeSeparator);
            }
            return false;
            break;
        case TokenType::Unknown:
            return false;
        default:
            data += token;
            return false;
        }
        break;

    case State::MaybeOOEscape:
        switch(tt)
        {
            case TokenType::Quote:
                data += token;
                transition(State::QuotedField);
                return false;
            case TokenType::Unknown:
                throw ParseError(row, col, "Unexpected char after quote!");
            default:
                transition(State::BeforeSeparator);
                return handle(token, tt);
        }

        break;

    case State::Escape:
        switch(tt)
        {
        case TokenType::Unknown:
        default:
            data += token;
        }
        transition(State::QuotedField);
        return false;
        break;

    case State::BeforeSeparator:
        switch(tt)
        {
        case TokenType::Separator:
            gotField();
            transition(State::BeforeField);
            break;
        case TokenType::NewLine:
            gotField();
            gotRow();
            transition(State::BeforeField);
            break;
        default:
            throw ParseError(row, col, "Separator or new line expected!");
        }
        break;
    }

    return true;
}

void CSV::purge(CSV::TokenType tt, const std::string &except)
{
    for(auto i = token_types.begin(); i!=token_types.end();)
    {
        if(i->second != tt)++i;
        else if(i->first == except)++i;
        else
        {
            huff.remove(i->first);
            i = token_types.erase(i);
        }
    }
}

void CSV::gotField()
{
    current_row << onField(data);
}

void CSV::gotRow()
{
    onRow(current_row);
    current_row.clear();
}

std::string CSV::onField(const std::string &field)
{
    return field;
}

CSV::Row& CSV::onRow(CSV::Row &row)
{
    return row;
}

void CSV::transition(CSV::State s)
{
    state = s;
}



std::string CSV::replaceAll(const std::string &original, const std::string &before, const std::string &after, int *replaced)
{
    if(replaced != nullptr)
    {
        *replaced = 0;
    }
    std::string retval;
    std::string::const_iterator end     = original.end();
    std::string::const_iterator current = original.begin();
    std::string::const_iterator next    = std::search( current, end, before.begin(), before.end() );
    while ( next != end )
    {
        if(replaced != nullptr)
        {
            *replaced += 1;
        }
        retval.append( current, next );
        retval.append( after );
        current = next + before.size();
        next = std::search( current, end, before.begin(), before.end() );
    }
    retval.append( current, next );
    return retval;
}

const std::map<std::string, CSV::TokenType> &CSV::getTokenTypes() const
{
    return token_types;
}

const std::string &CSV::getSeparator() const
{
    return separator.empty() ? default_separator : separator;
}

const std::string &CSV::getQuote() const
{
    return quote.empty() ? default_quote : quote;
}

const std::string &CSV::getNewline() const
{
    return newline.empty() ? default_newline : newline;
}

const std::string &CSV::getEscape() const
{
    return allow_oo_escape ? quote : (escape.empty() ? default_escape : escape);
}

std::ostream &operator <<(std::ostream &os, const CSV::Row row)
{
    std::string quote     = row.getParent().getQuote();
    std::string separator = row.getParent().getSeparator();
    std::string escape    = row.getParent().getEscape();
    std::string newline   = row.getParent().getEscape();

    bool first = true;
    for(std::string s : row)
    {
        if(first)first=false;
        else
        {
            os<<separator;
        }

        int n;
        s = CSV::replaceAll(s, quote, escape+quote, &n);
        if(n > 0 || s.find(separator) != std::string::npos || s.find(newline) != std::string::npos)
        {
            os<<quote<<s<<quote;
        }
        else os<<s;
    }
    os<<newline;
    return os;
}









