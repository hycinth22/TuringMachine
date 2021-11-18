// Compile this using C++11 standard
#include <iostream>
//#include <map>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <cstdlib>
using namespace std;
typedef unsigned long Num;

// ʵ���Զ����ݵĵ����
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

// ת�ƺ��������
struct OldState {
    int state;
    char read[2];
    // Ϊ�˴洢��map��key�У���Ҫ����Ƚ�
    bool operator<(const OldState &rhs) const {
        if (state != rhs.state) return state < rhs.state;
        if (read[0] != rhs.read[0]) return read[0] < rhs.read[0];
        return read[1] < rhs.read[1];
    }
    // Ϊ�˴洢��unordered_map��key�У���Ҫ�������
    bool operator==(const OldState &rhs) const {
        return state == rhs.state && read[0] == rhs.read[0] && read[1]==rhs.read[1];
    }
};
// Ϊ�˴洢��unordered_map��key�У���Ҫ����hash
struct OldStateHasher{
    std::hash<int> intHasher;
    std::hash<char> charHasher;
    unsigned operator()(const OldState& obj) const {
        return intHasher(obj.state) ^ charHasher(obj.read[0]) ^ charHasher(obj.read[1]);
    }
};

// ת�ƺ������Ҳ�
struct NewState {
    int state;
    char write[2];
    int move[2];
};

// ����ͼ���
class TuringM {
    int state;
    static const int finalState = 999999999;
    Tape tape1, tape2; // ���������
    Tape::size_type head1, head2; // ��дͷλ��
public:
    unordered_map<OldState, NewState, OldStateHasher> rules;     // �洢���е�ת�ƹ���
    TuringM() : state(0), head1(1), head2(1) {} // ���캯����ʼ��
    void work() {
        while (state != finalState) {
            auto ru = rules.find(OldState{state, tape1[head1], tape2[head2]});
            if (ru == rules.end()) {
                cerr << "abnormal halt! no next transferable status" << endl;
                printf("state: %d, (%c, %c)\n", state, tape1[head1], tape2[head2]);
                // �����ǰֽ��
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
        // ���������ֽ��
        cout << "Tape1: " << tape1.content.substr(1, tape1.last+1) << endl
             << "Tape2: " << tape2.content.substr(1, tape2.last+1) << endl;
    }

    // f(x)= a*x^2 + b*y
    Num compute(Num a, Num x, Num b, Num y) {
        // ��������ֽ��
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
        // ��ʼ����
        clock_t startTime = clock();
        this->work();
        clock_t endTime = clock();
        double cpu_time_used = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
        // ���н�������ֽ��2ȡ�����
        cout << "computation completed, time used " << cpu_time_used << " seconds." << endl;
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
            perror("read transfer rules from file failed:");
            printf("Please put the file into the directory where this program runs");
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
            while (fgetc(fp) != '\n' && !feof(fp)); //���Ը���ʣ�µ���������
            if (ret < 8) continue; // ������Ч��������ȡ��һ��
            if(mov1=='L') n.move[0]= -1; else if(mov1=='R') n.move[0]= +1; else n.move[0]= 0;
            if(mov2=='L') n.move[1]= -1; else if(mov2=='R') n.move[1]= +1; else n.move[1]= 0;
            rules.insert(make_pair(old, n));
            printf("q%d,(%c,%c)=q%d,(%c,%c),%c,%c\n",
                   old.state, old.read[0], old.read[1],
                   n.state, n.write[0], n.write[1],
                   mov1, mov2);
        }
        printf("all transfer rules have been read from file.\n");
    }
};

int main() {
    TuringM m;
    m.readRulesFromFile("./transfer_rules.txt");
    Num a, x, b, y;
    while (cin) {
        cout << "input a x b y to calculate a*x^2+b*y" << endl;
        cout << "a x b y:";
        cin >> a >> x >> b >> y;
        cout << "running...\n" << endl;

        Num result = m.compute(a, x, b, y);
        if (result != -1) {
            cout << "the result is " << result << endl << endl;
        } else {
            cout << "Turing machine shuts down abnormally!" << endl;
        }
        m.reset();
        cout << "-------------" << endl;
    }
    system("pause");
    return 0;
}
