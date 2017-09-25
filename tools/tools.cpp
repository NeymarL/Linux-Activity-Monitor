#include "tools.h"
#include "string.h"
#include <QDebug>
#include <stdio.h>

using namespace std;

bool is_digit(char* str)
{
    int n = strlen(str);
    for (int i = 0; i < n; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}

map<int, string>* get_username_dict()
{
    ifstream input;
    input.open("/etc/passwd");
    map<int, string>* dict;
    if (input.is_open()) {
        dict = new map<int, string>;
        char buffer[100];

        while (input.getline(buffer, 100)) {
            int i = 0;
            char* p = strtok(buffer, ":");
            char* name = NULL;
            while (p && i < 3) {
                if (i == 0) {
                    name = p;
                } else if (i == 2) {
                    int uid = atoi(p);
                    (*dict)[uid] = string(name);
                }
                //qDebug() << p << endl;
                i++;
                p = strtok(NULL, ":");
            }
        }
    }
    else {
        return NULL;
    }
    // map<int,string>::iterator it;
    // for(it = dict->begin(); it != dict->end(); ++it)
    //     qDebug() <<"key: "<< it->first <<" value: "<< (it->second).c_str() << endl;
    return dict;
}

string get_proper_unit(int num)
{
    string str = "KB";
    float fnum = num;
    if (fnum > 1024) {
        fnum /= 1024;
        str = "MB";
    }
    if (fnum > 1024) {
        fnum /= 1024;
        str = "GB";
    }
    char buf[20];
    if (fnum > 99) {
        sprintf(buf, "%.1f %s", fnum, str.c_str());
    } else {
        sprintf(buf, "%.2f %s", fnum, str.c_str());
    }

    return string(buf);
}

