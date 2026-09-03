#include "getdoctorinfotask.h"

GetDoctorInfoTask::GetDoctorInfoTask(QObject *parent)
    : BusinessTask{parent}
{

}

GetDoctorInfoTask::GetDoctorInfoTask(int len, const QByteArray &data, QObject *parent)
    :BusinessTask(len,data,parent)
{

}

void GetDoctorInfoTask::execute()
{
    CData::m_doctor_info.resize(0);
    doctor_info doc;
    int now=0;
    while(now<len){
        memcpy(&doc,data.data()+now,sizeof(doctor_info));
        now+=sizeof(doctor_info);
        doctor_info_use info;
        info.name=doc.name;
        info.time=doc.time;
        info.department=doc.department;
        info.id=doc.id;
        CData::m_doctor_info.push_back(info);
        qDebug()<<"id"<<info.id<<"name"<<info.name<<"depart"<<info.department<<"time"<<info.time;
    }
}
