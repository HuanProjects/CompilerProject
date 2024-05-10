#ifndef TREENODE_H
#define TREENODE_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <stack>

class TreeNode {
protected:
    std::string data;
    std::string nodeType;
    std::vector<std::shared_ptr<TreeNode>> children;

public:
    TreeNode(const std::string& data, const std::string& nodeType)
        : data(data), nodeType(nodeType) {
            children.reserve(5);
        }

    void addChild(const std::string& val, const std::string& nodeType) {
        // Assuming a default node type for simple values; adjust as necessary
        // children.push_back(std::make_shared<TreeNode>(val, nodeType));
        // debugging
        auto newChild = std::make_shared<TreeNode>(val, nodeType);
        children.push_back(newChild);
    }

    void addChild(std::shared_ptr<TreeNode>& a) {
        children.push_back(a);
    }

    std::string getData() const {
        return data;
    }

    std::string getNodeType() const {
        return nodeType;
    }

    void processNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string& currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);

    void traverseDFS(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string& currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);

    void processVariableDeclarationNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput);

    void processVariableNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput) {
        if (symbolTable.find(data) == symbolTable.end()){
            std::cerr << "Error: Variable "<< data <<" is not declared"<< std::endl;
        }
    }

    void processExpressionNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);

    void processAssignmentNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput);

    void processPrepareIfNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);

    void processIfNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);

    void processPrepareForNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);

    void processForNode(std::map<std::string, std::string>& symbolTable, std::stack<std::string>& registerStack, std::string &currentOutput, std::map<std::string,int>& statementCount, std::stack <std::string>& conditionContext);
};

#endif