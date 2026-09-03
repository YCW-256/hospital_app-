#ifndef CDATA_H
#define CDATA_H
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <QDebug>
#include <vector>
#include "protecol.h"
#include <QWidget>
using namespace std;

typedef struct{
    QString name;
    QString time;
    int state;
}APP_INFO;
typedef struct{
    int id;
    QString name;
    int time;
    QString department;
}doctor_info_use;


class CData
{
public:
    CData();
    static vector<int>devices;
    static void init();
    static bool read_devices_id();
    static bool readJsonFile();
    //---变量
    static QString ip;
    static int port;
    static int current_width;
    static int current_height;
    static int m_id;
    static QString m_name;
    static QWidget* current_widget;
    static vector<doctor_info_use> m_doctor_info;

};

#endif // CDATA_H
