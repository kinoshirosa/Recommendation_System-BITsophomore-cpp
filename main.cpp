#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <ctime>
#include <unordered_map>
#include <climits>
#include <cctype>
#include <cfloat>
#include <limits>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <sstream>
#include <cstdlib>  
using namespace std;

class CategoryNode;

// ��Ʒ��
class Product {
public:
    int id;          // ��ƷΨһ��ʶ��
    string name;     // ��Ʒ����
    double price;    // ��Ʒ�۸�
    int sales;       // ��Ʒ����
    double rating;   // ��Ʒ���֣�0-5�֣�
    time_t addTime;  // ��Ʒ���ʱ��
    string brand;    // ��ƷƷ��
    CategoryNode* categoryPtr; // ָ����������ָ��
    bool isHidden;   // �Ƿ�Ϊ������Ʒ
    
    // ���캯����ʼ����Ʒ����
    Product(int id, string name, double price, string brand, 
            CategoryNode* catPtr = nullptr, bool hidden = false)
        : id(id), name(name), price(price), brand(brand), 
          sales(rand() % 50), rating(rand() % 6), addTime(time(0) - rand() % (365 * 24 * 3600)), 
          categoryPtr(catPtr), isHidden(hidden) {}
};

// �û���
class User {
public:
    int id;        // �û�Ψһ��ʶ��
    string username; // �û���
    string password; // ����
    string role;   // �û���ɫ
    vector<int> browseHistory;  // �����ʷ��¼
    vector<int> purchaseHistory; // ������ʷ��¼
    double totalSpent; // �ۼ����ѽ��
    
    // ���캯����ʼ���û�����
    User(int id, string username, string password, string role) 
        : id(id), username(username), password(password), role(role), totalSpent(0.0) {}
    
    // ��������¼
    void addBrowse(int productId) {
        browseHistory.push_back(productId);
    }
    
    // ��ӹ����¼
    void addPurchase(int productId, double amount) {
        purchaseHistory.push_back(productId);
        totalSpent += amount;
    }
    
    // ��ʾ�û���Ϣ
    void display() const {
        cout << "�û�ID: " << id << endl;
        cout << "�û���: " << username << endl;
        cout << "��ɫ: " << role << endl;
        cout << "�ۼ�����: " << fixed << setprecision(2) << totalSpent << "Ԫ" << endl;
    }
    
    // ����Ƿ�ΪVIP
    bool isVIP() const {
        return role == "VIP�û�" || role == "����Ա";
    }
};

// ��Ʒ������ڵ�
class CategoryNode {
public:
    string name;                // �������
    vector<CategoryNode*> children; // ������б�
    vector<Product*> products;  // ������µ���Ʒ�б�
    CategoryNode* parent;       // �����ָ��
    set<string> allowedRoles;   // ������ʵĽ�ɫ����
    
    // ���캯����ʼ�����ڵ�
    CategoryNode(string name, CategoryNode* parent = nullptr) 
        : name(name), parent(parent) {}
    
    // ��������
    void addChild(CategoryNode* child) {
        children.push_back(child);
    }
    
    // ��ʾ�����Ϣ
    void display(int level = 0) const {
        for (int i = 0; i < level; i++) cout << "  ";
        cout << "- " << name;
        
        if (!products.empty()) {
            cout << " (" << products.size() << "����Ʒ)";
        }
        cout << endl;
        
        for (CategoryNode* child : children) {
            child->display(level + 1);
        }
    }
};

// �Ƽ�ϵͳ������
class RecommendationSystem {
private:
    CategoryNode* root; // ��������ڵ�
    map<int, Product*> products; // ������Ʒ
    map<int, User*> users;       // �����û�
    map<string, int> categoryPopularity; // ������ж�ͳ��
    int nextUserId = 1000;      // ��һ���û�ID
    int nextProductId = 1;      // ��һ����ƷID
    User* currentUser = nullptr; // ��ǰ��¼�û�
    const double VIP_DISCOUNT = 0.9; // VIP�ۿ���
    const double UPGRADE_THRESHOLD = 5000.0; // ����VIP��������ֵ
    
    // ��������������ڵ�
    CategoryNode* findCategoryDFS(CategoryNode* node, const string& name) {
        if (node->name == name) return node;
        for (CategoryNode* child : node->children) {
            CategoryNode* result = findCategoryDFS(child, name);
            if (result) return result;
        }
        return nullptr;
    }
    
