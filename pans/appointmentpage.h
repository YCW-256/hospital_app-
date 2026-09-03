#ifndef APPOINTMENTPAGE_H
#define APPOINTMENTPAGE_H

#include <QList>
#include <QWidget>

class AppointmentConfirmPopup;
class DoctorInfoCard;
class QButtonGroup;
class QGridLayout;
class QHideEvent;

/**
 * @brief 页面2：预约挂号页（QStackedWidget 索引 2）
 *
 * 白色圆角卡片内部横向分为左右两区：
 *  - 左侧：【选择科室】互斥可选中科室网格
 *  - 右侧：排班时段【上午/下午】 +【选择医生】3 列医生信息卡滚动网格
 * 卡片底部居中：【确认预约】按钮。
 *
 * 医生数据来自网络：进入页面 getDoctorInfo() 拉取今日值班医生到
 * CData::m_doctor_info；flush() 依据当前选中的科室 + 时段筛选出符合条件的
 * 医生，清空旧卡片并重建医生卡片网格。
 */
class AppointmentPage : public QWidget
{
    Q_OBJECT

public:
    explicit AppointmentPage(QWidget *parent = nullptr);

    void getDoctorInfo();

    /** 依据当前选中科室 + 时段，清空并重建右侧医生卡片网格（数据源 CData::m_doctor_info） */
    void flush();

signals:
    void backRequested();   // 返回首页
    void confirmRequested();// 确认预约
    void data_ready(QByteArray data, int size);

protected:
    void hideEvent(QHideEvent *event) override; // 页面隐藏时同步收起挂号确认弹窗

private slots:
    void onAppointmentConfirmed(); // 弹窗【确认挂号】触发：把选中医生的信息准备写入 CData

private:
    void initLayout(); // 构建页面布局
    void showDoctorConfirm(DoctorInfoCard *dc); // 弹出挂号确认弹窗（展示该医生挂号信息）
    QString currentDept() const;      // 当前选中的科室；未选中返回空串（不限科室）
    int currentSessionTime() const;   // 当前时段对应的 doctor_info.time；未选中返回 -1（不限时段）

    QWidget      *m_doctorContent = nullptr;  // 医生卡片滚动区内容容器
    QGridLayout  *m_doctorGrid    = nullptr;  // 医生卡片网格（3 列）
    QButtonGroup *m_deptGroup     = nullptr;  // 科室互斥组（读取当前选中科室）
    QButtonGroup *m_sessionGroup  = nullptr;  // 时段互斥组（读取当前选中时段）
    QString       m_avatarPath;               // 医生头像路径（暂统一为测试头像）
    QList<DoctorInfoCard *> m_doctorCards;    // 当前展示的医生卡片
    DoctorInfoCard *m_selectedDoctor = nullptr; // 当前选中的医生（供挂号确认提交使用）
    AppointmentConfirmPopup *m_confirmPopup = nullptr; // 挂号确认弹窗（复用单实例）
};

#endif // APPOINTMENTPAGE_H
