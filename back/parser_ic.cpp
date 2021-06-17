#include "parser_ic.hpp"
#include "assert.h"
#include <utility>

using std::to_string;

ParserIC::ParserIC(const string &_codeEeyore,
                   bool _ifTigger,
                   bool _ifRiscV)
    : codeEeyore(std::move(_codeEeyore)), ifTigger(_ifTigger), ifRiscV(_ifRiscV) {
    // if demand tigger code, generate tigger ast first
    if (ifTigger) {
        int lineno = 1;
        stringstream codeInput(codeEeyore);
        string line;
        symTabEE = SymbolTableEE();

        // parse lines and generate tigger ast
        do {
            getline(codeInput, line);
            if (codeInput.fail())   // have read all lines
                break;
            if (line.empty()) {
                lineno++;
                continue;
            }
            parseLine(line, lineno++);
        } while (true);

        compile();
    }
}

void ParserIC::parseLine(const string &line, int lineno) {
    stringstream lineInput(line);
    char tmpCh;
    int value;
    string tmpStr, name, op;
    EENode *statement = nullptr;

    if (line[0] != '\t') {
        switch (line[0]) {
            case 'v':   // var (width) var_name
                genVarDeclEE(lineInput);
                break;
            case 'l':   // label:
                lineInput >> tmpCh >> value;
                labels.insert(make_pair(value, lineno));
                break;
            case 'f':   // f_name [param_num]
                lineInput >> name >> tmpCh >> value;
                symTabEE.curFuncName = name;
                symTabEE.registerFunc(name, value);
                break;
            case 'e':   // end f_name
            {
                EntryFuncEE *curFunc = symTabEE.funcTable[symTabEE.curFuncName];
                // update goto node
                for (auto &i : curFunc->stmtSeq)
                    if (i.second->lineType == ltGOTO)
                        ((EENodeGoto *)i.second)->dstLabel = curFunc->labels[((EENodeGoto *)i.second)->dstLabel]->label;
                
                // update basic block information
                bool ifNextBlock = false;
                for (auto &i : curFunc->stmtSeq) {
                    if (i.second->label != -1 || ifNextBlock)
                        i.second->ifEntry = true;

                    if (i.second->lineType == ltGOTO && ((EENodeGoto *)i.second)->condition != nullptr)   // a if-goto node
                        ifNextBlock = true;
                    else
                        ifNextBlock = false;
                }

                // add params in tmpVars
                for (int i = 0; i < curFunc->numParams; i++)
                    symTabEE.registerVar("p" + to_string(i));

                symTabEE.curFuncName = "g";
                break;
            }   
            default :
                break;
        }
    } else {            // line start with \t, which means statements in a function and not a label
        lineInput.get();
        switch (line[1]) {
            case 'v':   // var (width) var_name
                genVarDeclEE(lineInput);
                break;
            case 'p':   // param var_name OR pn = ...
                if (line[2] == 'a') {   // param var_name
                    lineInput >> tmpStr >> name;
                    params.push_back(parseValue(name));
                    break;
                } else {                // pn = ...
                    // fall through
                }
            case 'T':   // Tn = ... OR pn = ...
                // fall through
            case 't':   // tn = ... OR Tn = ... OR pn = ...
            {
                lineInput >> name >> tmpStr;
                
                // printf("%s\n", name.c_str());
                
                EENodeRight *left = parseRight(name);
                lineInput.get();
                getline(lineInput, name);
                
                // printf("%s\n", name.c_str());
                
                EENodeRight *right = parseRight(name);
                // remove 0 in statement such as "a = b + 0" or "c = d - 0"
                // to do...
                statement = new EENodeAssign(left, right);
                break;
            }
            case 'i':   // if var_name op var_name goto label
            {
                lineInput >> tmpStr >> op;
                lineInput >> tmpStr; op += " " + tmpStr;
                lineInput >> tmpStr; op += " " + tmpStr;
                EENodeRight *condition = parseRight(op);
                lineInput >> tmpStr >> tmpCh >> value;
                statement = new EENodeGoto(value, (EENodeOp *)condition);
                break;
            }
            case 'g':   // goto label
                lineInput >> tmpStr >> tmpCh >> value;
                statement = new EENodeGoto(value);
                break;
            case 'r':   // return (var_name)
                lineInput >> tmpStr;
                if (lineInput.peek() == EOF)// return
                {
                    statement = new EENodeReturn();
                }
                else {                                  // return var_name
                    lineInput >> name;
                    statement  = new EENodeReturn(parseValue(name));
                }
                break;
            case 'c':   // call f_name
                getline(lineInput, tmpStr);
                statement = parseRight(tmpStr);
                break;
            default:
                break;
        }

        if (statement) {
            statement->lineno = lineno;
            statement->line = line;
            symTabEE.addStmt(statement);
            
            // clear redundant labels such as:
            // l1:
            // l2:
            // l3:
            //     statement...
            if (!labels.empty()) {
                int maximum = -1;
                for (auto i : labels)
                    if (i.first > maximum)
                        maximum = i.first;
                statement->label = maximum;
                
                for (auto i : labels)
                    symTabEE.funcTable[symTabEE.curFuncName]->labels[i.first] = statement;
                labels.clear();
            }
        }
    }
}

void ParserIC::genVarDeclEE(stringstream &input) {
    char tmp;
    string var;
    input >> var;
    input.get();
    tmp = (char)input.peek();
    if (tmp >= '0' && tmp <= '9') { // width SYMBOL
        int width;
        input >> width >> var;
        symTabEE.registerVar(var, width, true);
    } else {                        // SYMBOL
        input >> var;
        symTabEE.registerVar(var);
    }
}

EENodeRight *ParserIC::parseValue(const string &name) {
    if (name[0] <= '9' && name[0] >= '0' || name[0] == '-')  // NUM
        return new EENodeNum((int)strtol(name.c_str(), NULL, 0));
    else                                                    // SYMBOL
        return new EENodeSymbol(name);
}

EENodeRight *ParserIC::parseRight(const string &input) {
    stringstream code(input);
    char opUn;
    string opBin, tmp, name;
    
    if (input[0] == '-' || input[0] == '!') {   // unary expression
        code >> opUn >> name;
        if (name[0] >= '0' && name[0] <= '9') {
            int val = (int)strtol(name.c_str(), NULL, 0);
            if (opUn == '-')
                val = -val;
            else if (opUn == '!')
                val = !val;
            else
                assert(false);
            return new EENodeNum(val);
        }
        if (opUn == '-')
            return new EENodeOp(ltUNOP, "-", nullptr, parseValue(name));
        else if (opUn == '!')
            return new EENodeOp(ltUNOP, "!", nullptr, parseValue(name));
        else 
            assert(false);
    }

    if (input[0] == 'c') {                        // function call
        code >> tmp >> name;
        EENodeCall *ret = new EENodeCall(name, params);
        params.clear();
        return ret;
    }

    // var_name OR binary expression    
    int li, ri;
    if ((li = input.find('[')) != string::npos) {    // an array
        ri = input.find(']');
        EENodeRight *symbol = parseValue(input.substr(0, li));
        EENodeRight *exp = parseValue(input.substr(li+1, ri-li-1));
        return new EENodeOp(ltARRAY, "", symbol, exp);
    }
    
    // binary expression or NUM or SYMBOL
    string left, right;
    code >> left;
    
    // NUM or SYMBOL
    if (code.peek() == EOF)
        return parseValue(left);
    
    // binary expression
    code >> opBin >> right;
    return new EENodeOp(ltBINOP, opBin, parseValue(left), parseValue(right));
}