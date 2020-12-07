//MAIN FUNCTIONALITY
//(DONEish) random recipe
//(DONE,DONE) search for recipe by name (both tree and graph)
//(DONE,DONE) given ingredient, show recipes containing it (both tree and graph)
//(DONE) simple and complex (tree sorted by ingredient list size)
//(DONE) show similar recipes (graph with ingredient strings separated and pushed into a set)

#include <iostream>
#include "RecipeClasses.h"
#include <json/value.h>
#include "json/json.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <iterator>
#include <time.h>
#include <algorithm>
#include <chrono>
using namespace std;

/*dataset from https://eightportions.com/datasets/Recipes/ */
Json::Value readInDataset() {
    Json::Value test;
    std::ifstream test_file("recipes_raw/recipes_raw_nosource_epi.json", ifstream::binary);
    test_file >> test;
    //cout << (*test.begin())["title"].asString() << endl; //without quotations
    //cout << (*test.begin())["title"].toStyledString() << endl; //with quotations
    return test;
}
RecipeTreeNode* createTree(Json::Value& dataset, set<int>& used) {
    RecipeTreeNode* root = NULL;
    int limit = 0;
    //set<int> used;
    used.clear();
    srand(time(NULL));
    //random insertion
    while (limit < 300) { 
        int offset = rand() % dataset.size();
        auto iter = dataset.begin();
        std::advance(iter, offset);
        std::string name = (*iter)["title"].asString();
        int numIngreds = (*iter)["ingredients"].size();
        Json::Value code = *iter;
        if (used.find(offset) == used.end() && numIngreds!=0) {
            root = insert(root, name, numIngreds, code);
            used.insert(offset);
            limit++;
        }
    }
    /* //sequential insertion
    for (auto iter = dataset.begin(); iter != dataset.end(); iter++) {
        std::string name = (*iter)["title"].asString();
        int numIngreds = (*iter)["ingredients"].size();
        Json::Value code = *iter;
        root = insert(root, name, numIngreds, code);
        limit++;
        if (limit == 1000)
            break;
    }*/
    return root;
}
void createGraph(Json::Value& dataset, set<int>& used, RecipeGraph& g) {
    //step 1: create all graph nodes
    for (auto iterator = used.begin(); iterator != used.end(); iterator++) {
        int offset = *iterator;
        auto iter = dataset.begin();
        std::advance(iter, offset);
        std::string name = (*iter)["title"].asString();
        set<string> keywords;
        for (int i = 0; i < (*iter)["ingredients"].size(); i++) {
            string ingredline = (*iter)["ingredients"][i].asString();
            istringstream ss(ingredline);
            string keyword;
            while(ss>>keyword)
                keywords.insert(keyword);
        }
        Json::Value code = *iter;
        Recipe* recipe = new Recipe(name, keywords, code);
        g.vertices.insert(recipe);
    }
    //step 2: establish edge and edge weights
    for (auto iter = g.vertices.begin(); iter != g.vertices.end(); iter++) {
        for (auto jter = iter; jter != g.vertices.end(); jter++) {
            if (iter != jter) {
                set<string> inter;
                set<string> s1 = (*iter)->keywords;
                set<string> s2 = (*jter)->keywords;
                std::set_intersection(s1.begin(),s1.end(),s2.begin(),s2.end(),std::inserter(inter,inter.begin()));
                if (inter.size() > 0) {
                    g.adjList[*iter][*jter] = inter.size();
                    g.adjList[*jter][*iter] = inter.size();
                }
            }
        }
    }
}

void PrintRecipe(Json::Value& code) {
    cout << endl;
    cout << "Title: " << code["title"].asString() << endl;
    cout << "Ingredients: " << endl;
    for (int i = 0; i < code["ingredients"].size(); i++) {
        cout << "\t" << code["ingredients"][i].asString() << endl;
    }
    cout << "Instructions: " << endl;
    string instructions = code["instructions"].asString();
    cout << instructions.substr(0, instructions.length() / 2) << endl;
}
void PrintRecipeByName(string name, Json::Value& dataset, set<int>& used) {
    for (auto offter = used.begin(); offter != used.end(); offter++) {
        auto iter = dataset.begin();
        advance(iter, *offter);
        std::string title = (*iter)["title"].asString();
        istringstream ss(name);
        string term;
        bool found = false;
        while (ss >> term) {
            if (title.find(term) != std::string::npos)
                found = true;
            else
                found = false;
            if (!found)
                break;
        }
        if (found) {
            PrintRecipe(*iter);
            break;
        }
        /*if (title.find(name) != std::string::npos) {
            PrintRecipe(*iter);
            break;
        }*/
    }
}

void DisplayHeader() {
    cout << endl;
    cout << "======================RECIPE FINDER======================" << endl;
    cout << "1. Search recipes by name" << endl;
    cout << "2. Search recipes by ingredients" << endl;
    cout << "3. Show me a simple recipe" << endl;
    cout << "4. Show me a complex recipe" << endl;
    cout << "5. Show me recipes similar to ..." << endl;
    cout << "6. Display instructions for recipe" << endl;
    cout << "7. Exit" << endl;
    cout << "=========================================================" << endl << endl;
}

int main()
{
    //Initializing Tree and Graph values
    cout << "Reading in dataset..." << endl;
    Json::Value dataset = readInDataset();
    cout << "Creating recipe tree..." << endl;
    set<int> used;
    RecipeTreeNode* root = createTree(dataset,used);
    cout << "Creating recipe graph..." << endl;
    RecipeGraph graph;
    createGraph(dataset, used, graph);

    int option = 0;
    string input;
    while (option != 7) {
        DisplayHeader();
        cout << "Enter an option: ";
        cin >> option;
        cin.ignore();
        if (option == 1) {
            cout << "Enter recipe name/keywords): ";
            getline(cin, input);
            cout << endl << "Results by tree traversal: " << endl;
            auto t1 = std::chrono::high_resolution_clock::now();
            searchTreeByName(root, input);
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            cout << "Search took " << duration << " microseconds." << endl;

            cout << endl << "Results by graph traversal: " << endl;
            t1 = std::chrono::high_resolution_clock::now();
            graph.searchByName(input);
            t2 = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            cout << "Search took " << duration << " microseconds." << endl;
        }
        else if (option == 2) {
            cout << "Enter ingredients, space-separated: ";
            getline(cin, input);
            cout << endl << "Results by tree traversal: " << endl;
            auto t1 = std::chrono::high_resolution_clock::now();
            searchTreeByIngredients(root, input);
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            cout << "Search took " << duration << " microseconds." << endl;

            cout << endl << "Results by graph traversal: " << endl;
            t1 = std::chrono::high_resolution_clock::now();
            graph.searchByIngredients(input);
            t2 = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            cout << "Search took " << duration << " microseconds." << endl;
        }
        else if (option == 3) {
            cout << "Today's simple recipe is... " << endl;
            PrintRecipe(getSimple(root)->code);
        }
        else if (option == 4) {
            cout << "Today's complex recipe is... " << endl;
            PrintRecipe(getComplex(root)->code);
        }
        else if (option == 5) {
            cout << "Enter specific recipe name for similar recipes: ";
            getline(cin, input);
            graph.similarRecipesTo(input);
        }
        else if (option == 6) {
            cout << "Enter specific recipe name to get instructions: ";
            getline(cin, input);
            PrintRecipeByName(input, dataset, used);
        }
        else if (option == 7) {
            break;
        }
        else {
            cout << "Invalid option" << endl;
        }
    }
    return 0;
}

