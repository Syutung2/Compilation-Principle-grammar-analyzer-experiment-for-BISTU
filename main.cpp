#include "iostream"
#include "string"
#include "vector"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "queue"
#include "string.h"
#include <stack> //栈容器头文件
#include <map>   //map容器头文件

using namespace std;
#include "set"
char word[100] = {""};         //待测试的文字
char Vt[100] = {""};           //终结符
char Vn[100] = {""};           //非终结符
string Generative[100] = {""}; //文法产生式存储
int VtNum = 0;                 //终结符号的个数
int VnNum = 0;                 //非终结符号的个数
int GenNum = 0;                //文法产生式个数
int GenNumNew = 0;             //文法产生式分解后的个数

stack<char> st;            //预测分析栈
int table[100][100] = {0}; //预测表针

map<char, vector<string>> maps;       //分解后文法产生式存储
map<char, set<char>> firsts;          // first集合存储
map<char, set<char>> follows;         // follow集合存储
map<map<char, char>, string> PATable; // nullable集合存储

bool IsInSet(char x, char a[], int n)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] == x)
            return true;
    }
    return false;
}
//判断一个字符是否在一个集合中的程序
bool IsChar(char a, string b)
{
    int i = 0;
    while (b[i] != '\0')
    {
        if (a == b[i])
            return true;
        i++;
    }
    return false;
}
// 集合(follow)相加程序, with @
void readFile(char *argv[])
{
    //读取终结符合非终结符，位于文件中的第一行、第二行
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        printf("文件打开失败！\n");
        exit(0);
    }
    fscanf(fp, "%s", Vt);
    VtNum = strlen(Vt);
    fscanf(fp, "%s", Vn);
    VnNum = strlen(Vn);
    // 继续逐行读取文法产生式
    while (!feof(fp))
    {
        fscanf(fp, "%s", word);
        Generative[GenNum] = word;
        GenNum++;
    }
    fclose(fp);
}
//分解文法产生式
void splitGenerative()
{
    GenNumNew = 0;
    for (int i = 0; i < GenNum; i++)
    {
        // 获取产生式的第1个字符
        char firstChar = Generative[i][0];
        int pre = 3; // 前缀
        int j = 3;   // 第二趟循环的起始位置
        // 如果是|，则直接放入文法产生式分解后的结果
        while (j <= Generative[i].size())
        {
            // 如果是|，则直接放入文法产生式分解后的结果
            if (Generative[i][j] == '|' || Generative[i][j] == '\0')
            {
                string temp = Generative[i].substr(pre, j - pre);
                // 如果是空串，则跳过
                if (temp == "")
                {
                    j++;
                    pre = j;
                    continue;
                }
                // log(temp);
                cout << temp << endl;
                // 如果 firstChar
                maps[firstChar].push_back(temp);
                GenNumNew++;
                pre = j + 1;
            }
            j++;
        }
    }
    cout << "分解后的文法产生式个数：" << GenNumNew << endl;
    for (map<char, vector<string>>::iterator it = maps.begin(); it != maps.end(); it++)
    {
        for (int i = 0; i < it->second.size(); i++)
        {
            // 将it->first、it->second[i]连接起来存放到GenerativeNew
            cout << it->first << " " << it->second[i] << endl;
        }
    }
}
// 获取字符在 Vn 中的位置
int getVnIndex(char a)
{
    for (int i = 0; i < VnNum; i++)
    {
        if (a == Vn[i])
            return i;
    }
    return -1;
}
// 获取字符在 Vt 中的位置
int getVtIndex(char a)
{
    for (int i = 0; i < VtNum; i++)
    {
        if (a == Vt[i])
            return i;
    }
    return -1;
}
// 字符是否为非终结符
bool isVn(char a)
{
    for (int i = 0; i < VnNum; i++)
    {
        if (a == Vn[i])
            return true;
    }
    return false;
}
// 字符是否为终结符
bool isVt(char a)
{
    for (int i = 0; i < VtNum; i++)
    {
        if (a == Vt[i])
            return true;
    }
    return false;
}

/***
 * 获取first集
 * @param a
 * @return
 */
