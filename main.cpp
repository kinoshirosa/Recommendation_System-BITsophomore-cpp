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

// 商品类
class Product {
public:
    int id;          // 商品唯一标识符
    string name;     // 商品名称
    double price;    // 商品价格
    int sales;       // 商品销量
    double rating;   // 商品评分（0-5分）
    time_t addTime;  // 商品添加时间
    string brand;    // 商品品牌
    CategoryNode* categoryPtr; // 指向所属类别的指针
    bool isHidden;   // 是否为隐藏商品
    
    // 构造函数初始化商品属性
    Product(int id, string name, double price, string brand, 
            CategoryNode* catPtr = nullptr, bool hidden = false)
        : id(id), name(name), price(price), brand(brand), 
          sales(rand() % 50), rating(rand() % 6), addTime(time(0) - rand() % (365 * 24 * 3600)), 
          categoryPtr(catPtr), isHidden(hidden) {}
};

// 用户类
class User {
public:
    int id;        // 用户唯一标识符
    string username; // 用户名
    string password; // 密码
    string role;   // 用户角色
    vector<int> browseHistory;  // 浏览历史记录
    vector<int> purchaseHistory; // 购买历史记录
    double totalSpent; // 累计消费金额
    
    // 构造函数初始化用户属性
    User(int id, string username, string password, string role) 
        : id(id), username(username), password(password), role(role), totalSpent(0.0) {}
    
    // 添加浏览记录
    void addBrowse(int productId) {
        browseHistory.push_back(productId);
    }
    
    // 添加购买记录
    void addPurchase(int productId, double amount) {
        purchaseHistory.push_back(productId);
        totalSpent += amount;
    }
    
    // 显示用户信息
    void display() const {
        cout << "用户ID: " << id << endl;
        cout << "用户名: " << username << endl;
        cout << "角色: " << role << endl;
        cout << "累计消费: " << fixed << setprecision(2) << totalSpent << "元" << endl;
    }
    
    // 检查是否为VIP
    bool isVIP() const {
        return role == "VIP用户" || role == "管理员";
    }
};

// 商品类别树节点
class CategoryNode {
public:
    string name;                // 类别名称
    vector<CategoryNode*> children; // 子类别列表
    vector<Product*> products;  // 该类别下的商品列表
    CategoryNode* parent;       // 父类别指针
    set<string> allowedRoles;   // 允许访问的角色集合
    
    // 构造函数初始化类别节点
    CategoryNode(string name, CategoryNode* parent = nullptr) 
        : name(name), parent(parent) {}
    
    // 添加子类别
    void addChild(CategoryNode* child) {
        children.push_back(child);
    }
    
    // 显示类别信息
    void display(int level = 0) const {
        for (int i = 0; i < level; i++) cout << "  ";
        cout << "- " << name;
        
        if (!products.empty()) {
            cout << " (" << products.size() << "件商品)";
        }
        cout << endl;
        
        for (CategoryNode* child : children) {
            child->display(level + 1);
        }
    }
};

// 推荐系统核心类
class RecommendationSystem {
private:
    CategoryNode* root; // 类别树根节点
    map<int, Product*> products; // 所有商品
    map<int, User*> users;       // 所有用户
    map<string, int> categoryPopularity; // 类别流行度统计
    int nextUserId = 1000;      // 下一个用户ID
    int nextProductId = 1;      // 下一个商品ID
    User* currentUser = nullptr; // 当前登录用户
    const double VIP_DISCOUNT = 0.9; // VIP折扣率
    const double UPGRADE_THRESHOLD = 5000.0; // 升级VIP的消费阈值
    
    // 深度优先搜索类别节点
    CategoryNode* findCategoryDFS(CategoryNode* node, const string& name) {
        if (node->name == name) return node;
        for (CategoryNode* child : node->children) {
            CategoryNode* result = findCategoryDFS(child, name);
            if (result) return result;
        }
        return nullptr;
    }
    
    // 广度优先搜索类别节点
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
    
    // 检查用户访问权限
    bool hasAccess(CategoryNode* node, User* user) {
        return node->allowedRoles.empty() || 
               node->allowedRoles.find(user->role) != node->allowedRoles.end();
    }
    
    // 获取用户感兴趣的商品类别
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
    // 构造函数：初始化类别树
    RecommendationSystem() {
        srand(time(0)); 
        root = new CategoryNode("所有商品");
        initializeCategories();
        loadProductsFromCSV("products.csv");
        createAdminAccount();
    }
    
