#ifndef APPOINTMENTCONFIRMPOPUP_H
#define APPOINTMENTCONFIRMPOPUP_H

#include <QWidget>

class QLabel;

/**
 * @brief 挂号确认弹窗（预约挂号页点击医生卡片时弹出）
 *
 * 无边框、置顶、悬浮于主窗口之上（复用 AIAssistantPopup 的弹窗写法）。
 * 居中展示本次待确认医生的挂号信息：
 *   - 医生姓名
 *   - 科室
 *   - 挂号日期（服务器返回的是"今日值班"医生，故取当天日期）
 *   - 就诊时段（上午 / 下午 / 晚上，由 doctor_info_use.time 转换）
 * 底部提供【取消】与【确认挂号】两个按钮：
 *   - 确认 → 发出 confirmed()，由 AppointmentPage 的槽函数把医生信息写入 CData；
 *   - 取消 → 发出 cancelled() 并隐藏自身。
 */
class AppointmentConfirmPopup : public QWidget
{
    Q_OBJECT

public:
    explicit AppointmentConfirmPopup(QWidget *parent = nullptr);

    /**
     * 设置待确认医生并刷新信息显示
     * @param doctorName 医生姓名
     * @param department 所属科室
     * @param time       值班时段（doctor_info_use.time：0=上午 1=下午 2=晚上）
     */
    void setAppointmentInfo(const QString &doctorName, const QString &department, int time);

signals:
    void confirmed(); // 点击【确认挂号】
    void cancelled(); // 点击【取消】

private:
    void initUi();                          // 构建界面
    static QString sessionText(int time);   // 值班时段数字 → 中文（上午/下午/晚上）

    QLabel *m_doctorValue   = nullptr; // 医生姓名
    QLabel *m_deptValue     = nullptr; // 科室
    QLabel *m_dateValue     = nullptr; // 挂号日期
    QLabel *m_sessionValue  = nullptr; // 就诊时段
};

#endif // APPOINTMENTCONFIRMPOPUP_H
