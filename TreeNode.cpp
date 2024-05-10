#include "TreeNode.h"
#include <stack> 
#include <map>
#include <iostream>

void TreeNode::processNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext) {
    if (this -> nodeType == "Variable Declaration") {
        processVariableDeclarationNode(symbolTable, registerStack, currentOutput);
    } else if (this -> nodeType == "Variable"){
        processVariableNode(symbolTable, registerStack, currentOutput);
    } else if (this -> nodeType == "Expression"){
        processExpressionNode(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
    } else if (this -> nodeType == "Assignment"){
        processAssignmentNode(symbolTable, registerStack, currentOutput);
    } else if (this -> nodeType == "If"){
        processIfNode(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
    } else if (this -> nodeType == "PrepareIf"){
        processPrepareIfNode(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
    } else if (this -> nodeType == "PrepareFor"){
        processPrepareForNode(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
    } else if (this -> nodeType == "For"){
        processForNode(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
    }
}

void TreeNode::traverseDFS(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string& currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext) {
    for (auto& child : children) {
        if (child) {  // Check if the shared_ptr is valid
            child->traverseDFS(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
        }
    }
    processNode(symbolTable, registerStack, currentOutput, statementCount, conditionContext);
}


void TreeNode::processVariableDeclarationNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput) {
    if (children.size() < 2) {
        std::cerr << "Error: Not enough children in variable declaration node" << std::endl;
        return;
    }

    std::string variableType = this->children[0]->getData(); 
    std::string variableName = this->children[1]->getData();
    if (symbolTable.find(variableName) == symbolTable.end()) {
        if (!registerStack.empty()) {
            std::string registerValue = registerStack.top();
            registerStack.pop(); 
            symbolTable[variableName] = registerValue; 
        } else {
            std::cerr << "Error: The register stack is full" << std::endl;
            symbolTable[variableName] = ""; 
        }
    }
    currentOutput += "li " + symbolTable[variableName] +", 0\n";
}

void TreeNode::processExpressionNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext){
    if (children.size() < 3) {
        std::cerr << "Error: Not enough children in expression node" << std::endl;
        return;
    }

    std::string operation = children[0] -> getData(); 
    std::string tempRegister; // temporary registers for numbers
    std::string operand1; 
    std::string operand2; 

    // Retrieve the values of the first operand
    if (children[1]->getNodeType() == "Variable"){
        operand1 = symbolTable.at(children[1]->getData());
    } else if (children[1]->getNodeType() == "Literal"){
        operand1 = children[1]->getData();
        if (!registerStack.empty()) {
            tempRegister = registerStack.top();
            registerStack.pop();
            currentOutput += "li " + tempRegister + ", " + operand1 + "\n";
            operand1 = tempRegister;
        } else {
            std::cerr << "Error: No registers available for literals" << std::endl;
            return;
        }
    }

    // Retrieve the values of the second operand
    if (children[2]->getNodeType() == "Variable"){
        operand2 = symbolTable.at(children[2]->getData());
    } else if (children[2]->getNodeType() == "Literal") {
        operand2 = children[2]->getData();
        if (!registerStack.empty()) {
            tempRegister = registerStack.top();
            registerStack.pop();
            currentOutput += "li " + tempRegister + ", " + operand2 + "\n";
            operand2 = tempRegister;
        } else {
            std::cerr << "Error: No registers available for literals" << std::endl;
            return;
        }
    }

    //"addition", "subtraction", "multiplication", "division", "modulo", "lessThan", "biggerThan",
    // "biggerEqual", "lessEqual", "equal"
    if (!registerStack.empty()) {
        tempRegister = registerStack.top();
        registerStack.pop();
        if (operation == "addition") {
            currentOutput += "add " + tempRegister + ", " + operand1 + ", " + operand2 + "\n";
        } else if (operation == "subtraction") {
            currentOutput += "sub " + tempRegister + ", " + operand1 + ", " + operand2 + "\n";
        } else if (operation == "multiplication") {
            currentOutput += "mul " + tempRegister + ", " + operand1 + ", " + operand2 + "\n";
        } else if (operation == "division") {
            currentOutput += "div " + operand1 + ", " + operand2 + "\n";
            currentOutput += "mflo " + tempRegister + "\n";
        } else if (operation == "modulo") {
            currentOutput += "div " + operand1 + ", " + operand2 + "\n";
            currentOutput += "mfhi " + tempRegister + "\n";
        } else if (operation == "lessThan"){ 
            if (conditionContext.empty()){
                std::cerr << "Error: Condition context stack is empty." << std::endl; 
                return;
            } else if (conditionContext.top() == "if"){
                std::string offset;
                offset = "endIf" + std::to_string(statementCount["if"]);
                conditionContext.pop();
                conditionContext.push(offset);
                currentOutput += "bge " + operand1 + ", " + operand2 + ", " + offset + "\n";
            } else if (conditionContext.top() == "for"){
                std::string offset2 = "endFor" + std::to_string(statementCount["for"]);
                std::string offset1 = "bodyFor" + std::to_string(statementCount["for"]);
                currentOutput += offset1 + ":\n";
                currentOutput += "bge " + operand1 + ", " + operand2 + ", " + offset2 + "\n";
                conditionContext.pop();
                conditionContext.push(offset2);
                conditionContext.push(offset1);
            } else {
                std::cerr<< "Error: Condition stack is not proprer popped. Top condition: " << conditionContext.top() << std::endl;
            }
        } else if (operation == "biggerThan"){ 
            if (conditionContext.empty()){
                std::cerr << "Error: Condition context stack is empty." << std::endl; 
                return;
            } else if (conditionContext.top() == "if"){
                std::string offset;
                offset = "endIf" + std::to_string(statementCount["if"]);
                conditionContext.pop();
                conditionContext.push(offset);
                currentOutput += "ble " + operand1 + ", " + operand2 + ", " + offset + "\n";
            } else if (conditionContext.top() == "for"){
                std::string offset2 = "endFor" + std::to_string(statementCount["for"]);
                std::string offset1 = "bodyFor" + std::to_string(statementCount["for"]);
                currentOutput += offset1 + ":\n";
                currentOutput += "ble " + operand1 + ", " + operand2 + ", " + offset2 + "\n";
                conditionContext.pop();
                conditionContext.push(offset2);
                conditionContext.push(offset1);
            } else {
                std::cerr<< "Error: Condition stack is not proprer popped. Top condition: " << conditionContext.top() << std::endl;
            }
        } else if (operation == "lessEqual"){ 
            if (conditionContext.empty()){
                std::cerr << "Error: Condition context stack is empty." << std::endl; 
                return;
            } else if (conditionContext.top() == "if"){
                std::string offset;
                offset = "endIf" + std::to_string(statementCount["if"]);
                conditionContext.pop();
                conditionContext.push(offset);
                currentOutput += "bgt " + operand1 + ", " + operand2 + ", " + offset + "\n";
            } else if (conditionContext.top() == "for"){
                std::string offset2 = "endFor" + std::to_string(statementCount["for"]);
                std::string offset1 = "bodyFor" + std::to_string(statementCount["for"]);
                currentOutput += offset1 + ":\n";
                currentOutput += "bgt " + operand1 + ", " + operand2 + ", " + offset2 + "\n";
                conditionContext.pop();
                conditionContext.push(offset2);
                conditionContext.push(offset1);
            } else {
                std::cerr<< "Error: Condition stack is not proprer popped. Top condition: " << conditionContext.top() << std::endl;
            }
        } else if (operation == "biggerEqual"){ 
            if (conditionContext.empty()){
                std::cerr << "Error: Condition context stack is empty." << std::endl; 
                return;
            } else if (conditionContext.top() == "if"){
                std::string offset;
                offset = "endIf" + std::to_string(statementCount["if"]);
                conditionContext.pop();
                conditionContext.push(offset);
                currentOutput += "blt " + operand1 + ", " + operand2 + ", " + offset + "\n";
            } else if (conditionContext.top() == "for"){
                std::string offset2 = "endFor" + std::to_string(statementCount["for"]);
                std::string offset1 = "bodyFor" + std::to_string(statementCount["for"]);
                currentOutput += offset1 + ":\n";
                currentOutput += "blt " + operand1 + ", " + operand2 + ", " + offset2 + "\n";
                conditionContext.pop();
                conditionContext.push(offset2);
                conditionContext.push(offset1);
            } else {
                std::cerr<< "Error: Condition stack is not proprer popped. Top condition: " << conditionContext.top() << std::endl;
            }
        } else if (operation == "equal"){ 
            if (conditionContext.empty()){
                std::cerr << "Error: Condition context stack is empty." << std::endl; 
                return;
            } else if (conditionContext.top() == "if"){
                std::string offset;
                offset = "endIf" + std::to_string(statementCount["if"]);
                conditionContext.pop();
                conditionContext.push(offset);
                currentOutput += "bne " + operand1 + ", " + operand2 + ", " + offset + "\n";
            } else if (conditionContext.top() == "for"){
                std::string offset2 = "endFor" + std::to_string(statementCount["for"]);
                std::string offset1 = "bodyFor" + std::to_string(statementCount["for"]);
                currentOutput += offset1 + ":\n";
                currentOutput += "bne " + operand1 + ", " + operand2 + ", " + offset2 + "\n";
                conditionContext.pop();
                conditionContext.push(offset2);
                conditionContext.push(offset1);
            } else {
                std::cerr<< "Error: Condition stack is not proprer popped. Top condition: " << conditionContext.top() << std::endl;
            }
        } else if (operation == "notEqual"){ 
            if (conditionContext.empty()){
                std::cerr << "Error: Condition context stack is empty." << std::endl; 
                return;
            } else if (conditionContext.top() == "if"){
                std::string offset;
                offset = "endIf" + std::to_string(statementCount["if"]);
                conditionContext.pop();
                conditionContext.push(offset);
                currentOutput += "beq " + operand1 + ", " + operand2 + ", " + offset + "\n";
            } else if (conditionContext.top() == "for"){
                std::string offset2 = "endFor" + std::to_string(statementCount["for"]);
                std::string offset1 = "bodyFor" + std::to_string(statementCount["for"]);
                currentOutput += offset1 + ":\n";
                currentOutput += "beq " + operand1 + ", " + operand2 + ", " + offset2 + "\n";
                conditionContext.pop();
                conditionContext.push(offset2);
                conditionContext.push(offset1);
            } else {
                std::cerr<< "Error: Condition stack is not proprer popped. Top condition: " << conditionContext.top() << std::endl;
            }
        } else {
            std::cerr << "Error: operation " << operation << " not recognized" << std::endl; 
            return;
        }
    } else {
        std::cerr << "Error: No registers available to perform the operation" << std::endl;
        return;
    }

    bool popLiteral = true; 
    if (operation == "lessThan" || operation == "biggerThan" || operation == "biggerEqual" || 
        operation == "lessEqual" || operation == "equal") {
        popLiteral = false; 
    }
    //returning the temporaries registers
    if ((children[1] -> getNodeType() == "Literal") && popLiteral){
        registerStack.push(operand1);
    }
    if ((children[2] -> getNodeType() == "Literal") && popLiteral){
        registerStack.push(operand2);
    }
    registerStack.push(tempRegister);
}


void TreeNode::processAssignmentNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput){
    if (children.size() < 2) {
        std::cerr << "Error: Assignment node requires two children" << std::endl;
        return;
    }

    // process the left handside
    std::string lhs = children[0]->getData();
    if (symbolTable.find(lhs) == symbolTable.end()) {
        std::cerr << "Error: Variable " << lhs << " not declared" << std::endl;
        return;
    }
    std::string lhsRegister = symbolTable.at(lhs);

    std::string rhsRegister; 
    if (children[1]->getNodeType() =="Expression"){
        if (registerStack.empty()) {
            std::cerr << "Error: No register available to store the expression result" << std::endl;
            return;
        }
        rhsRegister = registerStack.top();
    } else if (children[1]->getNodeType() == "Variable"){
        rhsRegister = symbolTable.at(children[1]->getData());
    } else if (children[1]->getNodeType() == "Literal") {
        rhsRegister = children[1]->getData();
        if (!registerStack.empty()) {
            std::string tempRegister = registerStack.top();
            currentOutput += "li " + tempRegister + ", " + rhsRegister + "\n";
            rhsRegister = tempRegister;
        } else {
            std::cerr << "Error: No registers available for literals" << std::endl;
            return;
        }
    }
    currentOutput += "move " + lhsRegister + ", " + rhsRegister + "\n";
}

void TreeNode::processPrepareIfNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext){
    conditionContext.push("if");
    statementCount["if"] += 1;
}

void TreeNode::processIfNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext){
    std::string offset = conditionContext.top();
    conditionContext.pop();
    currentOutput += offset + ":\n";
}


void TreeNode::processPrepareForNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext){
    conditionContext.push("for");
    statementCount["for"] += 1;
}

void TreeNode::processForNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext){
    std::string offset = conditionContext.top();
    conditionContext.pop();
    currentOutput += "j " + offset + "\n";
    offset = conditionContext.top();
    conditionContext.pop();
    currentOutput += offset + ":\n";
}