void getFirst(char a)
{
    int index = getVnIndex(a);
    // 从maps中获取a的值
    vector<string> temp = maps[a];
    // 遍历所有产生式
    for (int i = 0; i < temp.size(); i++)
    {
        // 如果该产生式右部第一个字符是终结符号
        if (isVt(temp[i][0]))
        {
            // 将终结符号放入first集
            firsts[a].insert(temp[i][0]);
        }
        // 如果该产生式右部第一个字符是非终结符号
        else if (IsInSet(temp[i][0], Vn, VnNum))
        {
            // 将该非终结符号的first集放入first集
            // 递归调用，对该非终结符号的first集进行求解
            // 将该非终结符号的first集放入first集
            getFirst(temp[i][0]);
            //  合并set集
            firsts[a].insert(firsts[temp[i][0]].begin(), firsts[temp[i][0]].end());
        }
        else if (temp[i][0] == '$')
        {
            //如果该非终结符产生式是个空
            //则将空加入左部字符的FIRST集
            firsts[a].insert('$');
        }
    }
}
/***
 * 获取follow集
 * @param a 当前非终结符号
 * @return void
 */

void getFollow(char a)
{
    int index = getVnIndex(a);
    int i, j;
    if (index == 0)
    {
        //如果待求解字符是开始字符
        //则把'#'加入其FOLLOW集
        follows[a].insert('#');
    }
    // 遍历maps中所有的键值对
    for (map<char, vector<string>>::iterator it = maps.begin(); it != maps.end(); it++)
    {
        char key = it->first;
        vector<string> value = it->second;
        // 这一次实验为了简化步骤，默认只考虑 A->aB、和A->aBc 三种情况
        // 遍历所有产生式
        for (i = 0; i < value.size(); i++)
        {
            string temp = value[i];
            for (j = 0; j < temp.size(); j++)
            {
                if (temp[j] == a && temp[j + 1] == '\0')
                {
                    // 是 A-> aB 的情况, 其中 a 是 空
                    // 将 A 的 FOLLOW集加入 B 的 FOLLOW集
                    if (key != a)
                    {
                        getFollow(key);
                        follows[a].insert(follows[key].begin(), follows[key].end());
                    }
                }
                else if ((temp[j] == a && temp[j + 1] != '\0'))
                {
                    // 是 A-> aBc 的情况
                    if (isVt(temp[j + 1]))
                    {
                        // 直接加入到 A 的 FOLLOW集
                        follows[a].insert(temp[j + 1]);
                    }
                    else if (isVn(temp[j + 1]))
                    {

                        if (firsts[temp[j + 1]].count('$') != 0)
                        {
                            // 如果 B 的 FIRST集中有空
                            // 将 A 的 FOLLOW集加入 B 的 FOLLOW集
                            if (key != a)
                            {
                                getFollow(key);
                                follows[a].insert(follows[key].begin(), follows[key].end());
                            }
                        }
                        // 将 右边的 first 集 - 空 加入 A 的 FOLLOW集
                        follows[a].insert(firsts[temp[j + 1]].begin(), firsts[temp[j + 1]].end());
                        follows[a].erase('$');
                    }
                }
            }
        }
    }
}
// char in string

void buildPATable()
{
    int i, j, k;
    for (i = 0; i < VtNum; i++)
    {
        // 遍历maps中所有的键值对
        for (map<char, vector<string>>::iterator it = maps.begin(); it != maps.end(); it++)
        {
            char key = it->first;
            vector<string> value = it->second;
            // 遍历所有产生式
            for (j = 0; j < value.size(); j++)
            {
                string temp = value[j];
                if (temp[0] == '$')
                {
                    //如果当前的产生式是：A->a且，a='@'，则判断当前的Vt[i]是否在
                    // A的FOLLOW集中，如果在，则将A->a加入到PATable中
                    if (follows[key].count(Vt[i]) != 0)
                    {
                        map<char, char> key_value;
                        key_value[key] = Vt[i];
                        PATable[key_value] = temp;
                    }
                }
                if (temp[0] == Vt[i])
                {

                    //如果当前的产生式是：A->a且，a='@'，则判断当前的Vt[i]是否在
                    map<char, char> key_value;
                    key_value[key] = Vt[i];
                    PATable[key_value] = temp;
                }
                else if (temp[1] == Vt[i])
                {
                    //如果当前的产生式是：A->a且，a='@'，则判断当前的Vt[i]是否在
                    map<char, char> key_value;
                    key_value[key] = Vt[i];
                    PATable[key_value] = temp;
                }
                else if (isVn(temp[0]))
                {
                    if (firsts[temp[0]].count(Vt[i]) != 0)
                    {
                        map<char, char> key_value;
                        key_value[key] = Vt[i];
                        PATable[key_value] = temp;
                    }
                }
            }
        }
    }
}

