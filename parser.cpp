#include <iostream>
#include <string>
#include <stack> 
#include <map>
#include <algorithm>
#include "TreeNode.h"
#include <pugixml.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>

// Initializing the stack
std::stack<std::string> initializeRegisterStack() {
    std::stack<std::string> stack;
    stack.push("$s9");
    stack.push("$s8");
    stack.push("$s7");
    stack.push("$s6");
    stack.push("$s5");
    stack.push("$s4");
    stack.push("$s3");
    stack.push("$s2");
    stack.push("$s1");
    stack.push("$s0");
    stack.push("$t9");
    stack.push("$t8");
    stack.push("$t7");
    stack.push("$t6");
    stack.push("$t5");
    stack.push("$t4");
    stack.push("$t3");
    stack.push("$t2");
    stack.push("$t1");
    stack.push("$t0");
    return stack;
}


// Helper function to trim leading and trailing whitespace from a string
std::string trim(const std::string& str) {
    // Find the first character position after excluding leading whitespace
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    
    // Check if the string is all whitespaces
    if (start == std::string::npos)
        return "";

    // Find the last character position before excluding trailing whitespace
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    // Return the trimmed string
    return str.substr(start, end - start + 1);
}

std::shared_ptr<TreeNode> parseVariableDeclaration(pugi::xml_node& node) {
    // Extract type and name values from the XML node
    std::string type = node.child_value("type");
    type = trim(type);
    std::string name = node.child_value("name");
    name = trim(name);
    // Create a new VariableDeclarationNode
    auto root = std::make_shared<TreeNode>("Statement", "Variable Declaration");
    // Add type and name as children of this node
    
    root->addChild(type, "type");
    root->addChild(name, "name");
    return root;
}


std::shared_ptr<TreeNode> parseExpression(pugi::xml_node& node){
    // Check if the operation attribute exists
    if (!node.attribute("operation")) {
        std::cerr << "Error: Expression does not have an 'operation' attribute." << std::endl;
        return nullptr;
    }

    // Extract values
    std::string operation = node.attribute("operation").value(); 
    // Create a node for Expression
    auto root = std::make_shared<TreeNode>("Statement", "Expression");
    // Add the operation as a child
    root->addChild(operation, "Operation");

    // Handle operand1
    pugi::xml_node operand1 = node.child("operand1").first_child();
    if (std::string(operand1.name()) == "variable"){
        // syntax checking
        if (!operand1.child("name")) {
            std::cerr << "Error: Missing 'name' tag in operand1's variable." << std::endl;
            return nullptr;
        }
        std::string a = trim(operand1.child("name").child_value());
        root->addChild(a, "Variable");
    } else if (std::string(operand1.name()) == "literal"){
        // syntax checking
        if (!operand1.child("int")) {
            std::cerr << "Error: Missing 'int' tag in operand1's literal." << std::endl;
            return nullptr;
        }
        std::string a = trim(operand1.child("int").child_value());
        root->addChild(a, "Literal");
    } else {
        std::cerr << "Error: Unsupported operand type: " << operand1.name() << std::endl;
    }

    // Handle operand2
    pugi::xml_node operand2 = node.child("operand2").first_child();
    if (std::string(operand2.name()) == "variable"){
        //syntax checking
        if (!operand2.child("name")) {
            std::cerr << "Error: Missing 'name' tag in operand2's variable." << std::endl;
            return nullptr;
        }
        std::string b = trim(operand2.child("name").child_value());
        root->addChild(b, "Variable");
    } else if (std::string(operand2.name()) == "literal"){
        // syntax checking
        if (!operand2.child("int")) {
            std::cerr << "Error: Missing 'int' tag in operand2's literal." << std::endl;
            return nullptr;
        }
        std::string b = trim(operand2.child("int").child_value());
        root->addChild(b, "Literal");
    } else {
        std::cerr << "Error: Unsupported operand type: " << operand2.name() << std::endl;
    }

    return root;
}