    // ��������������ڵ�
    CategoryNode* findCategoryBFS(const string& name) {
        if (!root) return nullptr;
        
        queue<CategoryNode*> q;
        q.push(root);
        
        while (!q.empty()) {
            CategoryNode* current = q.front();
            q.pop();
            
            if (current->name == name) 
                return current;
                
            for (CategoryNode* child : current->children) {
                q.push(child);
            }
        }
        return nullptr;
    }
    
    // ����û�����Ȩ��
    bool hasAccess(CategoryNode* node, User* user) {
        return node->allowedRoles.empty() || 
               node->allowedRoles.find(user->role) != node->allowedRoles.end();
    }
    
    // ��ȡ�û�����Ȥ����Ʒ���
    vector<string> getUserInterestCategories(User* user) {
        map<string, int> categoryCount;
        
        for (int pid : user->browseHistory) {
            if (products.find(pid) != products.end()) {
                Product* p = products[pid];
                CategoryNode* cat = p->categoryPtr;
                
                while (cat) {
                    categoryCount[cat->name]++;
                    cat = cat->parent;
                }
            }
        }
        
        for (int pid : user->purchaseHistory) {
            if (products.find(pid) != products.end()) {
                Product* p = products[pid];
                CategoryNode* cat = p->categoryPtr;
                
                while (cat) {
                    categoryCount[cat->name] += 2;
                    cat = cat->parent;
                }
            }
        }
        
        vector<pair<string, int>> sorted(categoryCount.begin(), categoryCount.end());
        sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
            return a.second > b.second;
        });
        
        vector<string> result;
        for (int i = 0; i < min(3, (int)sorted.size()); i++) {
            result.push_back(sorted[i].first);
        }
        return result;
    }
    
