#include "ast_ee.hpp"
#include <vector>
#include <string>
#include <stdio.h>

using std::string;
using std::vector;

extern FILE *logOut;

EENode::EENode(LineType _lineType)
    : lineType(_lineType) {}

EENodeAssign::EENodeAssign(EENodeRight *_left,
                           EENodeRight *_right)
    : EENode(ltASSIGN), left(_left), right(_right) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeGoto::EENodeGoto(int _dstLabel,
                       EENodeOp *_condition)
    : EENode(ltGOTO), dstLabel(_dstLabel), condition(_condition) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeReturn::EENodeReturn(EENodeRight *_dstRet)
    : EENode(ltRETURN), dstRet(_dstRet) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeRight::EENodeRight(LineType _lineType)
    : EENode(_lineType) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeSymbol::EENodeSymbol(const string &_name)
    : EENodeRight(ltSYMBOL), name(_name) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeNum::EENodeNum(int _value)
    : EENodeRight(ltNUM), value(_value) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeOp::EENodeOp(LineType _lineType,
                   const string &_opType,
                   EENodeRight *_left,
                   EENodeRight *_right)
    : EENodeRight(_lineType), opType(_opType), left(_left), right(_right) {
    // fprintf(stdout, "%s\n", __FUNCTION__);
}

EENodeCall::EENodeCall(const string &_name,
                       vector<EENodeRight *> &_params)
    : EENodeRight(ltCALL), name(_name) {
    // fprintf(stdout, "%s\n", __FUNCTION__);

    params.assign(_params.begin(), _params.end());
}