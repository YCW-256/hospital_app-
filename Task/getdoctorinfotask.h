#ifndef GETDOCTORINFOTASK_H
#define GETDOCTORINFOTASK_H

#include <QWidget>
#include "businesstask.h"

class GetDoctorInfoTask : public BusinessTask
{
public:
    explicit GetDoctorInfoTask(QObject *parent = nullptr);

    GetDoctorInfoTask(int len,const QByteArray &data, QObject *parent);

    void execute();

};

#endif // GETDOCTORINFOTASK_H