    // 添加商品到系统
    void addProduct(string name, double price, string brand, string categoryPath, bool hidden = false) {
        int newId = nextProductId;
        while (products.find(newId) != products.end()) {
            newId++;
        }
        nextProductId = newId + 1;
        addProduct(newId, name, price, brand, categoryPath, hidden);
    }
    
    // 添加商品到系统（带ID）
    void addProduct(int id, string name, double price, string brand, string categoryPath, bool hidden = false) {
        if (products.find(id) != products.end()) {
            throw runtime_error("商品ID已存在: " + to_string(id));
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
    
    // 从CSV文件加载商品
    void loadProductsFromCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // 跳过标题行
        
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
                cerr << "解析错误: " << line << endl;
            }
        }
        file.close();
        cout << "成功加载 " << products.size() << " 件商品" << endl;
    }
    
    // 创建管理员账户
    void createAdminAccount() {
        registerUser("admin", "admin123", "管理员");
    }    
    
    // 获取当前登录用户
    User* getCurrentUser() const {
        return currentUser;
    }
    
    // 用户登录
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
    
    // 用户登出
    void logout() {
        currentUser = nullptr;
    }
    
    // 注册新用户
    void registerUser(string username, string password, string role = "普通用户") {
        for (auto& pair : users) {
            if (pair.second->username == username) {
                throw runtime_error("用户名已存在");
            }
        }      
        User* newUser = new User(nextUserId++, username, password, role);
        users[newUser->id] = newUser;
    }
    
    // 升级用户为VIP
    void upgradeToVIP(int userId) {
        auto it = users.find(userId);
        if (it != users.end()) {
            if (it->second->role == "普通用户") {
                it->second->role = "VIP用户";
                cout << "用户ID " << userId << " 已升级为VIP用户" << endl;
            } else {
                cout << "用户已是VIP或管理员" << endl;
            }
        } else {
            cout << "未找到用户ID: " << userId << endl;
        }
    }
    
    // 初始化类别树
    void initializeCategories() {
        vector<string> topCategories = {
            "电子产品", "服装", "家居", "食品", 
            "美妆", "图书", "运动", "母婴"
        };
        
        for (const string& name : topCategories) {
            CategoryNode* topCat = new CategoryNode(name, root);
            root->addChild(topCat);
        }
    }

    // 下架商品
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
            cout << "商品ID " << productId << " 已成功下架！" << endl;
        } else {
            cout << "未找到商品ID: " << productId << endl;
        }
    }
  
    // 导航式搜索（按类别路径）
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
    
    // 关键词搜索
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
    
    // 商品推荐
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
    
    // 商品过滤和排序
    vector<Product*> filterAndSort(vector<Product*> items, 
                                 string brand = "", 
                                 double minPrice = 0, 
                                 double maxPrice = DBL_MAX,
                                 double minRating = 0,
                                 string sortBy = "默认") {
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
        
        if (sortBy == "价格") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->price < b->price; });
        } else if (sortBy == "销量") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->sales > b->sales; });
        } else if (sortBy == "新品") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->addTime > b->addTime; });
        } else if (sortBy == "评分") {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->rating > b->rating; });
        } else {
            sort(results.begin(), results.end(), 
                [](Product* a, Product* b) { return a->addTime > b->addTime; });
        }
        
        return results;
    }
    
    // 趋势分析
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
    
    // 显示商品列表
    void displayProducts(const vector<Product*>& products) {
        if (products.empty()) {
            cout << "没有找到商品！" << endl;
            return;
        }
        
        cout << "\n商品列表：" << endl;
        cout << "======================================================================" << endl;
        cout << setw(5) << "ID" << setw(25) << "名称" << setw(10) << "价格" 
             << setw(10) << "品牌" << setw(10) << "销量" << setw(10) << "评分" << endl;
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
                cout << " [隐藏]";
            }
            cout << endl;
        }
        cout << "======================================================================" << endl;
        if (currentUser && currentUser->isVIP()) {
            cout << "* VIP用户享受" << (1 - VIP_DISCOUNT)*100 << "%折扣" << endl;
        }
    }
    
    // 显示类别树
    void displayCategoryTree() {
        if (!root) return;
        cout << "\n商品分类结构：" << endl;
        root->display();
    }
    
    // 浏览商品
    void browseProducts() {
        if (!currentUser) return;
        
        vector<string> path;
        CategoryNode* current = root;
        
        while (true) {
            cout << "\n当前位置: ";
            for (const string& p : path) cout << p << " > ";
            cout << current->name << endl;
            
            if (!current->children.empty()) {
                cout << "\n子类别：" << endl;
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
            
            cout << "\n请选择操作：" << endl;
            cout << "  0. 返回上级" << endl;
            if (!current->children.empty()) {
                cout << "  1-" << current->children.size() << ". 进入子类别" << endl;
            }
            cout << "  b. 浏览商品" << endl;
            cout << "  q. 退出浏览" << endl;
            
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
                    cout << "已在根目录！" << endl;
                }
            } else if (choice == "b") {
                if (!visibleProducts.empty()) {
                    displayProducts(visibleProducts);
                } else {
                    cout << "当前类别没有商品！" << endl;
                }
            } else {
                try {
                    int index = stoi(choice) - 1;
                    if (index >= 0 && index < current->children.size()) {
                        if (hasAccess(current->children[index], currentUser)) {
                            path.push_back(current->name);
                            current = current->children[index];
                        } else {
                            cout << "您没有访问此分类的权限！" << endl;
                        }
                    } else {
                        cout << "无效选择！" << endl;
                    }
                } catch (...) {
                    cout << "无效输入！" << endl;
                }
            }
        }
    }
    
    // 购买商品
    void purchaseProduct(int productId) {
        if (!currentUser) {
            cout << "请先登录！" << endl;
            return;
        }
        
        auto it = products.find(productId);
        if (it != products.end()) {
            Product* p = it->second;
            
            if (p->isHidden && !currentUser->isVIP()) {
                cout << "VIP专属商品，普通用户无法购买！" << endl;
                return;
            }
            
            double price = p->price;
            if (currentUser->isVIP()) {
                price *= VIP_DISCOUNT;
            }
            
            currentUser->addBrowse(productId);
            currentUser->addPurchase(productId, price);
            p->sales++;
            
            cout << "成功购买商品: " << p->name << "！" << endl;
            cout << "支付金额: " << fixed << setprecision(2) << price << "元" << endl;
            
            if (currentUser->role == "普通用户" && currentUser->totalSpent >= UPGRADE_THRESHOLD) {
                currentUser->role = "VIP用户";
                cout << "恭喜！您已升级为VIP用户！" << endl;
            }
        } else {
            cout << "未找到商品ID: " << productId << endl;
        }
    }
    
    // 管理员菜单
    void adminMenu() {
        while (true) {
            cout << "\n===== 管理员菜单 =====" << endl;
            cout << "1. 添加商品" << endl;
            cout << "2. 删除商品" << endl;
            cout << "3. 查看趋势" << endl;
            cout << "4. 显示所有商品" << endl;
            cout << "5. 显示分类结构" << endl;
            cout << "6. 升级用户为VIP" << endl;
            cout << "0. 退出管理员菜单" << endl;
            
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
                    
                    cout << "输入商品名称: ";
                    cin.ignore();
                    getline(cin, name);
                    
                    cout << "输入商品价格: ";
                    cin >> price;
                    
                    cout << "输入商品品牌: ";
                    cin.ignore();
                    getline(cin, brand);
                    
                    cout << "输入商品类别路径 (例如: 电子产品/手机/苹果): ";
                    getline(cin, categoryPath);
                    
                    cout << "是否为隐藏商品? (y/n): ";
                    cin >> hidden;
                    if (hidden == 'y' || hidden == 'Y') {
                        isHidden = true;
                    }
                    
//                    categoryPath = "所有商品/" + categoryPath;
                    addProduct(name, price, brand, categoryPath, isHidden);
                    cout << "商品添加成功！" << endl;
                    break;
                }
                case 2: {
                    int productId;
                    cout << "输入要删除的商品ID: ";
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
                    
                    cout << "\n===== 热门三级分类趋势 =====" << endl;
                    cout << "一级分类/二级分类/三级分类 - 总销量" << endl;
                    cout << "---------------------------------" << endl;
                    
                    int count = 0;
                    for (auto& entry : sortedSales) {
                        if (count++ >= 10) break;
                        cout << "  " << entry.first << " - " << entry.second << "件" << endl;
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
                    cout << "输入要升级的用户ID: ";
                    cin >> userId;
                    upgradeToVIP(userId);
                    break;
                }
                default:
                    cout << "无效选择！" << endl;
            }
        }
    }
    
    // 用户菜单
    void userMenu() {
        while (true) {
            cout << "\n===== 用户菜单 =====" << endl;
            cout << "1. 浏览商品" << endl;
            cout << "2. 搜索商品" << endl;
            cout << "3. 查看推荐商品" << endl;
            cout << "4. 购买商品" << endl;
            cout << "5. 显示分类结构" << endl;
            cout << "6. 查看用户信息" << endl;
            cout << "0. 退出登录" << endl;
            
            int choice;
            cout << "> ";
            cin >> choice;
            
            if (choice == 0) {
                logout();
                cout << "已退出登录！" << endl;
                break;
            }
            
            switch (choice) {
                case 1: {
                    browseProducts();
                    break;
                }
                case 2: {
                    string keyword;
                    cout << "输入搜索关键词: ";
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
                        cout << "\n===== 过滤和排序选项 =====" << endl;
                        cout << "1. 按品牌过滤" << endl;
                        cout << "2. 按价格范围过滤" << endl;
                        cout << "3. 按评分过滤" << endl;
                        cout << "4. 按价格排序" << endl;
                        cout << "5. 按销量排序" << endl;
                        cout << "6. 按评分排序" << endl;
                        cout << "7. 按新品排序" << endl;
                        cout << "0. 返回上级菜单" << endl;
        
                        int filterChoice;
                        cout << "> ";
                        cin >> filterChoice;
        
                        if (filterChoice == 0) {
                            break;
                        }
                        
                        string brand = "";
                        double minPrice = 0.0, maxPrice = DBL_MAX;
                        double minRating = 0.0;
                        string sortBy = "默认";
        
                        switch (filterChoice) {
                            case 1: {
                                cout << "输入品牌名称: ";
                                cin.ignore();
                                getline(cin, brand);
                                break;
                            }
                            case 2: {
                                cout << "输入最低价格: ";
                                cin >> minPrice;
                                cout << "输入最高价格: ";
                                cin >> maxPrice;
                                break;
                            }
                            case 3: {
                                cout << "输入最低评分: ";
                                cin >> minRating;
                                break;
                            }
                            case 4: {
                                sortBy = "价格";
                                break;
                            }
                            case 5: {
                                sortBy = "销量";
                                break;
                            }
                            case 6: {
                                sortBy = "评分";
                                break;
                            }
                            case 7: {
                                sortBy = "新品";
                                break;
                            }
                            default:
                                cout << "无效选择！" << endl;
                                continue;
                        }
        
                        currentList = filterAndSort(results, brand, minPrice, maxPrice, minRating, sortBy);
                        displayProducts(currentList);
                    }
    
                    if (!currentList.empty()) {
                        cout << "按回车键返回菜单...";
                        cin.ignore();
                        cin.get();
                    }
                    break;
                }
                case 3: {
                    vector<Product*> recommends = recommendProducts();
                    cout << "\n为您推荐以下商品：" << endl;
                    displayProducts(recommends);
                    
                    if (!recommends.empty()) {
                        cout << "按回车键返回菜单...";
                        cin.ignore();
                        cin.get();
                    }
                    break;
                }
                case 4: {
                    int productId;
                    cout << "输入要购买的商品ID: ";
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
                    cout << "无效选择！" << endl;
            }
        }
    }
};