public:
    // ���캯������ʼ�������
    RecommendationSystem() {
        srand(time(0)); 
        root = new CategoryNode("������Ʒ");
        initializeCategories();
        loadProductsFromCSV("products.csv");
        createAdminAccount();
    }
    
    // �����Ʒ��ϵͳ
    void addProduct(string name, double price, string brand, string categoryPath, bool hidden = false) {
        int newId = nextProductId;
        while (products.find(newId) != products.end()) {
            newId++;
        }
        nextProductId = newId + 1;
        addProduct(newId, name, price, brand, categoryPath, hidden);
    }
    
    // �����Ʒ��ϵͳ����ID��
    void addProduct(int id, string name, double price, string brand, string categoryPath, bool hidden = false) {
        if (products.find(id) != products.end()) {
            throw runtime_error("��ƷID�Ѵ���: " + to_string(id));
        }
    
        vector<string> categories;
        string temp;
        for (char c : categoryPath) {
            if (c == '/') {
                if (!temp.empty()) {
                    categories.push_back(temp);
                    temp = "";
                }
            } else {
                temp += c;
            }
        }
        if (!temp.empty()) categories.push_back(temp);
    
        CategoryNode* current = root; 
        for (const string& catName : categories) {
            bool found = false;
            for (CategoryNode* child : current->children) {
                if (child->name == catName) {
                    current = child;
                    found = true;
                    break;
                }
            }
            if (!found) {
                CategoryNode* newCat = new CategoryNode(catName, current);
                current->addChild(newCat);
                current = newCat;
            }
        }
    
        Product* p = new Product(id, name, price, brand, current, hidden);
        current->products.push_back(p);
        products[id] = p;
        categoryPopularity[current->name]++;  
    }
    
    // ��CSV�ļ�������Ʒ
    void loadProductsFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "�޷����ļ�: " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // ����������
        
        while (getline(file, line)) {
            stringstream ss(line);
            string token;
            vector<string> tokens;
            
            while (getline(ss, token, ',')) {
                tokens.push_back(token);
            }
            
            if (tokens.size() < 5) continue;
            
            try {
                int id = stoi(tokens[0]);
                string name = tokens[1];
                double price = stod(tokens[2]);
                string brand = tokens[3];
                string categoryPath = tokens[4];
                bool hidden = (tokens.size() > 5 && tokens[5] == "hidden");
                
                if (products.find(id) != products.end()) {
                    id = nextProductId++;
                } else if (id >= nextProductId) {
                    nextProductId = id + 1;
                }
                
                addProduct(id, name, price, brand, categoryPath, hidden);
            } catch (...) {
                cerr << "��������: " << line << endl;
            }
        }
        file.close();
        cout << "�ɹ����� " << products.size() << " ����Ʒ" << endl;
    }
    
    // ��������Ա�˻�
    void createAdminAccount() {
        registerUser("admin", "admin123", "����Ա");
    }    
    
    // ��ȡ��ǰ��¼�û�
    User* getCurrentUser() const {
        return currentUser;
    }
    
    // �û���¼
    bool login(string username, string password) {
        for (auto& pair : users) {
            User* user = pair.second;
            if (user->username == username && user->password == password) {
                currentUser = user;
                return true;
            }
        }
        return false;
    }
    
    // �û��ǳ�
    void logout() {
        currentUser = nullptr;
    }
    
    // ע�����û�
    void registerUser(string username, string password, string role = "��ͨ�û�") {
        for (auto& pair : users) {
            if (pair.second->username == username) {
                throw runtime_error("�û����Ѵ���");
            }
        }      
        User* newUser = new User(nextUserId++, username, password, role);
        users[newUser->id] = newUser;
    }
    
    // �����û�ΪVIP
    void upgradeToVIP(int userId) {
        auto it = users.find(userId);
        if (it != users.end()) {
            if (it->second->role == "��ͨ�û�") {
                it->second->role = "VIP�û�";
                cout << "�û�ID " << userId << " ������ΪVIP�û�" << endl;
            } else {
                cout << "�û�����VIP�����Ա" << endl;
            }
        } else {
            cout << "δ�ҵ��û�ID: " << userId << endl;
        }
    }
    
    // ��ʼ�������
    void initializeCategories() {
        vector<string> topCategories = {
            "���Ӳ�Ʒ", "��װ", "�Ҿ�", "ʳƷ", 
            "��ױ", "ͼ��", "�˶�", "ĸӤ"
        };
        
        for (const string& name : topCategories) {
            CategoryNode* topCat = new CategoryNode(name, root);
            root->addChild(topCat);
        }
    }

    // �¼���Ʒ
    void removeProduct(int productId) {
        auto it = products.find(productId);
        if (it != products.end()) {
            Product* p = it->second;
            if (p->categoryPtr) {
                auto& prods = p->categoryPtr->products;
                prods.erase(remove(prods.begin(), prods.end(), p), prods.end());
            }
            products.erase(it);
            delete p;
            cout << "��ƷID " << productId << " �ѳɹ��¼ܣ�" << endl;
        } else {
            cout << "δ�ҵ���ƷID: " << productId << endl;
        }
    }
  
    // ����ʽ�����������·����
    vector<Product*> navigateSearch(vector<string> path) {
        if (!root || !currentUser) return {};
        
        CategoryNode* current = root;
        for (const string& catName : path) {
            bool found = false;
            for (CategoryNode* child : current->children) {
                if (child->name == catName && hasAccess(child, currentUser)) {
                    current = child;
                    found = true;
                    break;
                }
            }
            if (!found) return {};
        }
        
        vector<Product*> result;
        for (Product* p : current->products) {
            if (!p->isHidden || currentUser->isVIP()) {
                result.push_back(p);
            }
        }
        return result;
    }
    
    // �ؼ�������
    vector<Product*> keywordSearch(string keyword) {
        vector<Product*> results;
        if (!currentUser) return results;
        
        string lowerKeyword;
        for (char c : keyword) {
            lowerKeyword += tolower(c);
        }
        
        for (auto& kv : products) {
            Product* p = kv.second;
            if (!p || !p->categoryPtr) continue;
            
            if (!hasAccess(p->categoryPtr, currentUser)) continue;
            if (p->isHidden && !currentUser->isVIP()) continue;
            
            string lowerName;
            for (char c : p->name) {
                lowerName += tolower(c);
            }
            
            if (lowerName.find(lowerKeyword) != string::npos) {
                results.push_back(p);
            }
        }
        return results;
    }
    
    // ��Ʒ�Ƽ�
    vector<Product*> recommendProducts() {
        vector<Product*> recommendations;
        if (!currentUser) return recommendations;
        
        set<int> history;
        history.insert(currentUser->browseHistory.begin(), currentUser->browseHistory.end());
        history.insert(currentUser->purchaseHistory.begin(), currentUser->purchaseHistory.end());
        
        for (int pid : history) {
            auto it = products.find(pid);
            if (it == products.end()) continue;
            
            Product* base = it->second;
            if (!base || !base->categoryPtr) continue;
            
            CategoryNode* cat = base->categoryPtr;
            
            for (Product* p : cat->products) {
                if (p->id != pid && (!p->isHidden || currentUser->isVIP())) {
                    recommendations.push_back(p);
                }
            }
            
            if (cat->parent) {
                for (CategoryNode* sibling : cat->parent->children) {
                    if (sibling == cat) continue;
                    for (Product* p : sibling->products) {
                        if (!p->isHidden || currentUser->isVIP()) {
                            recommendations.push_back(p);
                        }
                    }
                }
            }
        }
        
        vector<string> interestCats = getUserInterestCategories(currentUser);
        for (const string& catName : interestCats) {
            CategoryNode* cat = findCategoryBFS(catName);
            if (!cat || !hasAccess(cat, currentUser)) continue;
            
            for (Product* p : cat->products) {
                if (!p->isHidden || currentUser->isVIP()) {
                    recommendations.push_back(p);
                }
            }
        }
        
        set<int> added;
        vector<Product*> finalResults;
        for (Product* p : recommendations) {
            if (p && added.find(p->id) == added.end()) {
                finalResults.push_back(p);
                added.insert(p->id);
            }
        }
        
        random_shuffle(finalResults.begin(), finalResults.end());
        
        if (finalResults.size() > 20) {
            finalResults.resize(20);
        }
        
        return finalResults;
    }
    
    // ��Ʒ���˺�����
    vector<Product*> filterAndSort(vector<Product*> items, 
                                 string brand = "", 
                                 double minPrice = 0, 
                                 double maxPrice = DBL_MAX,
                                 double minRating = 0,
                                 string sortBy = "Ĭ��") {
        vector<Product*> results;
        
        if (!currentUser) return results;
        if (maxPrice < minPrice) swap(minPrice, maxPrice);
        minRating = max(0.0, min(minRating, 5.0));
        
        for (Product* p : items) {
            if (!p || !p->categoryPtr) continue;
            
            if (!hasAccess(p->categoryPtr, currentUser)) continue;
            if (p->isHidden && !currentUser->isVIP()) continue;
            if (!brand.empty() && p->brand != brand) continue;
            if (p->price < minPrice || p->price > maxPrice) continue;
            if (p->rating < minRating) continue;
            
            results.push_back(p);
        }
        
        if (sortBy == "�۸�") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->price < b->price; });
        } else if (sortBy == "����") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->sales > b->sales; });
        } else if (sortBy == "��Ʒ") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->addTime > b->addTime; });
        } else if (sortBy == "����") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->rating > b->rating; });
        } else {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->addTime > b->addTime; });
        }
        
        return results;
    }
    
    // ���Ʒ���
    vector<string> analyzeTrends() {
        vector<pair<string, int>> sortedPopularity(
            categoryPopularity.begin(), categoryPopularity.end());
        
        sort(sortedPopularity.begin(), sortedPopularity.end(), 
            [](auto& a, auto& b) { return a.second > b.second; });
        
        vector<string> trends;
        int count = min(5, (int)sortedPopularity.size());
        for (int i = 0; i < count; i++) {
            trends.push_back(sortedPopularity[i].first + ": " + to_string(sortedPopularity[i].second));
        }
        return trends;
    }
    
    // ��ʾ��Ʒ�б�
    void displayProducts(const vector<Product*>& products) {
        if (products.empty()) {
            cout << "û���ҵ���Ʒ��" << endl;
            return;
        }
        
        cout << "\n��Ʒ�б�" << endl;
        cout << "======================================================================" << endl;
        cout << setw(5) << "ID" << setw(25) << "����" << setw(10) << "�۸�" 
             << setw(10) << "Ʒ��" << setw(10) << "����" << setw(10) << "����" << endl;
        cout << "======================================================================" << endl;
        
        for (Product* p : products) {
            double displayPrice = p->price;
            if (currentUser && currentUser->isVIP() && !p->isHidden) {
                displayPrice *= VIP_DISCOUNT;
            }
            
            cout << setw(5) << p->id 
                 << setw(25) << (p->name.length() > 22 ? p->name.substr(0, 22) + "..." : p->name)
                 << setw(10) << fixed << setprecision(2) << displayPrice;
            
            if (currentUser && currentUser->isVIP() && !p->isHidden && displayPrice != p->price) {
                cout << "*";
            } else {
                cout << " ";
            }
            
            cout << setw(10) << p->brand 
                 << setw(10) << p->sales 
                 << setw(10) << p->rating;
            
            if (p->isHidden) {
                cout << " [����]";
            }
            cout << endl;
        }
        cout << "======================================================================" << endl;
        if (currentUser && currentUser->isVIP()) {
            cout << "* VIP�û�����" << (1 - VIP_DISCOUNT)*100 << "%�ۿ�" << endl;
        }
    }
    
    // ��ʾ�����
    void displayCategoryTree() {
        if (!root) return;
        cout << "\n��Ʒ����ṹ��" << endl;
        root->display();
    }
    
    // �����Ʒ
    void browseProducts() {
        if (!currentUser) return;
        
        vector<string> path;
        CategoryNode* current = root;
        
        while (true) {
            cout << "\n��ǰλ��: ";
            for (const string& p : path) cout << p << " > ";
            cout << current->name << endl;
            
            if (!current->children.empty()) {
                cout << "\n�����" << endl;
                for (int i = 0; i < current->children.size(); i++) {
                    cout << "  " << (i+1) << ". " << current->children[i]->name << endl;
                }
            }
            
            vector<Product*> visibleProducts;
            for (Product* p : current->products) {
                if (!p->isHidden || currentUser->isVIP()) {
                    visibleProducts.push_back(p);
                }
            }
            
            if (!visibleProducts.empty()) {
                displayProducts(visibleProducts);
            }
            
            cout << "\n��ѡ�������" << endl;
            cout << "  0. �����ϼ�" << endl;
            if (!current->children.empty()) {
                cout << "  1-" << current->children.size() << ". ���������" << endl;
            }
            cout << "  b. �����Ʒ" << endl;
            cout << "  q. �˳����" << endl;
            
            string choice;
            cout << "> ";
            cin >> choice;
            
            if (choice == "q") {
                break;
            } else if (choice == "0") {
                if (current->parent) {
                    current = current->parent;
                    if (!path.empty()) path.pop_back();
                } else {
                    cout << "���ڸ�Ŀ¼��" << endl;
                }
            } else if (choice == "b") {
                if (!visibleProducts.empty()) {
                    displayProducts(visibleProducts);
                } else {
                    cout << "��ǰ���û����Ʒ��" << endl;
                }
            } else {
                try {
                    int index = stoi(choice) - 1;
                    if (index >= 0 && index < current->children.size()) {
                        if (hasAccess(current->children[index], currentUser)) {
                            path.push_back(current->name);
                            current = current->children[index];
                        } else {
                            cout << "��û�з��ʴ˷����Ȩ�ޣ�" << endl;
                        }
                    } else {
                        cout << "��Чѡ��" << endl;
                    }
                } catch (...) {
                    cout << "��Ч���룡" << endl;
                }
            }
        }
    }
    
    // ������Ʒ
    void purchaseProduct(int productId) {
        if (!currentUser) {
            cout << "���ȵ�¼��" << endl;
            return;
        }
        
        auto it = products.find(productId);
        if (it != products.end()) {
            Product* p = it->second;
            
            if (p->isHidden && !currentUser->isVIP()) {
                cout << "VIPר����Ʒ����ͨ�û��޷�����" << endl;
                return;
            }
            
            double price = p->price;
            if (currentUser->isVIP()) {
                price *= VIP_DISCOUNT;
            }
            
            currentUser->addBrowse(productId);
            currentUser->addPurchase(productId, price);
            p->sales++;
            
            cout << "�ɹ�������Ʒ: " << p->name << "��" << endl;
            cout << "֧�����: " << fixed << setprecision(2) << price << "Ԫ" << endl;
            
            if (currentUser->role == "��ͨ�û�" && currentUser->totalSpent >= UPGRADE_THRESHOLD) {
                currentUser->role = "VIP�û�";
                cout << "��ϲ����������ΪVIP�û���" << endl;
            }
        } else {
            cout << "δ�ҵ���ƷID: " << productId << endl;
        }
    }
    
    // ����Ա�˵�
    void adminMenu() {
        while (true) {
            cout << "\n===== ����Ա�˵� =====" << endl;
            cout << "1. �����Ʒ" << endl;
            cout << "2. ɾ����Ʒ" << endl;
            cout << "3. �鿴����" << endl;
            cout << "4. ��ʾ������Ʒ" << endl;
            cout << "5. ��ʾ����ṹ" << endl;
            cout << "6. �����û�ΪVIP" << endl;
            cout << "0. �˳�����Ա�˵�" << endl;
            
            int choice;
            cout << "> ";
            cin >> choice;
            
            if (choice == 0) break;
            
            switch (choice) {
                case 1: {
                    string name, brand, categoryPath;
                    double price;
                    char hidden;
                    bool isHidden = false;
                    
                    cout << "������Ʒ����: ";
                    cin.ignore();
                    getline(cin, name);
                    
                    cout << "������Ʒ�۸�: ";
                    cin >> price;
                    
                    cout << "������ƷƷ��: ";
                    cin.ignore();
                    getline(cin, brand);
                    
                    cout << "������Ʒ���·�� (����: ���Ӳ�Ʒ/�ֻ�/ƻ��): ";
                    getline(cin, categoryPath);
                    
                    cout << "�Ƿ�Ϊ������Ʒ? (y/n): ";
                    cin >> hidden;
                    if (hidden == 'y' || hidden == 'Y') {
                        isHidden = true;
                    }
                    
//                    categoryPath = "������Ʒ/" + categoryPath;
                    addProduct(name, price, brand, categoryPath, isHidden);
                    cout << "��Ʒ��ӳɹ���" << endl;
                    break;
                }
                case 2: {
                    int productId;
                    cout << "����Ҫɾ������ƷID: ";
                    cin >> productId;
                    removeProduct(productId);
                    break;
                }
                case 3: {
                    map<string, int> categorySales;
                    
                    for (auto& kv : products) {
                        Product* p = kv.second;
                        if (!p->categoryPtr) continue;
                        
                        vector<string> path;
                        CategoryNode* current = p->categoryPtr;
                        while (current && current != root) {
                            path.insert(path.begin(), current->name);
                            current = current->parent;
                        }
                        
                        if (path.size() >= 3) {
                            string l3 = path[0] + "/" + path[1] + "/" + path[2];
                            categorySales[l3] += p->sales;
                        }
                    }
                    
                    vector<pair<string, int>> sortedSales(
                        categorySales.begin(), categorySales.end());
                    
                    sort(sortedSales.begin(), sortedSales.end(), 
                        [](auto& a, auto& b) { return a.second > b.second; });
                    
                    cout << "\n===== ���������������� =====" << endl;
                    cout << "һ������/��������/�������� - ������" << endl;
                    cout << "---------------------------------" << endl;
                    
                    int count = 0;
                    for (auto& entry : sortedSales) {
                        if (count++ >= 10) break;
                        cout << "  " << entry.first << " - " << entry.second << "��" << endl;
                    }
                    break;
                }
                case 4: {
                    vector<Product*> allProducts;
                    for (auto& kv : products) {
                        allProducts.push_back(kv.second);
                    }
                    displayProducts(allProducts);
                    break;
                }
                case 5: {
                    displayCategoryTree();
                    break;
                }
                case 6: {
                    int userId;
                    cout << "����Ҫ�������û�ID: ";
                    cin >> userId;
                    upgradeToVIP(userId);
                    break;
                }
                default:
                    cout << "��Чѡ��" << endl;
            }
        }
    }
    
    // �û��˵�
    void userMenu() {
        while (true) {
            cout << "\n===== �û��˵� =====" << endl;
            cout << "1. �����Ʒ" << endl;
            cout << "2. ������Ʒ" << endl;
            cout << "3. �鿴�Ƽ���Ʒ" << endl;
            cout << "4. ������Ʒ" << endl;
            cout << "5. ��ʾ����ṹ" << endl;
            cout << "6. �鿴�û���Ϣ" << endl;
            cout << "0. �˳���¼" << endl;
            
            int choice;
            cout << "> ";
            cin >> choice;
            
            if (choice == 0) {
                logout();
                cout << "���˳���¼��" << endl;
                break;
            }
            
            switch (choice) {
                case 1: {
                    browseProducts();
                    break;
                }
                case 2: {
                    string keyword;
                    cout << "���������ؼ���: ";
                    cin.ignore();
                    getline(cin, keyword);
    
                    vector<Product*> results = keywordSearch(keyword);
                    vector<Product*> currentList = results;
                    
                    displayProducts(currentList);
                    int a=0;
                    if (currentList.empty())
                    a=1;
                    if(a==1)
                    break;
                    
                    while (true) {
                        cout << "\n===== ���˺�����ѡ�� =====" << endl;
                        cout << "1. ��Ʒ�ƹ���" << endl;
                        cout << "2. ���۸�Χ����" << endl;
                        cout << "3. �����ֹ���" << endl;
                        cout << "4. ���۸�����" << endl;
                        cout << "5. ����������" << endl;
                        cout << "6. ����������" << endl;
                        cout << "7. ����Ʒ����" << endl;
                        cout << "0. �����ϼ��˵�" << endl;
        
                        int filterChoice;
                        cout << "> ";
                        cin >> filterChoice;
        
                        if (filterChoice == 0) {
                            break;
                        }
                        
                        string brand = "";
                        double minPrice = 0.0, maxPrice = DBL_MAX;
                        double minRating = 0.0;
                        string sortBy = "Ĭ��";
        
                        switch (filterChoice) {
                            case 1: {
                                cout << "����Ʒ������: ";
                                cin.ignore();
                                getline(cin, brand);
                                break;
                            }
                            case 2: {
                                cout << "������ͼ۸�: ";
                                cin >> minPrice;
                                cout << "������߼۸�: ";
                                cin >> maxPrice;
                                break;
                            }
                            case 3: {
                                cout << "�����������: ";
                                cin >> minRating;
                                break;
                            }
                            case 4: {
                                sortBy = "�۸�";
                                break;
                            }
                            case 5: {
                                sortBy = "����";
                                break;
                            }
                            case 6: {
                                sortBy = "����";
                                break;
                            }
                            case 7: {
                                sortBy = "��Ʒ";
                                break;
                            }
                            default:
                                cout << "��Чѡ��" << endl;
                                continue;
                        }
        
                        currentList = filterAndSort(results, brand, minPrice, maxPrice, minRating, sortBy);
                        displayProducts(currentList);
                    }
    
                    if (!currentList.empty()) {
                        cout << "���س������ز˵�...";
                        cin.ignore();
                        cin.get();
                    }
                    break;
                }
                case 3: {
                    vector<Product*> recommends = recommendProducts();
                    cout << "\nΪ���Ƽ�������Ʒ��" << endl;
                    displayProducts(recommends);
                    
                    if (!recommends.empty()) {
                        cout << "���س������ز˵�...";
                        cin.ignore();
                        cin.get();
                    }
                    break;
                }
                case 4: {
                    int productId;
                    cout << "����Ҫ�������ƷID: ";
                    cin >> productId;
                    purchaseProduct(productId);
                    break;
                }
                case 5: {
                    displayCategoryTree();
                    break;
                }
                case 6: {
                    if (currentUser) {
                        currentUser->display();
                    }
                    break;
                }
                default:
                    cout << "��Чѡ��" << endl;
            }
        }
    }
};

