#pragma once
#include <string>
#include <set>
#include <queue>
#include <json/value.h>
#include "json/json.h"
using namespace std;

//Tree-related functions
struct RecipeTreeNode {
    string name;
    int numIngreds;
    Json::Value code;
    RecipeTreeNode* left;
    RecipeTreeNode* right;
    RecipeTreeNode(string x, int y, Json::Value z) : name(x), numIngreds(y), code(z), right(NULL), left(NULL) {}
};
int findHeight(RecipeTreeNode* node) {
    if (node == NULL) {
        return 0;
    }
    else if (node->left == NULL && node->right == NULL) {
        return 1;
    }
    else {
        return 1 + max(findHeight(node->left), findHeight(node->right));
    }
}
int BF(RecipeTreeNode* node) {
    return findHeight(node->left) - findHeight(node->right);
}
RecipeTreeNode* rotateLeft(RecipeTreeNode* node) {
    RecipeTreeNode* descendRL = node->right->left;
    RecipeTreeNode* newRoot = node->right;
    newRoot->left = node;
    node->right = descendRL;
    return newRoot;
}
RecipeTreeNode* rotateRight(RecipeTreeNode* node) {
    RecipeTreeNode* descendLR = node->left->right;
    RecipeTreeNode* newRoot = node->left;
    newRoot->right = node;
    node->left = descendLR;
    return newRoot;
}
RecipeTreeNode* rotateLeftRight(RecipeTreeNode* node) {
    node->left = rotateLeft(node->left);
    return rotateRight(node);
}
RecipeTreeNode* rotateRightLeft(RecipeTreeNode* node) {
    node->right = rotateRight(node->right);
    return rotateLeft(node);
}
RecipeTreeNode* balanceRecipeTree(RecipeTreeNode* root) {
    if (root == NULL)
        return NULL;
    root->left = balanceRecipeTree(root->left);
    root->right = balanceRecipeTree(root->right);
    if (BF(root) >= 2) { //left subtree is taller
        if (BF(root->left) == 1 || BF(root->left) == 0) { //Left Left Case
            root = rotateRight(root);
            root = balanceRecipeTree(root);
        }
        else if (BF(root->left) == -1) { //Left Right Case
            root = rotateLeftRight(root);
            root = balanceRecipeTree(root);
        }
        return root;
    }
    else if (BF(root) <= -2) { //right subtree is taller
        if (BF(root->right) == -1 || BF(root->right) == 0) { //Right Right Case
            root = rotateLeft(root);
            root = balanceRecipeTree(root);
        }
        else if (BF(root->right) == 1) { //Right Left Case
            //root = rotateRight(root);
            root = rotateRightLeft(root);
            root = balanceRecipeTree(root);
        }
        return root;
    }
    else
        return root; //do nothing if balance factor -1,0,1
}
RecipeTreeNode* insert(RecipeTreeNode* root, string name, int numIngreds, Json::Value code) {
    if (root == NULL) {
        //cout << "successful" << endl;
        return new RecipeTreeNode(name, numIngreds, code);
    }
    else if (numIngreds <= root->numIngreds) {
        root->left = insert(root->left, name, numIngreds, code);
    }
    else if (numIngreds > root->numIngreds) {
        root->right = insert(root->right, name, numIngreds, code);
    }

    root = balanceRecipeTree(root);
    return root;
}
void searchTreeByName(RecipeTreeNode* root, string name) {
    if (root == NULL) {
        cout << "";
    }
    else {
        searchTreeByName(root->left, name);
        istringstream ss(name);
        string term;
        bool found = false;
        while (ss >> term) {
            if (root->name.find(term) != std::string::npos)
                found = true;
            else
                found = false;
            if (!found)
                break;
        }
        if (found)
            cout << "\t" << root->name << endl;
        searchTreeByName(root->right, name);
    }
}
void searchTreeByIngredients(RecipeTreeNode* root, string list) {
    if (root == NULL) {
        cout << "";
    }
    else {
        searchTreeByIngredients(root->left, list);
        istringstream ss(list);
        string ingredient;
        bool found = false;
        auto currList = root->code["ingredients"];
        while (ss >> ingredient) {
            for (int i = 0; i < currList.size(); i++) {
                string currElem = currList[i].asString();
                if (currElem.find(ingredient) != std::string::npos) {
                    found = true;
                    break;
                }
                else {
                    found = false;
                }
            }
            if (!found)
                break;
        }
        if (found) {
            cout << "\t" << root->name << endl;
        }
        searchTreeByIngredients(root->right, list);
    }
}
RecipeTreeNode* getSimple(RecipeTreeNode* root) {
    if (root == NULL)
        return NULL;
    else if (root->left == NULL)
        return root;
    else
        return getSimple(root->left);
}
RecipeTreeNode* getComplex(RecipeTreeNode* root) {
    if (root == NULL)
        return NULL;
    else if (root->right == NULL)
        return root;
    else
        return getComplex(root->right);
}