int main() {

    RecommendationSystem system;
    
    cout << "===== 欢迎使用商品推荐系统 =====" << endl;
    cout << "VIP升级标准: 累计消费满5000元" << endl;
    
    while (true) {
        cout << "\n===== 主菜单 =====" << endl;
        cout << "1. 登录" << endl;
        cout << "2. 注册" << endl;
        cout << "0. 退出系统" << endl;
        
        int choice;
        cout << "> ";
        cin >> choice;
        
        if (choice == 0) {
            cout << "感谢使用，再见！" << endl;
            break;
        }
        
        string username, password;
        
        switch (choice) {
            case 1: {
                cout << "输入用户名: ";
                cin >> username;
                cout << "输入密码: ";
                cin >> password;
                
                if (system.login(username, password)) {
                    User* user = system.getCurrentUser();
                    cout << "\n登录成功！欢迎回来，" << user->username << "！" << endl;
                    user->display();
                    
                    if (user->role == "管理员") {
                        system.adminMenu();
                    } else {
                        system.userMenu();
                    }
                } else {
                    cout << "用户名或密码错误！" << endl;
                }
                break;
            }
            case 2: {
                cout << "输入用户名: ";
                cin >> username;
                cout << "输入密码: ";
                cin >> password;
                
                try {
                    system.registerUser(username, password);
                    cout << "注册成功！请登录。" << endl;
                } catch (const exception& e) {
                    cout << "注册失败: " << e.what() << endl;
                }
                break;
            }
            default:
                cout << "无效选择！" << endl;
        }
    }
    
    return 0;
}