void analyze()
{
    int times = 1;
    queue<char> q;
    // 输入一个字符串
    string s;
    cout << "请输入一个字符串: ";
    cin >> s;
    // 将字符串中的字符全部入队
    for (int i = 0; i < s.size(); i++)
    {
        q.push(s[i]);
    }

    q.push('$');
    char abc;
    // 给 s 后面加上 $
    // 初始化状态
    st.push('$');
    st.push(Vn[0]);
    // 当堆栈顶部不是$ 和 队列顶部不是 $ 时
    while (st.top() != '$')
    {

        // 取出队列顶部的字符，但不出队
        abc = q.front();
        // 取出栈顶部的字符，但不出栈
        char key = st.top();
        if (key == abc)
        {
            cout << "匹配栈顶部的字符" << key << "和队列顶部的字符" << abc << "相同，出栈" << endl;
            q.pop();
            st.pop();
        }
        else
        {
            map<char, char> keys;
            keys[key] = abc; // 将栈顶部的字符和队列顶部的字符组合成一个键值对
            // 如果 PATable 中有该键值对
            cout << "栈顶元素：" << key << "待求字符串队列头部元素" << abc << " ";
            if (PATable.count(keys) != 0)
            {
                // 取出 PATable 中该键值对的值
                string s = PATable[keys];
                cout << "用" << key << "->" << s << "," << s << "逆序进栈" << endl;
                // key 出栈
                st.pop();
                // 将该值逆序放入栈中
                for (int i = s.size() - 1; i >= 0; i--)
                {
                    // 如果该值不是空
                    if (s[i] != '$')
                    {
                        st.push(s[i]);
                    }
                }
            }
            // 如果 PATable 中没有该键值对
            else
            {
                cout << "该字符串是非法的" << endl;
                break;
            }
        }
        times++;
    }
    // 如果队列顶部是$，则说明该字符串是合法的
    if (st.top() == '$')
    {
        cout << "该字符串是合法的" << endl;
    }
    else
    {
        cout << "该字符串是非法的" << endl;
    }
}
/***
 * Main function, push three parameters
 * parameter1: the file name of the file
 * @param argc number of parameters
 * @param argv parameters
 */
int main(int argc, char *argv[])
{
    // check the number of parameters
    if (argc != 2)
    {
        cout << "The number of parameters is wrong!" << endl;
        return 0;
    }
    // read the end character table from the file
    readFile(argv);
    // split the generative rules
    splitGenerative();
    // log all vn , vt
    cout << "Vn: " << VnNum << endl;
    for (int i = 0; i < VnNum; i++)
    {
        cout << Vn[i] << " ";
    }
    cout << endl;
    cout << "Vt: " << endl;
    for (int i = 0; i < VtNum; i++)
    {
        cout << Vt[i] << " ";
    }
    cout << endl;

    // get the first set
    for (int i = 0; i < VnNum; i++)
    {
        getFirst(Vn[i]);
    }

    cout << "first集" << endl;
    // 遍历maps中所有的键值对
    for (map<char, set<char>>::iterator it = firsts.begin(); it != firsts.end(); it++)
    {
        cout << it->first << ": ";
        for (set<char>::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
        {
            cout << *itt << " ";
        }
        cout << endl;
    }

    // get the first set

    for (int i = 0; i < VnNum; i++)
    {
        getFollow(Vn[i]);
    }

    cout << "follow集" << endl;
    // log the follow set
    for (map<char, set<char>>::iterator it = follows.begin(); it != follows.end(); it++)
    {
        cout << it->first << ": ";
        for (set<char>::iterator itt = it->second.begin(); itt != it->second.end(); itt++)
        {
            cout << *itt << " ";
        }
        cout << endl;
    }

    // build the parse table
    buildPATable();

    cout << "PATable" << endl;
    // log the parse table
    for (map<map<char, char>, string>::iterator it = PATable.begin(); it != PATable.end(); it++)
    {
        cout << it->first.begin()->first << "," << it->first.begin()->second << ": " << it->second << endl;
    }

    analyze();
}