int main() {

    RecommendationSystem system;
    
    cout << "===== ��ӭʹ����Ʒ�Ƽ�ϵͳ =====" << endl;
    cout << "VIP������׼: �ۼ�������5000Ԫ" << endl;
    
    while (true) {
        cout << "\n===== ���˵� =====" << endl;
        cout << "1. ��¼" << endl;
        cout << "2. ע��" << endl;
        cout << "0. �˳�ϵͳ" << endl;
        
        int choice;
        cout << "> ";
        cin >> choice;
        
        if (choice == 0) {
            cout << "��лʹ�ã��ټ���" << endl;
            break;
        }
        
        string username, password;
        
        switch (choice) {
            case 1: {
                cout << "�����û���: ";
                cin >> username;
                cout << "��������: ";
                cin >> password;
                
                if (system.login(username, password)) {
                    User* user = system.getCurrentUser();
                    cout << "\n��¼�ɹ�����ӭ������" << user->username << "��" << endl;
                    user->display();
                    
                    if (user->role == "����Ա") {
                        system.adminMenu();
                    } else {
                        system.userMenu();
                    }
                } else {
                    cout << "�û������������" << endl;
                }
                break;
            }
            case 2: {
                cout << "�����û���: ";
                cin >> username;
                cout << "��������: ";
                cin >> password;
                
                try {
                    system.registerUser(username, password);
                    cout << "ע��ɹ������¼��" << endl;
                } catch (const exception& e) {
                    cout << "ע��ʧ��: " << e.what() << endl;
                }
                break;
            }
            default:
                cout << "��Чѡ��" << endl;
        }
    }
    
    return 0;
}