//Graph-related functions
struct Recipe {
    string name;
    set<string> keywords;
    Json::Value code;
    Recipe(string x, set<string> y, Json::Value z) : name(x), keywords(y), code(z) {}
};
class RecipeGraph {
public:
    set<Recipe*> vertices;
    map<Recipe*, map<Recipe*, int>> adjList;
    void searchByName(string name);
    void searchByIngredients(string list);
    void similarRecipesTo(string name);
private:
    struct recipeComp {
        constexpr bool operator()(pair<Recipe*, int>const& a, pair<Recipe*, int>const& b) const noexcept {
            return a.second < b.second;
        }
    };
};
void RecipeGraph::searchByName(string name) {
    set<Recipe*> visited;
    //set<Recipe*> unvisited = vertices;
    queue<Recipe*> stk;
    visited.insert(*vertices.begin());
    //unvisited.erase(*vertices.begin());
    stk.push(*vertices.begin());
    while (!stk.empty()) {
        Recipe* u = stk.front();
        stk.pop();
        map<Recipe*, int> adj = adjList[u];
        istringstream ss(name);
        string term;
        bool found = false;
        while (ss >> term) {
            if (u->name.find(term) != std::string::npos)
                found = true;
            else
                found = false;
            if (!found)
                break;
        }
        if (found)
            cout << "\t" << u->name << endl;
        for (auto iter = adj.begin(); iter != adj.end(); iter++) {
            if (visited.find(iter->first) == visited.end()) {
                visited.insert(iter->first);
                //unvisited.erase(iter->first);
                stk.push(iter->first);
            }
        }/*
        if (stk.empty() && !unvisited.empty()) {
            visited.insert(*unvisited.begin());
            unvisited.erase(*unvisited.begin());
            stk.push(*unvisited.begin());
        }*/
    }
}
void RecipeGraph::searchByIngredients(string list) {
    set<Recipe*> visited;
    //set<Recipe*> unvisited = vertices;
    queue<Recipe*> stk;
    visited.insert(*vertices.begin());
    //unvisited.erase(*vertices.begin());
    stk.push(*vertices.begin());
    while (!stk.empty()) {
        Recipe* u = stk.front();
        stk.pop();
        map<Recipe*, int> adj = adjList[u];
        istringstream ss(list);
        string term;
        bool found = false;
        while (ss >> term) { //check if term is an element
            if (u->keywords.find(term) != u->keywords.end())
                found = true;
            else {// check if term is substring of an element
                for (auto iter = u->keywords.begin(); iter != u->keywords.end(); iter++) {
                    if ((*iter).find(term) != std::string::npos) {
                        found = true;
                        break;
                    }
                    else {
                        found = false;
                    }
                }
            }
            if (!found)
                break;
        }
        if (found)
            cout << "\t" << u->name << endl;
        for (auto iter = adj.begin(); iter != adj.end(); iter++) {
            if (visited.find(iter->first) == visited.end()) {
                visited.insert(iter->first);
                //unvisited.erase(iter->first);
                stk.push(iter->first);
            }
        }
    }
}
void RecipeGraph::similarRecipesTo(string name) {
    Recipe* target = NULL;
    for (auto iter = vertices.begin(); iter != vertices.end(); iter++) {
        if ((*iter)->name.find(name) != std::string::npos) {
            target = *iter;
            break;
        }
    }
    if (target == NULL) {
        cout << "Recipe not found" << endl;
        return;
    }
    else {
        cout << endl << "Recipes similar to " << target->name << ":" << endl;
        map<Recipe*, int> adj = adjList[target];
        priority_queue<pair<Recipe*, int>, vector<pair<Recipe*, int>>, recipeComp> pq;
        for (auto iter = adj.begin(); iter != adj.end(); iter++) {
            pq.push(*iter);
        }
        int numPrinted = 0;
        while (numPrinted < 10 && !pq.empty()) {
            pair<Recipe*, int> max = pq.top();
            pq.pop();
            cout << "\t" << max.first->name << endl;
            numPrinted++;
        }
    }
}