// Compile this using C++11 standard
#include <iostream>
//#include <map>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cassert>
using namespace std;
typedef unsigned long Num;

// 实现自动扩容的单向无穷带
const char EMPTY = 'E';
class Tape {
public:
    string content;
    typedef string::size_type size_type;
    size_type last;
    Tape() : content(1000000, EMPTY), last(0) {}
    char &operator[](size_type index) {
        last = max(index, last);
        if (index >= content.size()) {
            content.resize(index << 2, EMPTY);
        }
        return content[index];
    }
};

// 转移函数的左侧
struct OldState {
    int state;
    char read[2];
    // 为了存储在map的key中，需要定义比较
    bool operator<(const OldState &rhs) const {
        if (state != rhs.state) return state < rhs.state;
        if (read[0] != rhs.read[0]) return read[0] < rhs.read[0];
        return read[1] < rhs.read[1];
    }
    // 为了存储在unordered_map的key中，需要定义相等
    bool operator==(const OldState &rhs) const {
        return state == rhs.state && read[0] == rhs.read[0] && read[1]==rhs.read[1];
    }
};
// 为了存储在unordered_map的key中，需要定义hash
struct OldStateHasher{
    std::hash<int> intHasher;
    std::hash<char> charHasher;
    unsigned operator()(const OldState& obj) const {
        return intHasher(obj.state) ^ charHasher(obj.read[0]) ^ charHasher(obj.read[1]);
    }
};

// 转移函数的右侧
struct NewState {
    int state;
    char write[2];
    int move[2];
};

// 两带图灵机
class TuringM {
    int state;
    static const int finalState = 999999999;
    Tape tape1, tape2; // 单向无穷带
    Tape::size_type head1, head2; // 读写头位置
public:
    unordered_map<OldState, NewState, OldStateHasher> rules;     // 存储所有的转移规则
    TuringM() : state(0), head1(1), head2(1) {} // 构造函数初始化
    void work() {
        while (state != finalState) {
            auto ru = rules.find(OldState{state, tape1[head1], tape2[head2]});
            if (ru == rules.end()) {
                cerr << "非正常停机！没有下一个可以转换到的状态" << endl;
                printf("状态: %d, (%c, %c)\n", state, tape1[head1], tape2[head2]);
                // 输出当前纸带
                cout << tape1.content << endl
                     << tape2.content << endl;
                exit(-1);
            }
            NewState n = ru->second;
            state = n.state;
            tape1[head1] = n.write[0], tape2[head2] = n.write[1];
            head1 += n.move[0];
            head2 += n.move[1];
        }
        // 输出结束后纸带
        cout << "Tape1: " << tape1.content.substr(1, tape1.last+1) << endl
             << "Tape2: " << tape2.content.substr(1, tape2.last+1) << endl;
    }

    // f(x)= a*x^2 + b*y
    Num compute(Num a, Num x, Num b, Num y) {
        // 创建输入纸带
        Tape::size_type i = 0;
        tape1[i++] = EMPTY;
        for (int c = 0; c < a; c++) tape1[i++] = '1';
        tape1[i++] = '0';
        for (int c = 0; c < x; c++) tape1[i++] = '1';
        tape1[i++] = '0';
        for (int c = 0; c < b; c++) tape1[i++] = '1';
        tape1[i++] = '0';
        for (int c = 0; c < y; c++) tape1[i++] = '1';
        tape1[i++] = '0';
        // 开始运行
        clock_t startTime = clock();
        this->work();
        clock_t endTime = clock();
        double cpu_time_used = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
        // 运行结束，从纸带2取出结果
        cout << "计算完成，用时 " << cpu_time_used << " 秒." << endl;
        int result = 0;
        while (tape2.content[head2++] == '1') ++result;
        return result;
    }

    void reset() {
        state = 0;
        head1 = head2 = 1;
        tape1.content = tape2.content = string() + EMPTY;
    }

    void readRulesFromFile(const char *filepath) {
        FILE *fp = fopen(filepath, "r");
        if (!fp) {
            perror("从文件读取转移规则失败:");
            printf("需要把转移规则文件%s放到程序目录下", filepath);
            exit(1);
        }
        OldState old{}; char mov1, mov2;
        NewState n{};
        int ret;
        while (ret = fscanf(fp, "q%d,(%c,%c)=q%d,(%c,%c),%c,%c",
                            &old.state, &old.read[0], &old.read[1],
                            &n.state, &n.write[0], &n.write[1],
                            &mov1, &mov2
        ), ret != EOF) {
            while (fgetc(fp) != '\n' && !feof(fp)); //忽略该行剩下的所有内容
            if (ret < 8) continue; // 该行无效，继续读取下一行
            if(mov1=='L') n.move[0]= -1; else if(mov1=='R') n.move[0]= +1; else n.move[0]= 0;
            if(mov2=='L') n.move[1]= -1; else if(mov2=='R') n.move[1]= +1; else n.move[1]= 0;
            rules.insert(make_pair(old, n));
            printf("q%d,(%c,%c)=q%d,(%c,%c),%c,%c\n",
                   old.state, old.read[0], old.read[1],
                   n.state, n.write[0], n.write[1],
                   mov1, mov2);
        }
        printf("转移规则读取完成.\n");
    }
};

// 批量验证运算正确性
void batch_verify() {
    TuringM m;
    m.readRulesFromFile("./transfer_rules.txt");
    Num a = 0, x = 0, b = 0, y = 0;
    for (int i=0; i<= 200; i++) {
        switch (i % 4) {
            case 0: ++a;break;
            case 1: ++x;break;
            case 2: ++b;break;
            case 3: ++y;break;
        }
        Num result = m.compute(a, x, b, y);
        printf("%d %d %d %d = %d\n", a, x, b, y, result);
        assert(result == a*x*x+b*y); // 使用断言检查图灵机结果正确性，对比图灵机运算结果与计算机直接计算
        if(result != a*x*x+b*y) {
            exit(1);
        }
        m.reset();
    }
}

int main() {
    // batch_verify(); return 0;
    TuringM m;
    m.readRulesFromFile("./transfer_rules.txt");
    Num a, x, b, y;
    while (cin) {
        cout << "输入 a x b y 来计算 a*x^2+b*y" << endl;
        cout << "a x b y:";
        cin >> a >> x >> b >> y;
        cout << "运行中...\n" << endl;
        Num result = m.compute(a, x, b, y);
        assert(result == a*x*x+b*y); // 使用断言检查图灵机结果正确性，对比图灵机运算结果与计算机直接计算
        if (result != -1) {
            cout << "结果是 " << result << endl << endl;
        } else {
            cout << "图灵机非正常停机!" << endl;
        }
        m.reset();
        cout << "-------------" << endl;
    }
    system("pause");
    return 0;
}
