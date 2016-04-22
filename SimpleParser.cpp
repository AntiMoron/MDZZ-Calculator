#include<cstdio>
#include<regex>
#include<string>
#include<iostream>
#include<vector>
#include<unordered_map>

enum token_type 
{
    token_number, // 数（浮点&整形）
    token_add, // 加号
    token_mul, // 乘号
    token_div, // 除号
    token_sub, // 减号
    token_lpar, // 左括号
    token_rpar, // 右括号
};

struct parser_token 
{
    bool hasValue;
    std::string tokenContent;
    token_type tokenType;
    std::string tokenTypeName;
    void display() const
    {
        std::cout << "{\ntokenContent: \t" << tokenContent << "\ntokenType: \t" 
            << tokenTypeName << "\n}" << std::endl;
    }
};

std::unordered_map<std::string, std::pair<token_type, std::string> > tokenMap {
    {"+", {token_add, "ADD"}},
    {"-", {token_sub, "ADD"}},
    {"*", {token_mul, "MUL"}},
    {"/", {token_div, "MUL"}},
    {"(", {token_lpar, "LPAR"}},
    {")", {token_rpar, "RPAR"}}
};
// 大写作为token的类型，小写作为syntax的一部分
std::unordered_map<std::string, std::vector<std::string> > ruleMap {
    {"add", {"mul ADD add", "mul"} },
    {"mul", {"atom MUL mul", "atom"}},
    {"atom", {"NUM", "LPAR add RPAR", "neg"}},
    {"neg", {"ADD atom"}}
};

std::vector<std::string> split(const std::string& origin, const std::string& spliter)
{
	std::vector<std::string> result;
	const char* csrc = origin.c_str();
    char src[242];
    std::strcpy(src, csrc);
	const char* sep = spliter.c_str();
	char *p = strtok(src, sep);
	while(p)
	{
	    result.push_back(p);
	    p = strtok(NULL, sep);
	}
	return result;
}

parser_token match_token(const std::string& token)
{
    auto it = tokenMap.find(token);
    if(it != tokenMap.end())
    {
        return { true, it->first, (it->second).first, (it->second).second};
    }
    return { true, token, token_number, "NUM"};
}

struct rule_match
{
    bool isMatched;
    std::string name;
    parser_token matchedToken;
    std::vector<rule_match> matched;
    std::vector<parser_token> remainingTokens;
    void display() const
    {
        displayRecursive(*this, 0);
    }
    double getValue() 
    {
        if(matchedToken.hasValue) 
        {
            if(matchedToken.tokenType == token_number) 
            {
                return std::stod(matchedToken.tokenContent);
            }
        }
        if(matched.size() == 1)
        {
            return matched[0].getValue();
        } 
        else if (matched.size() == 2)
        {
            if(name == "neg") 
            {
                switch(matched[0].matchedToken.tokenType)
                {
                case token_add:
                    return matched[1].getValue();
                case token_sub:
                    return -matched[1].getValue();
                default:
                    break;
                }
            }
        }
        else if (matched.size() == 3)
        {
            switch(matched[1].matchedToken.tokenType)
            {
            case token_add:
                return matched[0].getValue() + matched[2].getValue();
            case token_sub:  
                return matched[0].getValue() - matched[2].getValue();
            case token_div:
                return matched[0].getValue() / matched[2].getValue();
            case token_mul:
                return matched[0].getValue() * matched[2].getValue();
            default:
                if(matched[0].matchedToken.tokenType == token_lpar && matched[2].matchedToken.tokenType == token_rpar)
                {
                    return matched[1].getValue();
                }
                break;
            }
        }
        return 0;
    }
private:
    void displayRecursive(const rule_match& rule, int depth) const
    {
        std::printf("%*s", depth, "");
        std::cout << rule.name;
        if(rule.matchedToken.hasValue)
        {
            std::cout << " " << rule.matchedToken.tokenContent;
        }
        std::cout <<std::endl;
        if(rule.matched.empty())
        {
            return ;
        }
        for(const auto& item: rule.matched)
        {
            displayRecursive(item, depth + 1);
        }
    }
};

rule_match match(const std::string& ruleName, const std::vector<parser_token>& tokens)
{
    // std::printf("New Round(%lu)! %s \n", tokens.size(), ruleName.c_str());
    if(!tokens.empty() && ruleName == tokens[0].tokenTypeName)
    {
        // printf("Matched!\n");
        return {true, tokens[0].tokenTypeName, tokens[0],{},
            std::vector<parser_token>(tokens.begin() + 1, tokens.end())};
    }
    for(const auto& rule: ruleMap[ruleName])
    {
        auto remainingTokens = tokens;
        std::vector<rule_match> matchedRules;
        bool flag = false;
        // std::cout << "Current Rule:" << rule << std::endl;
        for(const auto& subrule: split(rule, " "))
        {
            // std::cout << "\t";
            // for (const auto& x: remainingTokens)
            // {
            //     std::cout << "(" << x.tokenContent <<"," << x.tokenTypeName << ")" << " ";
            // }
            // std::cout << std::endl;
            auto temp = match(subrule, remainingTokens);
            auto matched = temp.isMatched;
            remainingTokens = temp.remainingTokens;
            if(!matched)
            {
                flag = true;
                break;
            }
            matchedRules.push_back(temp);
        }
        if(!flag)
        {
            return {true, ruleName, {}, matchedRules, remainingTokens};
        }
    }
    // printf("Failed! %s \n",ruleName.c_str());
    return {false, "", {}, {}, {}};
}

void eval(const std::string& str) 
{
    std::regex reg("[\\d\\.]+|[\\S]", std::regex_constants::ECMAScript);
    auto words_begin = std::sregex_iterator(str.begin(),str.end(), reg);
    auto words_end = std::sregex_iterator();
    std::vector<parser_token> tokens;
    for(auto i = words_begin; i != words_end; ++i)
    {
        auto aToken = match_token(i->str());
        // aToken.display();
        tokens.push_back(aToken);
    }
    // std::cout << "======================================================" << std::endl;
    auto matchResult = match("add", tokens);
    // std::cout << "======================================================" << std::endl;
    if(matchResult.remainingTokens.empty())
    {
        // matchResult.display();
        std::cout << "Result: " << matchResult.getValue() << std::endl;
    }
    else
    {
        std::cout << "Ma de zhi zhang(s): [" << matchResult.remainingTokens.size() << "] :";
        for(const auto& error: matchResult.remainingTokens) 
        {
            error.display();
        }
    }
}

int main() 
{
    // eval("(11 + 3)");
    char input[1024];
    while(std::cout << ">> " && std::cin.getline(input, 1024))
    {
        eval(input);
    }
    return 0;
}