std::shared_ptr<TreeNode> parseAssignment(pugi::xml_node& node){
    auto root = std::make_shared<TreeNode>("Statement", "Assignment");
    
    //parsing the left hand side
    auto lhs = node.child("lhs").first_child(); 
    if (std::string(lhs.name()) != "variable"){
        std::cerr << "Error: the left hand side needs to be variable!" << std::endl; 
        return nullptr;
    } else {
        //syntax checking
        if (!lhs.child("name")) {
            std::cerr << "Error: Missing 'name' tag in left hand side's variable." << std::endl;
            return nullptr;
        }
        std::string a = trim(lhs.child("name").child_value());
        root->addChild(a, "Variable");
    }

    //parsing the right hand side
    auto rhs = node.child("rhs").first_child(); 
    if (std::string(rhs.name()) == "expression"){
        std::shared_ptr<TreeNode> a = parseExpression(rhs);
        root->addChild(a); 
    } else if (std::string(rhs.name()) == "variable"){
        //syntax checking
        if (!rhs.child("name")) {
            std::cerr << "Error: Missing 'name' tag in right hand side's variable." << std::endl;
            return nullptr;
        }
        std::string a = trim(rhs.child("name").child_value());
        root->addChild(a, "Variable");
    } else if (std::string(rhs.name()) == "literal"){
        // syntax checking
        if (!rhs.child("int")) {
            std::cerr << "Error: Missing 'int' tag in right hand side's literal." << std::endl;
            return nullptr;
        }
        std::string a = trim(rhs.child("int").child_value());
        root->addChild(a, "Literal");
    } else {
        std::cerr << "Error: Unsupported operand type: " << rhs.name() << std::endl;
    }

    return root; 
}

std::shared_ptr<TreeNode> parseIfStatement(pugi::xml_node& node){
    auto root = std::make_shared<TreeNode>("Statement", "If");

    // add a node that will prepare the if statement
    auto prepareIf = std::make_shared<TreeNode> ("Statement", "PrepareIf"); 
    root ->addChild (prepareIf);
    
    // Parse condition
    pugi::xml_node conditionNode = node.child("condition");
    if (!conditionNode) {
        std::cerr << "Error: If statement without a 'condition' tag." << std::endl;
        return nullptr;
    }

    pugi::xml_node expressionNode = conditionNode.child("expression");
    if (!expressionNode) {
        std::cerr << "Error: 'condition' tag without 'expression' child." << std::endl;
        return nullptr;
    }

    std::shared_ptr<TreeNode> condition = parseExpression(expressionNode);
    if (!condition) {
        std::cerr << "Error: Cannot parse condition in if statement." << std::endl;
        return nullptr;
    }
    root->addChild(condition);

    // Parse body
    pugi::xml_node bodyNode = node.child("body");
    if (!bodyNode) {
        std::cerr << "Error: If statement without a 'body' tag." << std::endl;
        return nullptr;
    }

    for (pugi::xml_node child = bodyNode.first_child(); child; child = child.next_sibling()) {
        std::string childName = child.name();
        if (childName == "assignment") {
            std::shared_ptr<TreeNode> assignment = parseAssignment(child);
            if (!assignment) {
                std::cerr << "Error: Cannot parse assignment statement in if body." << std::endl;
                return nullptr;
            }
            root->addChild(assignment);
        } else if (childName == "variable-declaration") {
            std::shared_ptr<TreeNode> a = parseVariableDeclaration(child);
            root->addChild(a);
        } else if (childName == "if") {
            std::shared_ptr<TreeNode> a = parseIfStatement(child);
            root->addChild(a);
        } else {
            std::cerr << "Error: Unsupported statement type in if body: " << childName << std::endl;
            return nullptr;
        }
    }
    return root;
}

std::shared_ptr<TreeNode> parseForLoop(pugi::xml_node& node){
    auto root = std::make_shared <TreeNode> ("Statement", "For"); 

    // add a node that will prepare the for statement
    auto prepareFor = std::make_shared<TreeNode> ("Statement", "PrepareFor"); 
    root ->addChild (prepareFor);

    // Parse initialization
    pugi::xml_node initializationNode = node.child("initialization");
    auto initNode = std::make_shared<TreeNode>("Statement","Initialization");
    for (pugi::xml_node child = initializationNode.first_child(); child; child = child.next_sibling()) {
        std::string childName = child.name();
        if (childName == "variable-declaration") {
            std::shared_ptr<TreeNode> varDecl = parseVariableDeclaration(child);
            initNode->addChild(varDecl);
        } else if (childName == "assignment") {
            std::shared_ptr<TreeNode> assign = parseAssignment(child);
            initNode->addChild(assign);
        }
    }
    root->addChild(initNode);

    // Parse condition
    pugi::xml_node conditionNode = node.child("condition").child("expression");
    if (!conditionNode) {
        std::cerr << "Error: Missing condition in for loop." << std::endl;
        return nullptr;
    }
    std::shared_ptr<TreeNode> condition = parseExpression(conditionNode);
    if (!condition) {
        std::cerr << "Error: Cannot Parsing condition in for loop." << std::endl;
        return nullptr;
    }
    root->addChild(condition); 

    // Parse body
    auto bodyNode = std::make_shared<TreeNode>("Body", "");
    for (pugi::xml_node child = node.child("body").first_child(); child; child = child.next_sibling()) {
        std::string childName = child.name();
        if (childName == "assignment") {
            std::shared_ptr<TreeNode> assignment = parseAssignment(child);
            bodyNode->addChild(assignment);
        } else if (childName == "variable-declaration") {
            std::shared_ptr<TreeNode> a = parseVariableDeclaration(child);
            root->addChild(a);
        } else if (childName == "if") {
            std::shared_ptr<TreeNode> a = parseIfStatement(child);
            root->addChild(a);
        }  else if (childName == "for") {
            std::shared_ptr<TreeNode> a = parseForLoop(child);
            root->addChild(a);
        } else {
            std::cerr << "Unsupported statement type in for body: " << childName << std::endl;
            return nullptr;
        }
    }
    root->addChild(bodyNode);

    // Parse iteration step
    pugi::xml_node iterationNode = node.child("iteration").child("assignment");
    if (!iterationNode) {
        std::cerr << "Error: Missing iteration in for loop." << std::endl;
        return nullptr;
    }
    std::shared_ptr<TreeNode> iteration = parseAssignment(iterationNode);
    if (!iteration) {
        std::cerr << "Error parsing iteration in for loop." << std::endl;
        return nullptr;
    }
    root->addChild(iteration);

    return root;
}

int main(int argc, char* argv[]) {
    // Check if the command line arguments are provided
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input XML file> <output text file>\n";
        return 1;
    }

    // Load XML file from first command line argument
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(argv[1]);

    if (!result) {
        std::cerr << "Error: Cannot parse XML file: " << result.description() << std::endl;
        return 1;
    }

    // Open the output file stream with second command line argument
    std::ofstream outFile(argv[2]);
    if (!outFile) {
        std::cerr << "Error: Cannot open output file.\n";
        return 1;
    }

    // get thr root node
    pugi::xml_node program = doc.child("program");
    
    // get the list of registers
    std::stack<std::string> registerStack = initializeRegisterStack(); 
    // creating the symbol table
    std::map<std::string, std::string> symbolTable; 

    std::shared_ptr<TreeNode> root = std::make_shared<TreeNode>("program","Main"); 
    // get the child node
    for (pugi::xml_node node = program.first_child(); node; node = node.next_sibling()) {
        std::string nodeName = node.name();
        if (nodeName == "variable-declaration") {
            std::shared_ptr<TreeNode> a = parseVariableDeclaration(node);
            root->addChild(a); 
        } else if (nodeName == "expression") {
            std::shared_ptr<TreeNode> a = parseExpression(node);
            root->addChild(a); 
        } else if (nodeName == "assignment"){
            std::shared_ptr<TreeNode> a = parseAssignment(node);
            root->addChild(a); 
        } else if (nodeName == "if"){
            std::shared_ptr<TreeNode> a = parseIfStatement(node);
            root->addChild(a); 
        } else if (nodeName == "for"){
            std::shared_ptr<TreeNode> a = parseForLoop(node);
            root->addChild(a); 
        }
    }
    
    //creating the context for the if and for statement
    std::map<std::string, int> statementCount; 
    statementCount["if"] = 0; 
    statementCount["for"] = 0; 
    std::stack <std::string> conditionContext; 
    std::string output; 
    root->traverseDFS(symbolTable, registerStack, output, statementCount, conditionContext);

    outFile << output << std::endl;
    outFile.close();
    return 0;
}